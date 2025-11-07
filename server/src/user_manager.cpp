#include "user_manager.hpp"
#include <fstream>

namespace digipets {

std::optional<std::string> UserManager::register_user(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if username already exists
    if (username_to_id_.find(username) != username_to_id_.end()) {
        return std::nullopt;
    }
    
    User user(username, password);
    std::string user_id = user.get_id();
    
    users_by_id_[user_id] = user;
    username_to_id_[username] = user_id;
    
    return user_id;
}

std::optional<std::string> UserManager::authenticate(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = username_to_id_.find(username);
    if (it == username_to_id_.end()) {
        return std::nullopt;
    }
    
    const std::string& user_id = it->second;
    auto user_it = users_by_id_.find(user_id);
    
    if (user_it != users_by_id_.end() && user_it->second.verify_password(password)) {
        return user_id;
    }
    
    return std::nullopt;
}

std::optional<User> UserManager::get_user(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_by_id_.find(user_id);
    if (it != users_by_id_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<User> UserManager::get_user_by_username(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = username_to_id_.find(username);
    if (it != username_to_id_.end()) {
        auto user_it = users_by_id_.find(it->second);
        if (user_it != users_by_id_.end()) {
            return user_it->second;
        }
    }
    return std::nullopt;
}

bool UserManager::user_exists(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    return username_to_id_.find(username) != username_to_id_.end();
}

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
        
        for (const auto& user_json : j) {
            User user = User::from_json(user_json);
            std::string user_id = user.get_id();
            users_by_id_[user_id] = user;
            username_to_id_[user.get_username()] = user_id;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace digipets
