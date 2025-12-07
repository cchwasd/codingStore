#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include "mysql.h"

// 参考：https://gitee.com/fingsinz/my-sql-cpp/tree/master/
struct User {
    int id;
    std::string username;
    std::string password;
};

class SqlBase {
public:
    SqlBase(const char* host, const char* user, const char* password, const char* dbname, unsigned int port)
        : host_(host), user_(user), password_(password), dbname_(dbname), port_(port) {
        mysql_init(&mysql_);
    }

    ~SqlBase() {
        mysql_close(&mysql_);
    }

    bool connect() {
        if (!mysql_real_connect(&mysql_, host_, user_, password_, dbname_, port_, nullptr, 0)) {
            std::cerr << "Database connection failed: " << mysql_errno(&mysql_) << " " << mysql_error(&mysql_) << std::endl;
            return false;
        }
        return true;
    }

    bool addUser(const User& user) {
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO user (username, password) VALUES ('%s', '%s');", user.username.c_str(), user.password.c_str());
        return executeQuery(sql);
    }

    bool deleteUser(int id) {
        char sql[128];
        snprintf(sql, sizeof(sql), "DELETE FROM user WHERE id = %d;", id);
        return executeQuery(sql);
    }

    bool updateUser(const User& user) {
        char sql[256];
        snprintf(sql, sizeof(sql), "UPDATE user SET username = '%s', password = '%s' WHERE id = %d;", user.username.c_str(), user.password.c_str(), user.id);
        return executeQuery(sql);
    }

    std::vector<User> getUsers() {
        std::vector<User> users;
        const char* sql = "SELECT id, username, password FROM user;";
        if (mysql_real_query(&mysql_, sql, strlen(sql)) == 0) {
            MYSQL_RES* res = mysql_store_result(&mysql_);
            if (res) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    User user;
                    user.id = std::stoi(row[0]);
                    user.username = row[1];
                    user.password = row[2];
                    users.push_back(user);
                }
                mysql_free_result(res);
            }
        } else {
            std::cerr << "Query failed: " << mysql_errno(&mysql_) << " " << mysql_error(&mysql_) << std::endl;
        }
        return users;
    }

private:
    bool executeQuery(const char* sql) {
        if (mysql_real_query(&mysql_, sql, strlen(sql)) != 0) {
            std::cerr << "Query execution failed: " << mysql_errno(&mysql_) << " " << mysql_error(&mysql_) << std::endl;
            return false;
        }
        return true;
    }

    MYSQL mysql_;
    const char* host_;
    const char* user_;
    const char* password_;
    const char* dbname_;
    unsigned int port_;
};

int main() {
    const char* host = "localhost";
    const char* user = "root";
    const char* password = "123456";
    const char* dbname = "test";
    unsigned int port = 3306;

    SqlBase sqlBase(host, user, password, dbname, port);

    if (!sqlBase.connect()) {
        std::cerr << "Failed to connect to the database." << std::endl;
        return -1;
    }

    // Add a new user
    User newUser = {3, "小明", "password123"};
    if (sqlBase.addUser(newUser)) {
        std::cout << "User added successfully." << std::endl;
    }
    newUser = {4, "小红", "password123"};
    if (sqlBase.addUser(newUser)) {
        std::cout << "User added successfully." << std::endl;
    }

    // Retrieve and display all users
    std::vector<User> users = sqlBase.getUsers();
    std::cout << "Users in the database:" << std::endl;
    for (const auto& user : users) {
        std::cout << "ID: " << user.id << ", Username: " << user.username << ", Password: " << user.password << std::endl;
    }

    // Update a user
    if (!users.empty()) {
        User updateUser = users.back();
        updateUser.username = "updated_user";
        updateUser.password = "new_password123";
        if (sqlBase.updateUser(updateUser)) {
            std::cout << "User updated successfully." << std::endl;
        }
    }

    // Delete a user
    if (!users.empty()) {
        int userIdToDelete = users.back().id;
        if (sqlBase.deleteUser(userIdToDelete)) {
            std::cout << "User deleted successfully." << std::endl;
        }
    }

    return 0;
}