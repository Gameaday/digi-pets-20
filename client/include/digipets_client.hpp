#pragma once

#include "pet.hpp"
#include <string>
#include <vector>
#include <optional>
#include <functional>

namespace digipets {

class DigiPetsClient {
public:
    explicit DigiPetsClient(const std::string& server_url);
    ~DigiPetsClient() = default;

    // Server connection
    bool check_health();

    // Pet management
    std::optional<Pet> create_pet(const std::string& name, PetSpecies species);
    std::optional<Pet> get_pet(const std::string& id);
    std::vector<Pet> get_all_pets();
    bool delete_pet(const std::string& id);

    // Pet actions
    std::optional<Pet> feed_pet(const std::string& id);
    std::optional<Pet> train_pet(const std::string& id);
    std::optional<Pet> play_with_pet(const std::string& id);
    std::optional<Pet> rest_pet(const std::string& id);

    // Error handling
    std::string get_last_error() const { return last_error_; }

private:
    std::string server_url_;
    std::string last_error_;

    std::optional<std::string> http_get(const std::string& path);
    std::optional<std::string> http_post(const std::string& path, const std::string& body);
    std::optional<std::string> http_delete(const std::string& path);
};

} // namespace digipets
