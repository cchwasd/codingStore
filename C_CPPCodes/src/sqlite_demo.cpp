#include <iostream>
#include "WSqlite.h"

using namespace std;

int main()
{
    string path = "./student.db";
    WSqlite sqlOperate;
    string file_name(path); // 数据库的路径
   
    int result =  sqlOperate.Opendb(file_name);
	
   // 检查表是否已创建，一般可以忽略这一步，做出来只是为了学习
    if (sqlOperate.IsTableExist("students"))
    {
        cout << "表已存在!" << endl;
    }
    else
    {
        cout << "表不存在，创建表 students" << endl;
	    string ceateSql = "create table if not exists students(\
            id integer primary key autoincrement,\
            name varchar(32) not null,\
            age integer null,\
            sex integer check(sex in (0,1)),\
            phone varchar(11),\
            address text\
	    );";
        result = sqlOperate.CreateTable(ceateSql);
    }
   
    // 单行插入
    // string insertSql("insert into students(id,name,age,phone,address)values(1,'zhangsan',10,'12345678','beijing');");
    // 多行插入
    string insertSql = "insert into students(name,age,sex,phone,address)values('zhangsan',10,0,'12345678','beijing'),\
											('lisi',11,1,'22345678','nanjing'),\
											('wangwu',12,1,'32345678','shanghai'),\
											('liwu',13,0,'42345678','guangzhou');";
    // 插入数据
    sqlOperate.Insert(insertSql);
    // vector<student> stuVec;
    string updateSql = "update students set age=24 where name='lisi'";
    sqlOperate.Update(updateSql);

    string deleteSql = "delete from students where name='liwu'";
    sqlOperate.Delete(deleteSql);
    insertSql.clear();
    insertSql="insert into students(name,age,sex,phone,address)values('zhaoliu',12,0,'12345678','beijing');";
    sqlOperate.Insert(insertSql);

    string selectSql("select * from students;");
    vector<student> stuVec;
	result = sqlOperate.QueryData(selectSql, stuVec);
    
    string sex="";
	for (size_t i = 0; i < stuVec.size(); i++)
    {
        sex=stuVec[i].sex==0 ? "男" : "女";
        cout << "id:" << stuVec[i].id << ", name:" << stuVec[i].name << ", age:"<< stuVec[i].age<<",sex: "<< sex << ", phone:" << stuVec[i].phone << ", address:" << stuVec[i].address << endl;
    }
    
    
    return 0;
}

// g++ src\sqlite_demo.cpp src\WSqlite.cpp -o sqlite_demo -I include -L include\sqlite3\lib -lsqlite3 -g -Wall -std=c++11

