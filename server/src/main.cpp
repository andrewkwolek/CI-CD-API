#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <string>

void RunServer() {
    std::string server_address("0.0.0.0:8082");

    grpc::ServerBuilder builder;
}

int main(char* argc, char** argv) {
    return 0;
}