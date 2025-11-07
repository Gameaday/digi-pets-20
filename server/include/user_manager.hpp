#pragma once

#include "user.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <optional>

namespace digipets {

class UserManager {
public:
    UserManager() = default;

    // User management
    std::optional<std::string> register_user(const std::string& username, const std::string& password);
    std::optional<std::string> authenticate(const std::string& username, const std::string& password);
    std::optional<User> get_user(const std::string& user_id);
    std::optional<User> get_user_by_username(const std::string& username);
    bool user_exists(const std::string& username);

    // Persistence
    bool save_to_file(const std::string& filename);
    bool load_from_file(const std::string& filename);

private:
    std::unordered_map<std::string, User> users_by_id_;
    std::unordered_map<std::string, std::string> username_to_id_;
    mutable std::mutex mutex_;
};

} // namespace digipets
