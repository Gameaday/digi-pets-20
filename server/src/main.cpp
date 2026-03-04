#include "pet_manager.hpp"
#include "user_manager.hpp"
#include "user.hpp"          // is_valid_username / is_valid_password
#include <httplib.h>
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

using namespace digipets;

// Global flag for graceful shutdown
static std::atomic<bool> running{true};

static void signal_handler(int /*signal*/) {
    std::cout << "\nShutting down...\n";
    running = false;
}

// ---- auth middleware helper -----------------------------------------------
// Returns user_id if the Bearer session token is valid, nullopt otherwise.
static std::optional<std::string> require_auth(const httplib::Request& req,
                                                httplib::Response&      res,
                                                UserManager&            user_mgr) {
    const std::string prefix = "Bearer ";
    auto auth = req.get_header_value("Authorization");
    if (auth.size() <= prefix.size() || auth.substr(0, prefix.size()) != prefix) {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return std::nullopt;
    }
    std::string token = auth.substr(prefix.size());
    auto uid = user_mgr.validate_session(token);
    if (!uid) {
        res.status = 401;
        res.set_content(R"({"error":"Invalid or expired token"})", "application/json");
        return std::nullopt;
    }
    return uid;
}

// ---- JSON error helpers ---------------------------------------------------
static void bad_request(httplib::Response& res, const std::string& msg) {
    res.status = 400;
    res.set_content(nlohmann::json{{"error", msg}}.dump(), "application/json");
}

static void not_found(httplib::Response& res) {
    res.status = 404;
    res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
}

