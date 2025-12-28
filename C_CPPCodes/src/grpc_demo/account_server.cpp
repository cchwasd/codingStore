

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "account.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class LoginServiceImpl final : public example::LoginService::Service {
public:
    LoginServiceImpl() {
        users_db = {
            {"admin", "password"},
            {"user1", "pass1"},
            {"user2", "pass2"}
        };
    }
	Status Login(ServerContext* context, const example::LoginRequest* request,
				 example::LoginResponse* response) override {
		const std::string user = request->username();
		const std::string pass = request->password();

		// if (user == "admin" && pass == "password") {
		// 	response->set_message("Welcome " + user + "!");
		// 	response->set_success(true);
		// } else {
		// 	response->set_message("Invalid credentials");
		// 	response->set_success(false);
		// }
        auto it = users_db.find(user);
        if (it != users_db.end() && it->second == pass) {
            response->set_message("Welcome " + user + "!");
            response->set_success(true);
        } else {
            response->set_message("Invalid credentials");
            response->set_success(false);
        }
		return Status::OK;
	}
private:
    std::unordered_map<std::string, std::string> users_db;
};

void RunServer(const std::string& server_address) {
	LoginServiceImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

int main(int argc, char** argv) {
	std::string addr = "0.0.0.0:50051";
	if (argc > 1) addr = argv[1];
	RunServer(addr);
	return 0;
}

