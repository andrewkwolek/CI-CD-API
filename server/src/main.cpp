#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <string>
#include <map>

#include "data_transfer.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using data_transfer::Item;
using data_transfer::ItemTransfer;

class ItemTransferImpl final : public ItemTransfer::Service {
public:
    ItemTransferImpl() {}

    Status GetItem(ServerContext* context, const Item* request, Item* response) override {
        int id = request->id();
        std::string name = request->name();
    

        if (table.find(request->id()) != table.end()) {
            Item* item;
            item->set_id(id);
            item->set_name(table[id]);
            *response = *item;
            return Status::OK;
        }
        else {
            return Status(grpc::StatusCode::NOT_FOUND, "Item not found");
        }
    }

    Status SetItem(ServerContext* context, const Item* request, Item* response) override {
        table[request->id()] = request->name();
        *response = *request;

        return Status::OK;
    }

private:
    std::unordered_map<int, std::string> table;
};

void RunServer() {
    std::string server_address("0.0.0.0:8082");
    ItemTransferImpl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}