#include <iostream>
#include "WSqlite2.h"
#include <cstring>
using namespace std;

int main()
{
    string path = "./student.db";
    WSqlite sqlOperate;
    string file_name(path); // 数据库的路径
    sqlOperate.Opendb(file_name);
    
    int result = sqlOperate.CreateDbFile(path);
	if (result != SQLITE_OK){
		cout << "文件创建失败！" << endl;
	}
    else
    {
        cout << "表不存在" << endl;
        // return -1;
        cout << "创建表 students" << endl;
	string ceateSql = "create table if not exists students(\
		id integer primary key,\
		name varchar(32),\
		age integer,\
		phone varchar(11),\
		address text\
	);";
	result = sqlOperate.CreateTable(ceateSql);
    if (result != SQLITE_OK)
		{
			cout << "表创建失败！" <<endl;
		}
		else
		{
			cout << "表创建成功！" << endl;
		}
    }
    
    // 单行插入
    // string insertSql("insert into students(id,name,age,phone,address)values(1,'zhangsan',10,'12345678','beijing');");
    // 多行插入
    string insertSql = "insert into students(id,name,age,phone,address)values(1,'zhangsan',10,'12345678','beijing'),\
											(2,'lisi',11,'22345678','nanjing'),\
											(3,'wangwu',12,'32345678','shanghai'),\
											(4,'liwu',13,'42345678','guangzhou');";
    // 插入数据
    sqlOperate.Insert(insertSql);
    // vector<student> stuVec;
    string updateSql = "update students set age=24 where name='lisi'";
    sqlOperate.Update(insertSql);

    string deleteSql = "delete from students where name='liwu'";
    sqlOperate.Delete(deleteSql);

    string selectSql("select * from students;");
    vector<string> arrKey;
	vector<vector<string>> arrValue;
    arrKey.clear();
	arrValue.clear();
	result = sqlOperate.QueryData(selectSql, arrKey, arrValue);
 
	if (result == SQLITE_OK && !arrKey.empty() && !arrValue.empty())
	{
		cout << "整个数据库查询结果：\n";
		for(int i1=0;i1<arrValue.size();i1++ ){
			for(int i2=0;i2<arrValue[0].size();i2++ ){
                cout << arrKey[i2] <<":"<< "\t";
                cout << arrValue[i1][i2] << "\t";
			}
            cout << "\n";
		}
	}
	else{
        cout << "查询时文件打开失败" << endl;
	}
    
    
    return 0;
}

// g++ src\sqlite_demo.cpp src\WSqlite.cpp -o sqlite_demo -I include -L include\sqlite3\lib -lsqlite3 -g -Wall -std=c++11

