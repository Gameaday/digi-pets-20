#pragma once

#include "pet.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace digipets {

class PetManager {
public:
    PetManager() = default;

    // CRUD operations
    std::string create_pet(const std::string& name, PetSpecies species);
    std::optional<Pet> get_pet(const std::string& id);
    std::vector<Pet> get_all_pets();
    bool update_pet(const Pet& pet);
    bool delete_pet(const std::string& id);

    // Pet actions
    bool feed_pet(const std::string& id);
    bool train_pet(const std::string& id);
    bool play_with_pet(const std::string& id);
    bool rest_pet(const std::string& id);

    // Update all pets based on time
    void update_all_pets();

    // Persistence
    bool save_to_file(const std::string& filename);
    bool load_from_file(const std::string& filename);

private:
    std::unordered_map<std::string, Pet> pets_;
    mutable std::mutex mutex_;
};

} // namespace digipets
