#ifndef LOG_H
#define LOG_H
#define LOG(str) \
	cout << __FILE__ << ":" << __LINE__ << " " << \
	__TIMESTAMP__ << " : " << str << endl;
#endif // !LOG_H

