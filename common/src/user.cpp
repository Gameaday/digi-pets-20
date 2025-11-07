#include "user.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <functional>

namespace digipets {

namespace {
    std::string generate_uuid() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 4; i++) ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 12; i++) ss << dis(gen);
        return ss.str();
    }
}

User::User(const std::string& username, const std::string& password)
    : id_(generate_uuid())
    , username_(username)
    , password_hash_(hash_password(password))
    , created_at_(std::chrono::system_clock::now())
{
}

std::string User::generate_id() {
    return generate_uuid();
}

std::string User::hash_password(const std::string& password) const {
    // Simple hash for demonstration - in production use bcrypt or argon2
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + "salt_" + username_);
    
    std::stringstream ss;
    ss << std::hex << hash_value;
    return ss.str();
}

bool User::verify_password(const std::string& password) const {
    return hash_password(password) == password_hash_;
}

void User::update_password(const std::string& new_password) {
    password_hash_ = hash_password(new_password);
}

nlohmann::json User::to_json() const {
    using namespace std::chrono;
    
    return {
        {"id", id_},
        {"username", username_},
        {"password_hash", password_hash_},
        {"created_at", duration_cast<seconds>(created_at_.time_since_epoch()).count()}
    };
}

User User::from_json(const nlohmann::json& j) {
    using namespace std::chrono;
    
    User user;
    user.id_ = j.at("id").get<std::string>();
    user.username_ = j.at("username").get<std::string>();
    user.password_hash_ = j.at("password_hash").get<std::string>();
    user.created_at_ = system_clock::time_point(
        seconds(j.at("created_at").get<int64_t>())
    );
    
    return user;
}

} // namespace digipets
