#ifndef MYSQLCONN_H
#define MYSQLCONN_H

#ifdef _MSC_VER
#ifdef _WIN64
#include <WinSock2.h>
#elif _WIN32
#include <winsock.h>
#endif
#endif
#include <iostream>
#include <mysql.h>
#include <chrono>
using namespace std;
using namespace chrono;
class MysqlConn
{
public:
    // ��ʼ�����ݿ�����
    MysqlConn();
    // �ͷ����ݿ�����
    ~MysqlConn();
    // �������ݿ�
    bool connect(string user, string passwd, string dbName, string ip, unsigned short port = 3306);
    // �������ݿ�: insert, update, delete
    bool update(string sql);
    // ��ѯ���ݿ�
    bool query(string sql);
    // ������ѯ�õ��Ľ����
    bool next();
    // �õ�������е��ֶ�ֵ
    string value(int index);
    // �������
    bool transaction();
    // �ύ����
    bool commit();
    // ����ع� 
    bool rollback();
    // ˢ����ʼ�Ŀ���ʱ���
    void refreshAliveTime();
    // �������Ӵ�����ʱ��
    long long getAliveTime();
private:
    void freeResult();
    MYSQL* m_conn = nullptr;
    MYSQL_RES* m_result = nullptr;//��ѯ���
    MYSQL_ROW m_row = nullptr;//��ѯ��
    steady_clock::time_point m_alivetime;
};
#endif // !MYSQLCONN_H


