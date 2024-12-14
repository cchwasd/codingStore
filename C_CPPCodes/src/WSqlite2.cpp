// WSqlite.cpp
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
 

#include "WSqlite2.h"
 
using namespace std;

 
WSqlite::WSqlite(){
    pDb = NULL;
}
 
WSqlite::~WSqlite(){
    Destory();
}
 
void WSqlite::Destory()
{
    if (pDb)
    {
        sqlite3_close(pDb);
        pDb = NULL;
    }
}
 
int WSqlite::CreateDbFile(const string& path)
{
    return sqlite3_open(path.c_str(), &pDb);
}
 
int WSqlite::CreateTable(const string& sqlCreatetable) {
    char* szMsg = NULL;
    return sqlite3_exec(pDb, sqlCreatetable.c_str(), NULL, NULL, &szMsg);
}
 
int WSqlite::Opendb(const string& path)
{
    return sqlite3_open(path.c_str(), &pDb);
}
 
int WSqlite::Insert(const string& sqlInsert)
{
    if (sqlInsert.empty()) {
        return -1;
    }
 
    char* zErrMsg = NULL;
    int ret = sqlite3_exec(pDb, sqlInsert.c_str(), NULL, NULL, &zErrMsg);
    if (zErrMsg) {
        sqlite3_free(zErrMsg);
    }
    return ret;
}
 
int WSqlite::Delete(const string& sqlDelete)
{
    int nCols = 0;
    int nRows = 0;
    char** azResult = NULL;
    char* errMsg = NULL;
    int res = sqlite3_get_table(pDb, sqlDelete.c_str(), &azResult, &nRows, &nCols, &errMsg);
    if (res != SQLITE_OK) {
        return false;
    }
 	if (azResult) {
        sqlite3_free_table(azResult);
    }
    if (errMsg) {
        sqlite3_free(errMsg);
    }
    return true;
}
 
int WSqlite::Update(const string& sqlUpdate)
{
    char* zErrMsg = NULL;
    int ret = sqlite3_exec(pDb, sqlUpdate.c_str(), NULL, NULL, &zErrMsg);
	if (zErrMsg) {
        sqlite3_free(zErrMsg);
    }
    return ret;
}
 
int WSqlite::QueryData(const string& sqlQuery, vector<string>& arrKey, vector<vector<string>>& arrValue)
{
    if (sqlQuery.empty()) {
        return -1;
    }
 
    int nCols = -1;
    int nRows = -1;
    char** azResult = NULL;
    char* errMsg = NULL;
    int index = 0;
	const int ret = sqlite3_get_table(pDb, sqlQuery.c_str(), &azResult, &nRows, &nCols, &errMsg);
 
    index = nCols;
    arrKey.clear();
    arrKey.reserve(nCols);// 改变容器容量，避免内存重新分配
    arrValue.clear();
    arrValue.reserve(nRows);
 
	bool bKeyCaptured = false;
    for (int i = 0; i < nRows; i++) {
        vector<string> temp;
        for (int j = 0; j < nCols; j++) {
            if (!bKeyCaptured) {
                arrKey.push_back(azResult[j]);
            }
            temp.push_back(azResult[index]);
            index++;
        }
        bKeyCaptured = true;
        arrValue.push_back(temp);
    }
          
    if (azResult) {
        sqlite3_free_table(azResult);
    }
    if (errMsg) {
        sqlite3_free(errMsg);
    }
    return ret;
}