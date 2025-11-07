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
    std::string create_pet(const std::string& name, PetSpecies species, const std::string& owner_id);
    std::optional<Pet> get_pet(const std::string& id, const std::string& owner_id = "");
    std::vector<Pet> get_all_pets(const std::string& owner_id = "");
    bool update_pet(const Pet& pet);
    bool delete_pet(const std::string& id, const std::string& owner_id = "");

    // Pet actions
    bool feed_pet(const std::string& id, const std::string& owner_id = "");
    bool train_pet(const std::string& id, const std::string& owner_id = "");
    bool play_with_pet(const std::string& id, const std::string& owner_id = "");
    bool rest_pet(const std::string& id, const std::string& owner_id = "");

    // Update all pets based on time
    void update_all_pets();

    // Persistence
    bool save_to_file(const std::string& filename);
    bool load_from_file(const std::string& filename);

private:
    std::unordered_map<std::string, Pet> pets_;
    mutable std::mutex mutex_;
    
    // Helper to check ownership
    bool check_ownership(const std::string& pet_id, const std::string& owner_id);
};

} // namespace digipets
