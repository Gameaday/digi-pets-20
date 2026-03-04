#pragma once

#include "user.hpp"
#include <unordered_map>
#include <mutex>
#include <string>
#include <optional>
#include <chrono>

namespace digipets {

static constexpr auto SESSION_TTL = std::chrono::hours(24);

struct Session {
    std::string user_id;
    std::chrono::system_clock::time_point expires_at;

    bool is_expired() const {
        return std::chrono::system_clock::now() >= expires_at;
    }
};

class UserManager {
public:
    UserManager() = default;

    // User management
    std::optional<std::string> register_user(const std::string& username,
                                             const std::string& password);
    std::optional<User> get_user(const std::string& user_id);
    std::optional<User> get_user_by_username(const std::string& username);
    bool user_exists(const std::string& username);

    // Authentication: returns user_id on success
    std::optional<std::string> authenticate(const std::string& username,
                                            const std::string& password);

    // Session management
    std::string  create_session(const std::string& user_id);
    std::optional<std::string> validate_session(const std::string& token);
    void revoke_session(const std::string& token);
    void cleanup_expired_sessions();

    // Persistence (users only; sessions are in-memory)
    bool save_to_file(const std::string& filename);
    bool load_from_file(const std::string& filename);

private:
    std::unordered_map<std::string, User>        users_by_id_;
    std::unordered_map<std::string, std::string> username_to_id_;
    std::unordered_map<std::string, Session>     sessions_;  // token → Session
    mutable std::mutex mutex_;

    static std::string generate_token();
};

} // namespace digipets
