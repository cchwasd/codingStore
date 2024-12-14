// WSqlite.h
#ifndef WSQLITE_H_
#define WSQLITE_H_

#include <string>
#include <vector>
#include <iostream>
#include "sqlite3/sqlite3.h"


using namespace std;

typedef struct
{
    int id;
    string name;
    int age;
    int sex;
    string phone;
    string address;
} student;

class WSqlite
{
public:
    WSqlite();
	~WSqlite();

	bool CreateTable(const string& sqlCreatetable);	// 创建数据库表
    bool IsTableExist(const string &dbName);        // 检查表是否已存在
    
	bool Opendb(const string& path);		// 连接数据库,如果不存在库，则创建
	bool Insert(const string& sqlInsert);	// 增
	bool Delete(const string& sqlDelete);	// 删
	bool Update(const string& sqlUpdate);	// 改
	bool QueryData(const string& sqlQuery, vector<student> &studentDataVector);	// 查
    
private:
    //sqlie对象的销毁放在析构里，不需要用户关心
	void Destory();

    sqlite3* pDb;  // 数据库链接指针

};

#endif WSQLITE_H_