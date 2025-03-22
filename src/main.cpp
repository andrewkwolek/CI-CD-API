#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;

// Simple in-memory data store for our REST API
class DataStore {
private:
    std::mutex mutex_;
    std::unordered_map<std::string, json::object> items_;
    int next_id_ = 1;

public:
    // Get all items
    json::array get_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        json::array result;
        for (const auto& pair : items_) {
            result.push_back(pair.second);
        }
        return result;
    }
    
    // Get single item by ID
    std::optional<json::object> get(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = items_.find(id);
        if (it != items_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    // Create new item
    json::object create(const json::object& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::string id = std::to_string(next_id_++);
        json::object new_item = item;
        new_item["id"] = id;
        
        items_[id] = new_item;
        return new_item;
    }
    
    // Update existing item
    bool update(const std::string& id, const json::object& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (items_.find(id) == items_.end()) {
            return false;
        }
        
        json::object updated_item = item;
        updated_item["id"] = id;
        items_[id] = updated_item;
        return true;
    }
    
    // Delete item
    bool remove(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        return items_.erase(id) > 0;
    }
};

// Global data store instance
DataStore data_store;

// This function parses the target to extract path segments and query parameters
std::pair<std::vector<std::string>, std::unordered_map<std::string, std::string>> 
parse_request_target(beast::string_view target) {
    std::vector<std::string> path_segments;
    std::unordered_map<std::string, std::string> query_params;
    
    // Find the query string
    auto query_pos = target.find('?');
    beast::string_view path = query_pos == beast::string_view::npos ? 
                              target : target.substr(0, query_pos);
    
    // Parse path segments
    size_t pos = 0;
    while (pos < path.size()) {
        if (path[pos] == '/') {
            pos++;
            continue;
        }
        
        size_t end = path.find('/', pos);
        if (end == beast::string_view::npos) {
            end = path.size();
        }
        
        if (end > pos) {
            path_segments.push_back(std::string(path.substr(pos, end - pos)));
        }
        
        pos = end;
    }
    
    // Parse query parameters if present
    if (query_pos != beast::string_view::npos && query_pos + 1 < target.size()) {
        beast::string_view query = target.substr(query_pos + 1);
        pos = 0;
        
        while (pos < query.size()) {
            size_t end = query.find('&', pos);
            if (end == beast::string_view::npos) {
                end = query.size();
            }
            
            size_t equals_pos = query.find('=', pos);
            if (equals_pos != beast::string_view::npos && equals_pos < end) {
                std::string key(query.substr(pos, equals_pos - pos));
                std::string value(query.substr(equals_pos + 1, end - equals_pos - 1));
                query_params[key] = value;
            }
            
            pos = end + 1;
        }
    }
    
    return {path_segments, query_params};
}

// This function handles routing to the appropriate API endpoints
template<class Body, class Allocator>
http::response<http::string_body>
handle_request(http::request<Body, http::basic_fields<Allocator>>&& req)
{
    // Returns a bad request response
    auto const bad_request =
    [&req](beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        json::object error_obj;
        error_obj["error"] = std::string(why);
        res.body() = json::serialize(error_obj);
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found =
    [&req](beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        json::object error_obj;
        error_obj["error"] = "The resource '" + std::string(target) + "' was not found.";
        res.body() = json::serialize(error_obj);
        res.prepare_payload();
        return res;
    };

    // Request path must be absolute and not contain "..".
    if(req.target().empty() ||
       req.target()[0] != '/' ||
       req.target().find("..") != beast::string_view::npos)
        return bad_request("Illegal request-target");

    // Parse the request target
    auto [path_segments, query_params] = parse_request_target(req.target());
    
    // Initialize the response
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    
    try {
        // Basic routing based on path segments and HTTP method
        if (path_segments.empty() || (path_segments.size() == 1 && path_segments[0] == "api")) {
            // Root or /api path - return API info
            json::object api_info;
            api_info["name"] = "Boost.Beast REST API Example";
            api_info["version"] = "1.0";
            api_info["endpoints"] = {
                "/api/items - GET (list all), POST (create new)",
                "/api/items/{id} - GET (retrieve), PUT (update), DELETE (remove)"
            };
            res.body() = json::serialize(api_info);
        }
        else if (path_segments.size() >= 2 && path_segments[0] == "api" && path_segments[1] == "items") {
            if (path_segments.size() == 2) {
                // /api/items endpoint
                if (req.method() == http::verb::get) {
                    // GET /api/items - List all items
                    json::array items = data_store.get_all();
                    res.body() = json::serialize(items);
                }
                else if (req.method() == http::verb::post) {
                    // POST /api/items - Create new item
                    auto json_data = json::parse(req.body()).as_object();
                    auto new_item = data_store.create(json_data);
                    res.result(http::status::created);
                    res.body() = json::serialize(new_item);
                }
                else {
                    res.result(http::status::method_not_allowed);
                    json::object error;
                    error["error"] = "Method not allowed";
                    res.body() = json::serialize(error);
                }
            }
            else if (path_segments.size() == 3) {
                // /api/items/{id} endpoint
                std::string id = path_segments[2];
                
                if (req.method() == http::verb::get) {
                    // GET /api/items/{id} - Get item by ID
                    auto item = data_store.get(id);
                    if (item) {
                        res.body() = json::serialize(*item);
                    }
                    else {
                        return not_found(req.target());
                    }
                }
                else if (req.method() == http::verb::put) {
                    // PUT /api/items/{id} - Update item
                    auto json_data = json::parse(req.body()).as_object();
                    bool success = data_store.update(id, json_data);
                    if (success) {
                        auto updated_item = data_store.get(id);
                        res.body() = json::serialize(*updated_item);
                    }
                    else {
                        return not_found(req.target());
                    }
                }
                else if (req.method() == http::verb::delete_) {
                    // DELETE /api/items/{id} - Delete item
                    bool success = data_store.remove(id);
                    if (success) {
                        json::object result;
                        result["success"] = true;
                        result["message"] = "Item deleted";
                        res.body() = json::serialize(result);
                    }
                    else {
                        return not_found(req.target());
                    }
                }
                else {
                    res.result(http::status::method_not_allowed);
                    json::object error;
                    error["error"] = "Method not allowed";
                    res.body() = json::serialize(error);
                }
            }
            else {
                return not_found(req.target());
            }
        }
        else {
            return not_found(req.target());
        }
    }
    catch (const std::exception& e) {
        return bad_request(std::string("Error processing request: ") + e.what());
    }
    
    res.prepare_payload();
    return res;
}

// This is the C++11 equivalent of a generic lambda.
// The function object is used to send an HTTP message.
struct send_lambda
{
    beast::tcp_stream& stream_;
    bool& close_;
    beast::error_code& ec_;

    send_lambda(
        beast::tcp_stream& stream,
        bool& close,
        beast::error_code& ec)
        : stream_(stream)
        , close_(close)
        , ec_(ec)
    {
    }

    template<bool isRequest, class Body, class Fields>
    void
    operator()(http::message<isRequest, Body, Fields>&& msg) const
    {
        // Determine if we should close the connection after
        close_ = msg.need_eof();

        // Write the response
        http::serializer<isRequest, Body, Fields> sr{msg};
        http::write(stream_, sr, ec_);
    }
};

// Handles an HTTP server connection
void handle_session(beast::tcp_stream& stream)
{
    bool close = false;
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    // This lambda is used to send messages
    send_lambda lambda{stream, close, ec};

    for(;;)
    {
        // Read a request
        http::request<http::string_body> req;
        http::read(stream, buffer, req, ec);
        if(ec == http::error::end_of_stream)
            break;
        if(ec)
            return;

        // Send the response
        lambda(handle_request(std::move(req)));
        if(ec)
            return;

        if(close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            break;
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);
    // At this point the connection is closed gracefully
}

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 3)
        {
            std::cerr << "Usage: rest-api-server <address> <port>\n";
            std::cerr << "Example:\n";
            std::cerr << "    rest-api-server 0.0.0.0 8080\n";
            return EXIT_FAILURE;
        }
        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};

        std::cout << "REST API Server started at: " << address << ":" << port << std::endl;
        std::cout << "Endpoints:" << std::endl;
        std::cout << "  GET    /api            - API information" << std::endl;
        std::cout << "  GET    /api/items      - List all items" << std::endl;
        std::cout << "  POST   /api/items      - Create a new item" << std::endl;
        std::cout << "  GET    /api/items/{id} - Get an item by ID" << std::endl;
        std::cout << "  PUT    /api/items/{id} - Update an item" << std::endl;
        std::cout << "  DELETE /api/items/{id} - Delete an item" << std::endl;

        for(;;)
        {
            // This will receive the new connection
            tcp::socket socket{ioc};
            
            // Block until we get a connection
            acceptor.accept(socket);
            
            // Launch the session, transferring ownership of the socket
            std::thread{[q = std::move(socket)]() mutable {
                beast::tcp_stream stream{std::move(q)};
                handle_session(stream);
            }}.detach();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}