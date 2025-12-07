// WSqlite.h
#ifndef WSQLITE_H_
#define WSQLITE_H_
 
#include <string>
#include <vector>
#include <iostream>
#include "sqlite3.h"

using namespace std;
 
class WSqlite {
 
public:
	WSqlite();
	~WSqlite();
 
	int CreateDbFile(const string &path);	// 创建数据库文件
	int CreateTable(const string& sqlCreatetable);	// 创建数据库表
	int Opendb(const string& path);		// 连接数据库
	int Insert(const string& sqlInsert);	// 增
	int Delete(const string& sqlDelete);	// 删
	int Update(const string& sqlUpdate);	// 改
	int QueryData(const string& sqlQuery, vector<string> &arrKey, vector<vector<string>> &arrValue);	// 查
 
private:
	sqlite3* pDb = NULL;
 
private:
	//sqlie对象的销毁放在析构里，不需要用户关心
	void Destory();
};
 
#endif WSQLITE_H_