#include "digipets_client.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>

namespace digipets {

DigiPetsClient::DigiPetsClient(const std::string& server_url)
    : server_url_(server_url)
{
}

std::optional<std::string> DigiPetsClient::http_get(const std::string& path) {
    try {
        httplib::Client client(server_url_);
        client.set_connection_timeout(5, 0);
        client.set_read_timeout(10, 0);
        
        auto res = client.Get(path.c_str());
        
        if (res && res->status == 200) {
            return res->body;
        } else if (res) {
            last_error_ = "HTTP " + std::to_string(res->status);
            return std::nullopt;
        } else {
            last_error_ = "Connection failed";
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        last_error_ = e.what();
        return std::nullopt;
    }
}

std::optional<std::string> DigiPetsClient::http_post(const std::string& path, const std::string& body) {
    try {
        httplib::Client client(server_url_);
        client.set_connection_timeout(5, 0);
        client.set_read_timeout(10, 0);
        
        auto res = client.Post(path.c_str(), body, "application/json");
        
        if (res && (res->status == 200 || res->status == 201)) {
            return res->body;
        } else if (res) {
            last_error_ = "HTTP " + std::to_string(res->status);
            return std::nullopt;
        } else {
            last_error_ = "Connection failed";
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        last_error_ = e.what();
        return std::nullopt;
    }
}

std::optional<std::string> DigiPetsClient::http_delete(const std::string& path) {
    try {
        httplib::Client client(server_url_);
        client.set_connection_timeout(5, 0);
        client.set_read_timeout(10, 0);
        
        auto res = client.Delete(path.c_str());
        
        if (res && res->status == 200) {
            return res->body;
        } else if (res) {
            last_error_ = "HTTP " + std::to_string(res->status);
            return std::nullopt;
        } else {
            last_error_ = "Connection failed";
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        last_error_ = e.what();
        return std::nullopt;
    }
}

bool DigiPetsClient::check_health() {
    auto response = http_get("/health");
    return response.has_value();
}

std::optional<Pet> DigiPetsClient::create_pet(const std::string& name, PetSpecies species) {
    nlohmann::json request = {
        {"name", name},
        {"species", pet_species_to_string(species)}
    };
    
    auto response = http_post("/api/pets", request.dump());
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            return Pet::from_json(j);
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    return std::nullopt;
}

std::optional<Pet> DigiPetsClient::get_pet(const std::string& id) {
    auto response = http_get("/api/pets/" + id);
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            return Pet::from_json(j);
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    return std::nullopt;
}

std::vector<Pet> DigiPetsClient::get_all_pets() {
    std::vector<Pet> pets;
    
    auto response = http_get("/api/pets");
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            for (const auto& pet_json : j) {
                pets.push_back(Pet::from_json(pet_json));
            }
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    
    return pets;
}

bool DigiPetsClient::delete_pet(const std::string& id) {
    auto response = http_delete("/api/pets/" + id);
    return response.has_value();
}

std::optional<Pet> DigiPetsClient::feed_pet(const std::string& id) {
    auto response = http_post("/api/pets/" + id + "/feed", "");
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            return Pet::from_json(j);
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    return std::nullopt;
}

std::optional<Pet> DigiPetsClient::train_pet(const std::string& id) {
    auto response = http_post("/api/pets/" + id + "/train", "");
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            return Pet::from_json(j);
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    return std::nullopt;
}

std::optional<Pet> DigiPetsClient::play_with_pet(const std::string& id) {
    auto response = http_post("/api/pets/" + id + "/play", "");
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            return Pet::from_json(j);
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    return std::nullopt;
}

std::optional<Pet> DigiPetsClient::rest_pet(const std::string& id) {
    auto response = http_post("/api/pets/" + id + "/rest", "");
    if (response) {
        try {
            auto j = nlohmann::json::parse(*response);
            return Pet::from_json(j);
        } catch (const std::exception& e) {
            last_error_ = std::string("Parse error: ") + e.what();
        }
    }
    return std::nullopt;
}

} // namespace digipets
