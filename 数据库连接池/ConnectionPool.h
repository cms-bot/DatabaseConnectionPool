#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include <string>
#include <queue>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <functional>
#include "MysqlConn.h"
class ConnectionPool
{
public:
	// ��ȡ���ӳض���ʵ��
	static ConnectionPool* getConnectionPool();
	ConnectionPool(const ConnectionPool&) = delete;
	ConnectionPool operator =(const ConnectionPool&) = delete;
	// ���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ�����õĿ�������
	shared_ptr<MysqlConn> getConnection();
private:
	// ����#1 ���캯��˽�л�
	ConnectionPool();
	// �������ļ��м���������
	bool loadConfigFile();
	// �����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();
	// ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
	void scannerConnectionTask();
	std::string _ip; // mysql��ip��ַ
	unsigned short _port; // mysql�Ķ˿ں� 3306
	std::string _username; // mysql��¼�û���
	std::string _password; // mysql��¼����
	std::string _dbname; // ���ӵ����ݿ�����
	int _initSize; // ���ӳصĳ�ʼ������
	int _maxSize; // ���ӳص����������
	int _maxIdleTime; // ���ӳ�������ʱ��
	int _connectionTimeout; // ���ӳػ�ȡ���ӵĳ�ʱʱ��
	std::queue<MysqlConn*> _connectionQue; // �洢mysql���ӵĶ���
	std::mutex _queueMutex; // ά�����Ӷ��е��̰߳�ȫ������
	std::atomic_int _connectionCnt; // ��¼������������connection���ӵ������� 
	std::condition_variable cv; // ���������������������������̺߳����������̵߳�ͨ��
};
#endif
