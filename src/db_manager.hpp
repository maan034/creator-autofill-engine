#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DatabaseManager {
private:
    std::string supabase_url;
    std::string supabase_key;

public:
    DatabaseManager(const std::string& url, const std::string& key) 
        : supabase_url(url), supabase_key(key) {}

    bool test_connection() {
        if (!supabase_url.empty() && !supabase_key.empty()) {
            std::cout << "[DB] Supabase Engine configured successfully!" << std::endl;
            return true;
        }
        std::cerr << "[DB Error] Supabase URL or Key missing!" << std::endl;
        return false;
    }

    bool register_user(const std::string& email, const std::string& username, const std::string& password_hash) {
        try {
            json body = {
                {"email", email},
                {"username", username},
                {"password_hash", password_hash}
            };

            std::ofstream out("temp_payload.json");
            out << body.dump();
            out.close();

            std::string target_endpoint = supabase_url + "/rest/v1/users";
            std::string command = "curl -s -X POST \"" + target_endpoint + "\" "
                + "-H \"apikey: " + supabase_key + "\" "
                + "-H \"Authorization: Bearer " + supabase_key + "\" "
                + "-H \"Content-Type: application/json\" "
                + "-H \"Prefer: return=representation\" "
                + "-d @temp_payload.json -o res_user.json";

            std::cout << "[DB Log] Registering User..." << std::endl;
            int result = std::system(command.c_str());

            std::ifstream res_file("res_user.json");
            json response_json;
            if (res_file >> response_json) {
                if (response_json.is_object() && response_json.contains("code")) {
                    std::cerr << "[DB Error] " << response_json.dump() << std::endl;
                    return false;
                }
            }
            return (result == 0);
        } catch (const std::exception& e) {
            std::cerr << "[DB Insert Error] " << e.what() << std::endl;
            return false;
        }
    }

    bool save_creator_profile(const std::string& user_id, const std::string& instagram, int followers, const std::string& address) {
        try {
            // Matched exact schema column names from Supabase creator_profiles table
            json body = {
                {"user_id", user_id},
                {"instagram_handle", instagram},
                {"instagram_followers", followers},
                {"encrypted_shipping_address", address}
            };

            std::ofstream out("temp_profile.json");
            out << body.dump();
            out.close();

            std::string target_endpoint = supabase_url + "/rest/v1/creator_profiles";
            std::string command = "curl -s -X POST \"" + target_endpoint + "\" "
                + "-H \"apikey: " + supabase_key + "\" "
                + "-H \"Authorization: Bearer " + supabase_key + "\" "
                + "-H \"Content-Type: application/json\" "
                + "-H \"Prefer: return=representation\" "
                + "-d @temp_profile.json -o res_profile.json";

            std::cout << "[DB Log] Saving Creator Profile..." << std::endl;
            int result = std::system(command.c_str());

            std::ifstream res_file("res_profile.json");
            json response_json;
            if (res_file >> response_json) {
                if (response_json.is_object() && response_json.contains("code")) {
                    std::cerr << "[DB Schema Error] " << response_json.dump() << std::endl;
                    return false;
                }
            }

            return (result == 0);
        } catch (const std::exception& e) {
            std::cerr << "[DB Insert Error] " << e.what() << std::endl;
            return false;
        }
    }
};