// ---- main -----------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    const std::string pets_file  = "pets.json";
    const std::string users_file = "users.json";
    const int port = (argc > 1) ? std::stoi(argv[1]) : 8080;

    PetManager  pet_mgr;
    UserManager user_mgr;

    if (pet_mgr.load_from_file(pets_file))
        std::cout << "Loaded pets from "  << pets_file  << '\n';
    if (user_mgr.load_from_file(users_file))
        std::cout << "Loaded users from " << users_file << '\n';

    httplib::Server svr;

    // ---- CORS --------------------------------------------------------------
    svr.set_default_headers({
        {"Access-Control-Allow-Origin",  "*"},
        {"Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"},
    });
    // Preflight
    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // ---- health ------------------------------------------------------------
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok","version":"1.1.0"})", "application/json");
    });

    // ---- POST /api/auth/register ------------------------------------------
    svr.Post("/api/auth/register",
        [&user_mgr, &users_file](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j        = nlohmann::json::parse(req.body);
            auto username = j.at("username").get<std::string>();
            auto password = j.at("password").get<std::string>();

            if (!is_valid_username(username))
                return bad_request(res,
                    "Username must be 3-32 characters (letters, digits, underscores)");
            if (!is_valid_password(password))
                return bad_request(res, "Password must be 8-128 characters");

            auto uid = user_mgr.register_user(username, password);
            if (!uid) {
                res.status = 409;
                res.set_content(R"({"error":"Username already taken"})", "application/json");
                return;
            }
            user_mgr.save_to_file(users_file);
            res.status = 201;
            res.set_content(
                nlohmann::json{{"user_id", *uid}, {"username", username}}.dump(),
                "application/json");
        } catch (const std::exception& e) {
            bad_request(res, std::string("Invalid request: ") + e.what());
        }
    });

    // ---- POST /api/auth/login --------------------------------------------
    svr.Post("/api/auth/login",
        [&user_mgr](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j        = nlohmann::json::parse(req.body);
            auto username = j.at("username").get<std::string>();
            auto password = j.at("password").get<std::string>();

            auto uid = user_mgr.authenticate(username, password);
            if (!uid) {
                res.status = 401;
                res.set_content(R"({"error":"Invalid credentials"})", "application/json");
                return;
            }
            // Issue a real session token (random 64-char hex, 24h TTL)
            std::string token = user_mgr.create_session(*uid);
            res.set_content(
                nlohmann::json{
                    {"user_id", *uid},
                    {"username", username},
                    {"token",   token},
                    {"expires_in_seconds", 86400}
                }.dump(),
                "application/json");
        } catch (const std::exception& e) {
            bad_request(res, std::string("Invalid request: ") + e.what());
        }
    });

    // ---- POST /api/auth/logout -------------------------------------------
    svr.Post("/api/auth/logout",
        [&user_mgr](const httplib::Request& req, httplib::Response& res) {
        const std::string prefix = "Bearer ";
        auto auth = req.get_header_value("Authorization");
        if (auth.size() > prefix.size() && auth.substr(0, prefix.size()) == prefix) {
            user_mgr.revoke_session(auth.substr(prefix.size()));
        }
        res.set_content(R"({"message":"Logged out"})", "application/json");
    });

    // ---- GET /api/pets ---------------------------------------------------
    svr.Get("/api/pets",
        [&pet_mgr, &user_mgr](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_auth(req, res, user_mgr);
        if (!uid) return;

        auto pets = pet_mgr.get_all_pets(*uid);
        nlohmann::json j = nlohmann::json::array();
        for (const auto& p : pets) j.push_back(p.to_json());
        res.set_content(j.dump(), "application/json");
    });

    // ---- GET /api/pets/:id -----------------------------------------------
    svr.Get(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))",
        [&pet_mgr, &user_mgr](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_auth(req, res, user_mgr);
        if (!uid) return;

        auto pet = pet_mgr.get_pet(req.matches[1], *uid);
        if (!pet) return not_found(res);
        res.set_content(pet->to_json().dump(), "application/json");
    });

    // ---- POST /api/pets --------------------------------------------------
    svr.Post("/api/pets",
        [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_auth(req, res, user_mgr);
        if (!uid) return;

        try {
            auto j    = nlohmann::json::parse(req.body);
            auto name = j.at("name").get<std::string>();
            auto sstr = j.at("species").get<std::string>();

            if (name.empty() || name.size() > 64)
                return bad_request(res, "Name must be 1-64 characters");

            auto species = pet_species_from_string(sstr);
            auto id      = pet_mgr.create_pet(name, species, *uid);
            auto pet     = pet_mgr.get_pet(id, *uid);
            pet_mgr.save_to_file(pets_file);
            res.status = 201;
            res.set_content(pet->to_json().dump(), "application/json");
        } catch (const std::exception& e) {
            bad_request(res, std::string("Invalid request: ") + e.what());
        }
    });

    // ---- DELETE /api/pets/:id -------------------------------------------
    svr.Delete(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))",
        [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto uid = require_auth(req, res, user_mgr);
        if (!uid) return;

        if (!pet_mgr.delete_pet(req.matches[1], *uid))
            return not_found(res);
        pet_mgr.save_to_file(pets_file);
        res.set_content(R"({"success":true})", "application/json");
    });

    // ---- Pet action helper -----------------------------------------------
    auto pet_action = [&](const httplib::Request& req, httplib::Response& res,
                          auto action_fn) {
        auto uid = require_auth(req, res, user_mgr);
        if (!uid) return;

        std::string id = req.matches[1];
        if (!action_fn(id, *uid)) return not_found(res);

        auto pet = pet_mgr.get_pet(id, *uid);
        pet_mgr.save_to_file(pets_file);
        res.set_content(pet->to_json().dump(), "application/json");
    };

    // ---- Pet actions ------------------------------------------------------
    const std::string uuid_rx = R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))";
    svr.Post(uuid_rx + "/feed",
        [&](const httplib::Request& r, httplib::Response& rs) {
            pet_action(r, rs, [&](const std::string& id, const std::string& uid) {
                return pet_mgr.feed_pet(id, uid); }); });

    svr.Post(uuid_rx + "/train",
        [&](const httplib::Request& r, httplib::Response& rs) {
            pet_action(r, rs, [&](const std::string& id, const std::string& uid) {
                return pet_mgr.train_pet(id, uid); }); });

    svr.Post(uuid_rx + "/play",
        [&](const httplib::Request& r, httplib::Response& rs) {
            pet_action(r, rs, [&](const std::string& id, const std::string& uid) {
                return pet_mgr.play_with_pet(id, uid); }); });

    svr.Post(uuid_rx + "/rest",
        [&](const httplib::Request& r, httplib::Response& rs) {
            pet_action(r, rs, [&](const std::string& id, const std::string& uid) {
                return pet_mgr.rest_pet(id, uid); }); });

    // ---- Background maintenance thread -----------------------------------
    std::thread maintenance_thread([&]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::minutes(5));
            if (!running) break;
            pet_mgr.update_all_pets();
            pet_mgr.save_to_file(pets_file);
            user_mgr.cleanup_expired_sessions();
        }
    });

    std::cout << "DigiPets server v1.1.0 listening on :" << port << '\n';
    std::cout << "Auth endpoints: POST /api/auth/register|login|logout\n";
    std::cout << "Pet endpoints (auth required): GET|POST /api/pets, actions /feed|train|play|rest\n";

    std::thread server_thread([&svr, port]() {
        svr.listen("0.0.0.0", port);
    });

    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Stopping...\n";
    svr.stop();
    if (server_thread.joinable())     server_thread.join();
    if (maintenance_thread.joinable()) maintenance_thread.join();

    pet_mgr.save_to_file(pets_file);
    user_mgr.save_to_file(users_file);
    std::cout << "Data saved. Goodbye.\n";
    return 0;
}
