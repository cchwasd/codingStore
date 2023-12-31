// WSqlite.cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "WSqlite.h"


using namespace std;

WSqlite::WSqlite()
{
    pDb = NULL;
}

WSqlite::~WSqlite()
{
    Destory();
}

void WSqlite::Destory()
{
    if (pDb)
    {
        sqlite3_close_v2(pDb);
        pDb = NULL;
        cout << "关闭数据库连接" << endl;
    }
}


bool WSqlite::Opendb(const string &path)
{
    // 根据文件路径打开数据库连接。如果数据库不存在，则创建。
    // 数据库文件的路径必须以C字符串传入。
    int result = sqlite3_open_v2(path.c_str(), &pDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);

    if (result == SQLITE_OK)
    {
        cout << "数据库连接成功:" << path << endl;
        return true;
    }
    else
    {
        cout << "打开数据库连接失败" << endl;
        return false;
    }
}


bool WSqlite::CreateTable(const string &sqlCreatetable)
{
    char *szMsg = NULL;
    int ret = sqlite3_exec(pDb, sqlCreatetable.c_str(), NULL, NULL, &szMsg);
    if (szMsg)
    {
        sqlite3_free(szMsg);
    }
    if (ret != SQLITE_OK)
    {
        cout << "建表时失败" << endl;
        return false;
    }
    return true;
}


bool WSqlite::Insert(const string &sqlInsert)
{
    if (sqlInsert.empty())
    {
        return -1;
    }

    char *zErrMsg = NULL;
    int ret = sqlite3_exec(pDb, sqlInsert.c_str(), NULL, NULL, &zErrMsg);
    if (zErrMsg)
    {
        sqlite3_free(zErrMsg);
    }
    if (ret != SQLITE_OK)
    {
        cout << "插入时失败" << endl;
        return false;
    }
    cout << "数据插入成功" << endl;
    return true;
}

bool WSqlite::Delete(const string &sqlDelete)
{
    int nCols = 0;
    int nRows = 0;
    char **azResult = NULL;
    char *errMsg = NULL;
    int res = sqlite3_get_table(pDb, sqlDelete.c_str(), &azResult, &nRows, &nCols, &errMsg);
    if (res != SQLITE_OK)
    {
        return false;
    }
    if (azResult)
    {
        sqlite3_free_table(azResult);
    }
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return true;
}

bool WSqlite::Update(const string &sqlUpdate)
{
   sqlite3_stmt *stmt = NULL; // stmt语句句柄
    // sqlite3_prepare_v2将sql文本转换成一个准备语句（prepared statement）对象，同时返回这个对象的指针
    // nByte小于0，则函数取出zSql中从开始到第一个0终止符的内容,
    // 如果nByte不是负的，那么它就是这个函数能从zSql中读取的字节数的最大值
    int result = sqlite3_prepare_v2(pDb, sqlUpdate.c_str(), -1, &stmt, NULL);

    if (result == SQLITE_OK)
    {
        // 执行该语句
        sqlite3_step(stmt);
        // 清理语句句柄，准备执行下一个语句
        sqlite3_finalize(stmt);
        cout << "更新数据成功！" << endl;
        return true;
    }
    else
    {
        cout << "更新数据失败！" << endl;
        // 清理语句句柄
        sqlite3_finalize(stmt);
        return false;
    }
 
}


bool WSqlite::IsTableExist(const string &dbName)
{
    sqlite3_stmt *stmt = NULL; // stmt语句句柄
    char selectsql[256]={0};
    int count = 0;

    snprintf(selectsql, sizeof(selectsql), "select count(*) from sqlite_master where type='table' and name = '%s'", dbName.c_str());

    // 进行查询前的准备工作——检查语句合法性
    //-1代表系统会自动计算SQL语句的长度
    int result = sqlite3_prepare_v2(pDb, selectsql, -1, &stmt, NULL);
    if (result == SQLITE_OK)
    {
        // 每调一次sqlite3_step()函数，stmt语句句柄就会指向下一条记录
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // 取出第0列字段的值
            count = sqlite3_column_int(stmt, 0);
        }
        // 清理语句句柄，准备执行下一个语句
        sqlite3_finalize(stmt);
    }
    else
    {
        cout << "查询语句有问题! " << endl;
        sqlite3_finalize(stmt);
    }

    if (count)
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool WSqlite::QueryData(const string &sqlQuery, vector<student> &studentDataVector)
{
    if (sqlQuery.empty())
    {
        cout << "Query sql is empty!" << endl;
        return -1;
    }

    int nCols = -1;         // 列数
    int nRows = -1;         // 行数
    char **azResult = NULL; // 结果集
    char *errMsg = NULL;
    int index = 0;
    const int ret = sqlite3_get_table(pDb, sqlQuery.c_str(), &azResult, &nRows, &nCols, &errMsg);

    index = nCols;
    studentDataVector.clear();
    studentDataVector.reserve(nCols); // 改变容器容量，避免内存重新分配

    // bool bKeyCaptured = false;
    if (nRows <= 0 && nCols <= 0)
    {
        cout << "Query Data is empty!" << endl;
        return -1;
    }

    map<string, int> control_param = {
        { "id", 1 },
        { "name", 2 },
        { "age",3 },
        { "sex",4 },
        { "phone",5 },
        { "address",6 }
    };

    for (int i = 0; i < nRows; i++)
    {
 
        student stu={id:0,name:"",age:0,phone:"",address:""};
        for (int j = 0; j < nCols; j++)
        {
            int caseKey = control_param[azResult[j]];
            switch (caseKey)
            {
            case 1:
                stu.id = atoi(azResult[index]);
                break;
            case 2:
                stu.name = azResult[index];
                break;
            case 3:
                stu.age = atoi(azResult[index]);
                break;
            case 4:
                stu.sex = atoi(azResult[index]);
                break;
            case 5:
                stu.phone = azResult[index];
                break;
            case 6:
                stu.address = azResult[index];
                break;
            default:
                break;
            }
            index++;
        }
        
        studentDataVector.push_back(stu);
    }

    if (azResult)
    {
        // 使用完 sqlite3_get_table 后务必释放为记录分配的内存，否则会内存泄漏
        sqlite3_free_table(azResult);
    }
    if (errMsg)
    {
        sqlite3_free(errMsg);
    }
    return ret;
}

