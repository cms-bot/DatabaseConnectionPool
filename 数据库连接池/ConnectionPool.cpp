#include "ConnectionPool.h"
#include "json.h"
#include <fstream>
#include <thread>
#include "Log.h"
using namespace Json;
// �̰߳�ȫ���������������ӿ�
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool; // lock��unlock
	return &pool;
}

// �������ļ��м���������
bool ConnectionPool::loadConfigFile()
{
	ifstream ifs("dbconfig.json");
	Reader rd;
	Value root;
	rd.parse(ifs, root);
	if (root.isObject())
	{
		_ip = root["ip"].asString();
		_port = root["port"].asInt();
		_username = root["userName"].asString();
		_password = root["password"].asString();
		_dbname = root["dbName"].asString();
		_initSize = root["initSize"].asInt();
		_maxSize = root["maxSize"].asInt();
		_maxIdleTime = root["maxIdleTime"].asInt();
		_connectionTimeout = root["timeout"].asInt();
		return true;
	}
	return false;
}

// ���ӳصĹ���
ConnectionPool::ConnectionPool()
{
	// ������������
	if (!loadConfigFile())
	{
		return;
	}

	// ������ʼ����������
	for (int i = 0; i < _initSize; ++i)
	{
		MysqlConn* p = new MysqlConn();
		p->connect(_username, _password, _dbname, _ip);
		p->refreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
		_connectionQue.push(p);
		_connectionCnt++;
	}

	// ����һ���µ��̣߳���Ϊ���ӵ������� linux thread => pthread_create
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();

	// ����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

// �����ڶ������߳��У�ר�Ÿ�������������
void ConnectionPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<std::mutex> lock(_queueMutex);
		cv.wait(lock, [this]() {
			return _connectionQue.empty();
			});

		// ��������û�е������ޣ����������µ�����
		if (_connectionCnt < _maxSize)
		{
			MysqlConn* p = new MysqlConn();
			p->connect(_username, _password, _dbname, _ip);
			p->refreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
			_connectionQue.push(p);
			_connectionCnt++;
		}

		// ֪ͨ�������̣߳���������������
		cv.notify_all();
	}
}

// ���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
shared_ptr<MysqlConn> ConnectionPool::getConnection()
{
	unique_lock<std::mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		// sleep
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG("��ȡ�������ӳ�ʱ��...��ȡ����ʧ��!");
				return nullptr;
			}
		}
	}

	/*
	shared_ptr����ָ������ʱ�����connection��Դֱ��delete�����൱��
	����connection������������connection�ͱ�close���ˡ�
	������Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ����connectionֱ�ӹ黹��queue����
	*/
	shared_ptr<MysqlConn> sp(_connectionQue.front(),
		[&](MysqlConn* pcon) {
			// �������ڷ�����Ӧ���߳��е��õģ�����һ��Ҫ���Ƕ��е��̰߳�ȫ����
			unique_lock<std::mutex> lock(_queueMutex);
			pcon->refreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
			_connectionQue.push(pcon);
		});

	_connectionQue.pop();
	cv.notify_all();  // �����������Ժ�֪ͨ�������̼߳��һ�£��������Ϊ���ˣ��Ͻ���������

	return sp;
}

// ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		// ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// ɨ���������У��ͷŶ��������
		unique_lock<std::mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			MysqlConn* p = _connectionQue.front();
			if (p->getAliveTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; // ����~Connection()�ͷ�����
			}
			else
			{
				break; // ��ͷ������û�г���_maxIdleTime���������ӿ϶�û��
			}
		}
	}
}
