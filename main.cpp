#include "crow.h"
#include <nlohmann/json.hpp>
#include "src/db_manager.hpp"
#include <iostream>

using json = nlohmann::json;

const std::string SUPABASE_URL = "https://aogqqciaztaxtjpeejhv.supabase.co";
const std::string SUPABASE_ANON_KEY = "sb_publishable_M6Pq1Ig8YDnDow6AkxIGYw_AyRrIuLG";

int main() {
    crow::SimpleApp app;
    DatabaseManager db(SUPABASE_URL, SUPABASE_ANON_KEY);

    db.test_connection();

    // 1. Health Check Route
    CROW_ROUTE(app, "/health")([](){
        json res = {
            {"status", "success"},
            {"message", "C++ Engine is running!"}
        };
        return crow::response(200, res.dump());
    });

    // 2. User Registration Route
    CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::POST)([&db](const crow::request& req){
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("email") || !body.contains("username") || !body.contains("password")) {
            return crow::response(400, "Invalid JSON body");
        }

        std::string email = body["email"];
        std::string username = body["username"];
        std::string password = body["password"];

        bool success = db.register_user(email, username, password);

        if (success) {
            json res = {{"status", "success"}, {"message", "User registered successfully!"}};
            return crow::response(201, res.dump());
        } else {
            json res = {{"status", "error"}, {"message", "Failed to register user"}};
            return crow::response(500, res.dump());
        }
    });

    // 3. Creator Profile Vault Route
    CROW_ROUTE(app, "/api/profile").methods(crow::HTTPMethod::POST)([&db](const crow::request& req){
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("user_id") || !body.contains("instagram_handle")) {
            return crow::response(400, "Invalid JSON payload: user_id and instagram_handle required.");
        }

        std::string user_id = body["user_id"];
        std::string instagram = body["instagram_handle"];
        int followers = body.value("instagram_followers", 0);
        std::string address = body.value("encrypted_shipping_address", "");

        bool success = db.save_creator_profile(user_id, instagram, followers, address);

        if (success) {
            json res = {{"status", "success"}, {"message", "Creator Profile saved successfully!"}};
            return crow::response(201, res.dump());
        } else {
            json res = {{"status", "error"}, {"message", "Failed to save profile due to schema/db error"}};
            return crow::response(500, res.dump());
        }
    });

// 4. ROUTE: Trigger Browser Form Auto-Fill Automation
    CROW_ROUTE(app, "/api/autofill").methods(crow::HTTPMethod::POST)([](const crow::request& req){
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("user_id") || !body.contains("target_url")) {
            return crow::response(400, "Invalid JSON: user_id and target_url are required.");
        }

        std::string user_id = body["user_id"];
        std::string target_url = body["target_url"];

        std::cout << "[AutoFill Engine] Triggering automation for user: " << user_id << " on URL: " << target_url << std::endl;

        // Use 'py' launcher for Windows Python execution
        std::string automation_command = "py automation/fill_form.py --url \"" + target_url + "\" --user \"" + user_id + "\"";
        int result = std::system(automation_command.c_str());

        if (result == 0) {
            json res = {{"status", "success"}, {"message", "Form auto-filled successfully via browser automation!"}};
            return crow::response(200, res.dump());
        } else {
            json res = {{"status", "error"}, {"message", "Automation script execution failed."}};
            return crow::response(500, res.dump());
        }
    });

    std::cout << "Starting C++ Server on http://localhost:8080..." << std::endl;
    app.port(8080).multithreaded().run();
}