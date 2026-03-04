#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace digipets {

class User {
public:
    User(const std::string& username, const std::string& password);
    User() = default;

    // Getters
    const std::string& get_id() const { return id_; }
    const std::string& get_username() const { return username_; }
    std::chrono::system_clock::time_point get_created_at() const { return created_at_; }

    // Authentication
    bool verify_password(const std::string& password) const;
    void update_password(const std::string& new_password);

    // Serialization
    nlohmann::json to_json() const;
    static User from_json(const nlohmann::json& j);

private:
    std::string id_;
    std::string username_;
    std::string salt_;           // Random hex-encoded salt per user
    std::string password_hash_;  // SHA-256(salt + password)
    std::chrono::system_clock::time_point created_at_;

    static std::string generate_uuid();
    static std::string generate_salt();
    std::string hash_password(const std::string& password) const;
};

// Input validation helpers
bool is_valid_username(const std::string& username);
bool is_valid_password(const std::string& password);

} // namespace digipets
