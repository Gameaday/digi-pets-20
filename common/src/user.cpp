#include "user.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <regex>
#include <openssl/evp.h>

namespace digipets {

namespace {
    // Generate a lowercase UUID v4
    std::string make_uuid() {
        thread_local std::mt19937_64 gen{std::random_device{}()};
        thread_local std::uniform_int_distribution<uint32_t> dis(0, 15);
        thread_local std::uniform_int_distribution<uint32_t> dis2(8, 11);

        std::ostringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; ++i) ss << dis(gen);
        ss << '-';
        for (int i = 0; i < 4; ++i) ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; ++i) ss << dis(gen);
        ss << '-';
        ss << dis2(gen);
        for (int i = 0; i < 3; ++i) ss << dis(gen);
        ss << '-';
        for (int i = 0; i < 12; ++i) ss << dis(gen);
        return ss.str();
    }

    // Compute SHA-256 and return hex string
    std::string sha256_hex(const std::string& data) {
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hash_len = 0;

        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
        EVP_DigestUpdate(ctx, data.data(), data.size());
        EVP_DigestFinal_ex(ctx, hash, &hash_len);
        EVP_MD_CTX_free(ctx);

        std::ostringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < hash_len; ++i) {
            ss << std::setw(2) << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
}  // namespace

// ---- static helpers -------------------------------------------------------

std::string User::generate_uuid() {
    return make_uuid();
}

std::string User::generate_salt() {
    // 16 cryptographically random bytes → 32-char hex string
    thread_local std::mt19937_64 gen{std::random_device{}()};
    thread_local std::uniform_int_distribution<uint8_t> dis(0, 255);

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        ss << std::setw(2) << static_cast<int>(dis(gen));
    }
    return ss.str();
}

// ---- constructor ----------------------------------------------------------

User::User(const std::string& username, const std::string& password)
    : id_(generate_uuid())
    , username_(username)
    , salt_(generate_salt())
    , password_hash_(hash_password(password))
    , created_at_(std::chrono::system_clock::now())
{
}

// ---- password helpers -----------------------------------------------------

std::string User::hash_password(const std::string& password) const {
    // SHA-256(salt || password)
    return sha256_hex(salt_ + password);
}

bool User::verify_password(const std::string& password) const {
    return hash_password(password) == password_hash_;
}

void User::update_password(const std::string& new_password) {
    salt_ = generate_salt();          // rotate salt on password change
    password_hash_ = hash_password(new_password);
}

// ---- serialization --------------------------------------------------------

nlohmann::json User::to_json() const {
    using namespace std::chrono;
    return {
        {"id",            id_},
        {"username",      username_},
        {"salt",          salt_},
        {"password_hash", password_hash_},
        {"created_at",    duration_cast<seconds>(created_at_.time_since_epoch()).count()}
    };
}

User User::from_json(const nlohmann::json& j) {
    using namespace std::chrono;
    User user;
    user.id_            = j.at("id").get<std::string>();
    user.username_      = j.at("username").get<std::string>();
    // Backward compat: old records may not have a salt field
    user.salt_          = j.value("salt", "");
    user.password_hash_ = j.at("password_hash").get<std::string>();
    user.created_at_    = system_clock::time_point(
                              seconds(j.at("created_at").get<int64_t>()));
    return user;
}

// ---- input validation helpers ---------------------------------------------

bool is_valid_username(const std::string& username) {
    if (username.size() < 3 || username.size() > 32) return false;
    static const std::regex pattern(R"([A-Za-z0-9_]+)");
    return std::regex_match(username, pattern);
}

bool is_valid_password(const std::string& password) {
    return password.size() >= 8 && password.size() <= 128;
}

} // namespace digipets
