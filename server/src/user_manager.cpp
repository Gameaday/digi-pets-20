#include "user_manager.hpp"
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>

namespace digipets {

// ---- static helper --------------------------------------------------------

std::string UserManager::generate_token() {
    // 32 cryptographically random bytes → 64-char hex string
    // thread_local avoids both static-init races and lock contention.
    thread_local std::mt19937_64 gen{std::random_device{}()};
    thread_local std::uniform_int_distribution<uint8_t> dis(0, 255);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 32; ++i) {
        ss << std::setw(2) << static_cast<int>(dis(gen));
    }
    return ss.str();
}

// ---- user management ------------------------------------------------------

std::optional<std::string> UserManager::register_user(const std::string& username,
                                                       const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (username_to_id_.count(username)) return std::nullopt;

    User user(username, password);
    std::string user_id = user.get_id();
    users_by_id_[user_id]     = std::move(user);
    username_to_id_[username] = user_id;
    return user_id;
}

std::optional<std::string> UserManager::authenticate(const std::string& username,
                                                      const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = username_to_id_.find(username);
    if (it == username_to_id_.end()) return std::nullopt;

    auto user_it = users_by_id_.find(it->second);
    if (user_it != users_by_id_.end() && user_it->second.verify_password(password)) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<User> UserManager::get_user(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = users_by_id_.find(user_id);
    if (it != users_by_id_.end()) return it->second;
    return std::nullopt;
}

std::optional<User> UserManager::get_user_by_username(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = username_to_id_.find(username);
    if (it == username_to_id_.end()) return std::nullopt;
    auto user_it = users_by_id_.find(it->second);
    if (user_it != users_by_id_.end()) return user_it->second;
    return std::nullopt;
}

bool UserManager::user_exists(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    return username_to_id_.count(username) > 0;
}

// ---- session management ---------------------------------------------------

std::string UserManager::create_session(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string token = generate_token();
    sessions_[token]  = {user_id, std::chrono::system_clock::now() + SESSION_TTL};
    return token;
}

std::optional<std::string> UserManager::validate_session(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(token);
    if (it == sessions_.end() || it->second.is_expired()) {
        if (it != sessions_.end()) sessions_.erase(it);  // lazy eviction
        return std::nullopt;
    }
    return it->second.user_id;
}

void UserManager::revoke_session(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(token);
}

void UserManager::cleanup_expired_sessions() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = sessions_.begin(); it != sessions_.end(); ) {
        it = it->second.is_expired() ? sessions_.erase(it) : std::next(it);
    }
}

// ---- persistence (users only; sessions are ephemeral) --------------------

bool UserManager::save_to_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& [id, user] : users_by_id_) {
            j.push_back(user.to_json());
        }
        std::ofstream file(filename);
        if (!file) return false;
        file << j.dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

bool UserManager::load_from_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        std::ifstream file(filename);
        if (!file) return false;
        nlohmann::json j;
        file >> j;
        users_by_id_.clear();
        username_to_id_.clear();
        for (const auto& uj : j) {
            User user = User::from_json(uj);
            std::string uid = user.get_id();
            username_to_id_[user.get_username()] = uid;
            users_by_id_[uid] = std::move(user);
        }
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace digipets
