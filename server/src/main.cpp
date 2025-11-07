#include "pet_manager.hpp"
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

int main(int argc, char* argv[]) {
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const std::string save_file = "pets.json";
    const int port = (argc > 1) ? std::stoi(argv[1]) : 8080;

    PetManager manager;
    
    // Try to load existing pets
    if (manager.load_from_file(save_file)) {
        std::cout << "Loaded existing pets from " << save_file << std::endl;
    }

    httplib::Server server;

    // CORS headers
    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    // Health check
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    // List all pets
    server.Get("/api/pets", [&manager](const httplib::Request&, httplib::Response& res) {
        auto pets = manager.get_all_pets();
        nlohmann::json j = nlohmann::json::array();
        
        for (const auto& pet : pets) {
            j.push_back(pet.to_json());
        }
        
        res.set_content(j.dump(), "application/json");
    });

    // Get specific pet
    server.Get(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))", [&manager](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto pet_opt = manager.get_pet(id);
        
        if (pet_opt) {
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found"})", "application/json");
        }
    });

    // Create new pet
    server.Post("/api/pets", [&manager, &save_file](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = nlohmann::json::parse(req.body);
            
            std::string name = j.at("name").get<std::string>();
            std::string species_str = j.at("species").get<std::string>();
            
            PetSpecies species = pet_species_from_string(species_str);
            std::string id = manager.create_pet(name, species);
            
            auto pet_opt = manager.get_pet(id);
            if (pet_opt) {
                manager.save_to_file(save_file);
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
    server.Delete(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+))", [&manager, &save_file](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        
        if (manager.delete_pet(id)) {
            manager.save_to_file(save_file);
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found"})", "application/json");
        }
    });

    // Pet actions
    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/feed)", [&manager, &save_file](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        
        if (manager.feed_pet(id)) {
            auto pet_opt = manager.get_pet(id);
            manager.save_to_file(save_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found"})", "application/json");
        }
    });

    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/train)", [&manager, &save_file](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        
        if (manager.train_pet(id)) {
            auto pet_opt = manager.get_pet(id);
            manager.save_to_file(save_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found"})", "application/json");
        }
    });

    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/play)", [&manager, &save_file](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        
        if (manager.play_with_pet(id)) {
            auto pet_opt = manager.get_pet(id);
            manager.save_to_file(save_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found"})", "application/json");
        }
    });

    server.Post(R"(/api/pets/(\w+-\w+-\w+-\w+-\w+)/rest)", [&manager, &save_file](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        
        if (manager.rest_pet(id)) {
            auto pet_opt = manager.get_pet(id);
            manager.save_to_file(save_file);
            res.set_content(pet_opt->to_json().dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error":"Pet not found"})", "application/json");
        }
    });

    // Background thread to periodically update all pets
    std::thread update_thread([&manager, &save_file]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::minutes(5));
            if (running) {
                manager.update_all_pets();
                manager.save_to_file(save_file);
            }
        }
    });

    std::cout << "Starting DigiPets server on port " << port << "..." << std::endl;
    std::cout << "API endpoints:" << std::endl;
    std::cout << "  GET    /health" << std::endl;
    std::cout << "  GET    /api/pets" << std::endl;
    std::cout << "  GET    /api/pets/{id}" << std::endl;
    std::cout << "  POST   /api/pets" << std::endl;
    std::cout << "  DELETE /api/pets/{id}" << std::endl;
    std::cout << "  POST   /api/pets/{id}/feed" << std::endl;
    std::cout << "  POST   /api/pets/{id}/train" << std::endl;
    std::cout << "  POST   /api/pets/{id}/play" << std::endl;
    std::cout << "  POST   /api/pets/{id}/rest" << std::endl;

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
    manager.save_to_file(save_file);
    std::cout << "Server stopped. Pets saved." << std::endl;

    return 0;
}
