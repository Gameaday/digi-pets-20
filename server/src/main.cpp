#include "pet_manager.hpp"
#include "user_manager.hpp"
#include <httplib.h>
#include <iostream>
#include <thread>
#include <csignal>

using namespace digipets;

// Global flag for graceful shutdown
std::atomic<bool> running{true};

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

// Helper function to extract user_id from Authorization header
std::optional<std::string> get_user_from_auth(const httplib::Request& req, UserManager& user_mgr) {
    auto auth_header = req.get_header_value("Authorization");
    
    if (auth_header.empty()) {
        return std::nullopt;
    }
    
    // Simple token-based auth: "Bearer <user_id>"
    if (auth_header.find("Bearer ") == 0) {
        std::string user_id = auth_header.substr(7);
        
        // Verify user exists
        auto user_opt = user_mgr.get_user(user_id);
        if (user_opt) {
            return user_id;
        }
    }
    
    return std::nullopt;
}

int main(int argc, char* argv[]) {
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const std::string pets_file = "pets.json";
    const std::string users_file = "users.json";
    const int port = (argc > 1) ? std::stoi(argv[1]) : 8080;

    PetManager pet_mgr;
    UserManager user_mgr;
    
    // Try to load existing data
    if (pet_mgr.load_from_file(pets_file)) {
        std::cout << "Loaded existing pets from " << pets_file << std::endl;
    }
    
    if (user_mgr.load_from_file(users_file)) {
        std::cout << "Loaded existing users from " << users_file << std::endl;
    }

    httplib::Server server;

    // CORS headers
    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    // Health check
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    // User registration
    server.Post("/api/auth/register", [&user_mgr, &users_file](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = nlohmann::json::parse(req.body);
            
            std::string username = j.at("username").get<std::string>();
            std::string password = j.at("password").get<std::string>();
            
            auto user_id_opt = user_mgr.register_user(username, password);
            
            if (user_id_opt) {
                user_mgr.save_to_file(users_file);
                
                res.status = 201;
                res.set_content(
                    nlohmann::json{
                        {"user_id", *user_id_opt},
                        {"username", username},
                        {"message", "User registered successfully"}
                    }.dump(),
                    "application/json"
                );
            } else {
                res.status = 409;
                res.set_content(R"({"error":"Username already exists"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", std::string("Invalid request: ") + e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // User login
    server.Post("/api/auth/login", [&user_mgr](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = nlohmann::json::parse(req.body);
            
            std::string username = j.at("username").get<std::string>();
            std::string password = j.at("password").get<std::string>();
            
            auto user_id_opt = user_mgr.authenticate(username, password);
            
            if (user_id_opt) {
                res.set_content(
                    nlohmann::json{
                        {"user_id", *user_id_opt},
                        {"username", username},
                        {"token", *user_id_opt},  // Simple token = user_id
                        {"message", "Login successful"}
                    }.dump(),
                    "application/json"
                );
            } else {
                res.status = 401;
                res.set_content(R"({"error":"Invalid credentials"})", "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", std::string("Invalid request: ") + e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // List all pets (user-filtered)
    server.Get("/api/pets", [&pet_mgr, &user_mgr](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        auto pets = pet_mgr.get_all_pets(*user_id_opt);
        nlohmann::json j = nlohmann::json::array();
        
        for (const auto& pet : pets) {
            j.push_back(pet.to_json());
        }
        
        res.set_content(j.dump(), "application/json");
    });

    // Get specific pet
    server.Get(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))", [&pet_mgr, &user_mgr](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        std::string id = req.matches[1];
        auto pet_opt = pet_mgr.get_pet(id, *user_id_opt);
        
        if (pet_opt) {
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
        }
    });

    // Create new pet
    server.Post("/api/pets", [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        try {
            auto j = nlohmann::json::parse(req.body);
            
            std::string name = j.at("name").get<std::string>();
            std::string species_str = j.at("species").get<std::string>();
            
            PetSpecies species = pet_species_from_string(species_str);
            std::string id = pet_mgr.create_pet(name, species, *user_id_opt);
            
            auto pet_opt = pet_mgr.get_pet(id, *user_id_opt);
            if (pet_opt) {
                pet_mgr.save_to_file(pets_file);
                res.status = 201;
                res.set_content(pet_opt->to_json().dump(), "application/json");
            }
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(
                nlohmann::json{{"error", std::string("Invalid request: ") + e.what()}}.dump(),
                "application/json"
            );
        }
    });

    // Delete pet
    server.Delete(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))", [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        std::string id = req.matches[1];
        
        if (pet_mgr.delete_pet(id, *user_id_opt)) {
            pet_mgr.save_to_file(pets_file);
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
        }
    });

    // Pet actions with authentication
    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/feed)", [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        std::string id = req.matches[1];
        
        if (pet_mgr.feed_pet(id, *user_id_opt)) {
            auto pet_opt = pet_mgr.get_pet(id, *user_id_opt);
            pet_mgr.save_to_file(pets_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
        }
    });

    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/train)", [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        std::string id = req.matches[1];
        
        if (pet_mgr.train_pet(id, *user_id_opt)) {
            auto pet_opt = pet_mgr.get_pet(id, *user_id_opt);
            pet_mgr.save_to_file(pets_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
        }
    });

    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/play)", [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        std::string id = req.matches[1];
        
        if (pet_mgr.play_with_pet(id, *user_id_opt)) {
            auto pet_opt = pet_mgr.get_pet(id, *user_id_opt);
            pet_mgr.save_to_file(pets_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
        }
    });

    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/rest)", [&pet_mgr, &user_mgr, &pets_file](const httplib::Request& req, httplib::Response& res) {
        auto user_id_opt = get_user_from_auth(req, user_mgr);
        
        if (!user_id_opt) {
            res.status = 401;
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            return;
        }
        
        std::string id = req.matches[1];
        
        if (pet_mgr.rest_pet(id, *user_id_opt)) {
            auto pet_opt = pet_mgr.get_pet(id, *user_id_opt);
            pet_mgr.save_to_file(pets_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found or access denied"})", "application/json");
        }
    });

    // Background thread to periodically update all pets
    std::thread update_thread([&pet_mgr, &pets_file]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::minutes(5));
            if (running) {
                pet_mgr.update_all_pets();
                pet_mgr.save_to_file(pets_file);
            }
        }
    });

    std::cout << "Starting DigiPets Multi-User server on port " << port << "..." << std::endl;
    std::cout << "API endpoints:" << std::endl;
    std::cout << "  GET    /health" << std::endl;
    std::cout << "  POST   /api/auth/register" << std::endl;
    std::cout << "  POST   /api/auth/login" << std::endl;
    std::cout << "  GET    /api/pets (requires auth)" << std::endl;
    std::cout << "  GET    /api/pets/{id} (requires auth)" << std::endl;
    std::cout << "  POST   /api/pets (requires auth)" << std::endl;
    std::cout << "  DELETE /api/pets/{id} (requires auth)" << std::endl;
    std::cout << "  POST   /api/pets/{id}/feed (requires auth)" << std::endl;
    std::cout << "  POST   /api/pets/{id}/train (requires auth)" << std::endl;
    std::cout << "  POST   /api/pets/{id}/play (requires auth)" << std::endl;
    std::cout << "  POST   /api/pets/{id}/rest (requires auth)" << std::endl;

    // Start server in a separate thread
    std::thread server_thread([&server, port]() {
        server.listen("0.0.0.0", port);
    });

    // Wait for shutdown signal
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Cleanup
    std::cout << "Stopping server..." << std::endl;
    server.stop();
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    if (update_thread.joinable()) {
        update_thread.join();
    }

    // Final save
    pet_mgr.save_to_file(pets_file);
    user_mgr.save_to_file(users_file);
    std::cout << "Server stopped. Data saved." << std::endl;

    return 0;
}
