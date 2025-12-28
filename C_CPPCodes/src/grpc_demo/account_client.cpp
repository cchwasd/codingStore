
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "account.grpc.pb.h"

class LoginClient {
public:
	explicit LoginClient(std::shared_ptr<grpc::Channel> channel)
		: stub_(example::LoginService::NewStub(channel)) {}

	// 返回是否登录成功，并把服务器消息写入 out_message（可为 nullptr）
	bool Login(const std::string& username, const std::string& password, std::string* out_message) {
		example::LoginRequest request;
		request.set_username(username);
		request.set_password(password);

		example::LoginResponse response;
		grpc::ClientContext context;

		grpc::Status status = stub_->Login(&context, request, &response);
		if (!status.ok()) {
			if (out_message) *out_message = std::string("RPC failed: ") + status.error_message();
			return false;
		}
		if (out_message) *out_message = response.message();
		return response.success();
	}

private:
	std::unique_ptr<example::LoginService::Stub> stub_;
};

int main(int argc, char** argv) {
	std::string target = "localhost:50051";
	if (argc > 1) target = argv[1];

	std::string username = "admin";
	std::string password = "password";
	if (argc > 2) username = argv[2];
	if (argc > 3) password = argv[3];

	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	LoginClient client(channel);

	std::string server_msg;
	bool ok = client.Login(username, password, &server_msg);
	std::cout << "Login result: " << (ok ? "success" : "failure") << ", message=\"" << server_msg << "\"" << std::endl;

    std::unordered_map<std::string, std::string> users{
        {"admin2", "password2"},
        {"user1", "pass1"},
        {"user3", "pass3"}
    };
    // c++11
    for (auto it = users.begin(); it != users.end(); ++it) {
        const std::string& user = it->first;
        const std::string& pass = it->second;
        bool res = client.Login(user, pass, &server_msg);
        std::cout << "Login attempt for user \"" << user << "\": " << (res ? "success" : "failure") << ", message=\"" << server_msg << "\"" << std::endl;
    }

    for (const auto& elem : users) {
        bool res = client.Login(elem.first, elem.second, &server_msg);
        std::cout << "Login attempt for user \"" << elem.first << "\": " << (res ? "success" : "failure") << ", message=\"" << server_msg << "\"" << std::endl;
    }
    // C++17 及以上
    for (const auto& [user, pass] : users) {
        bool res = client.Login(user, pass, &server_msg);
        std::cout << "Login attempt for user \"" << user << "\": " << (res ? "success" : "failure") << ", message=\"" << server_msg << "\"" << std::endl;
    }

    // 使用 lambda 表达式处理每个元素
    std::for_each(users.begin(), users.end(),
        // 使用引用捕获所有外部变量
        [&client, &server_msg](const std::pair<std::string, std::string>& elem) {
            bool res = client.Login(elem.first, elem.second, &server_msg);
            std::cout << "Login attempt for user \"" << elem.first << "\": " << (res ? "success" : "failure") << ", message=\"" << server_msg << "\"" << std::endl;
        }
    );
	return ok ? 0 : 1;
}
