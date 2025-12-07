#include <iostream>
#include "mysql.h"

using namespace std;

int main(int argc, char *argv[])
{
    MYSQL mysql;    	// 创建数据库句柄
	mysql_init(&mysql); // 初始化句柄

	//MYSQL *mysql = mysql_init(nullptr);

    const char* host = "localhost";
    const char* user = "root";
    const char* password = "123456";
    const char* dbname = "test";
    unsigned int port = 3306;

	///< 连接的数据库（句柄、主机名、用户名、密码、数据库名、端口号、socket指针、标记）
	if (!mysql_real_connect(&mysql, host, user, password, dbname, port, nullptr, 0))
	{
		cout << "数据库连接失败:" << mysql_errno(&mysql) << mysql_error(&mysql) << endl;
		return -1;
	}

	cout << "数据库连接成功" << endl << endl;

	///< 创建数据库回应结构体
	MYSQL_RES *res = nullptr;
	///< 创建存放结果的结构体
	MYSQL_ROW row;

	char sql[1024]{ 0 };
	sprintf_s(sql, 1024, "SELECT * FROM user;");

	///< 调用查询接口
	if (mysql_real_query(&mysql, sql, (unsigned int)strlen(sql)))
	{
		cout << "查询失败" << ": "  << mysql_errno(&mysql) << endl;
	}
	else
	{
		cout << "查询成功" << endl << endl;

		///< 装载结果集
		res = mysql_store_result(&mysql);

		if (nullptr == res)
		{
			cout << "装载数据失败" << ": " << mysql_errno(&mysql)  << endl;
		}
		else
		{
			///< 取出结果集中内容
			while (row = mysql_fetch_row(res))
			{
				cout << row[0] << "  "  << row[1] << endl;
			}
		}
	}

	///< 释放结果集
	mysql_free_result(res);

	///< 关闭数据库连接
	mysql_close(&mysql);

    std::getchar();

	return 0;
}
