#include <iostream>
#include <string>
#include "MysqlConn.h"
#include "ConnectionPool.h"
#include <thread>
#include <chrono>
#define debugtest 1
using namespace std;

int main()
{
#if debugtest
	// 设置开始时间
	auto start = std::chrono::system_clock::now();
	std::thread test([]()
		{
			
			for (int i = 0; i < 1000; ++i)
			{
				MysqlConn* conn = new MysqlConn;
				conn->connect("root", "cms241583", "testdb", "127.0.0.1");
				string sql = "insert into student values('10','tom')";
				conn->update(sql);
				cout << i << endl;
			}
		});
	test.join();
	auto end = std::chrono::system_clock::now();
	// 精确到微秒，除此之外，还有五种时间单位：hours, minutes, seconds, milliseconds, nanoseconds
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	// duration.count() 返回统计的时间
	// num 和 den分别表示分子(numerator)和分母(denominator)
	// 在代码中，num等于1， den等于1,000,000
	std::cout << double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << std::endl;
#else
		auto start = std::chrono::system_clock::now();
	std::thread test([]()
		{
			ConnectionPool* pool = ConnectionPool::getConnectionPool();
			for (int i = 0; i < 1000; ++i)
			{
				
				shared_ptr<MysqlConn> sp = pool->getConnection();
				sp->update("insert into student values('10','tom')");
				cout << i << endl;
			}
		});
	test.join();
	auto end = std::chrono::system_clock::now();
	// 精确到微秒，除此之外，还有五种时间单位：hours, minutes, seconds, milliseconds, nanoseconds
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	// duration.count() 返回统计的时间
	// num 和 den分别表示分子(numerator)和分母(denominator)
	// 在代码中，num等于1， den等于1,000,000
	std::cout << double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << std::endl;

#endif
	return 0;
}