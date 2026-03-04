#include "pet_manager.hpp"
#include <fstream>
#include <chrono>

namespace digipets {

bool PetManager::check_ownership(const std::string& pet_id, const std::string& owner_id) {
    // If owner_id is empty, allow access (backward compatibility or admin)
    if (owner_id.empty()) {
        return true;
    }
    
    auto it = pets_.find(pet_id);
    if (it == pets_.end()) {
        return false;
    }
    
    return it->second.get_owner_id() == owner_id;
}

std::string PetManager::create_pet(const std::string& name, PetSpecies species, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Pet pet(name, species, owner_id);
    std::string id = pet.get_id();
    pets_[id] = pet;
    
    return id;
}

std::optional<Pet> PetManager::get_pet(const std::string& id, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = pets_.find(id);
    if (it != pets_.end() && check_ownership(id, owner_id)) {
        // Update pet before returning
        auto now = std::chrono::system_clock::now();
        it->second.update(now);
        return it->second;
    }
    return std::nullopt;
}

std::vector<Pet> PetManager::get_all_pets(const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Pet> result;
    auto now = std::chrono::system_clock::now();
    
    for (auto& [id, pet] : pets_) {
        // If owner_id is provided, filter by ownership
        if (!owner_id.empty() && pet.get_owner_id() != owner_id) {
            continue;
        }
        
        pet.update(now);
        result.push_back(pet);
    }
    
    return result;
}

bool PetManager::update_pet(const Pet& pet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = pets_.find(pet.get_id());
    if (it != pets_.end()) {
        it->second = pet;
        return true;
    }
    return false;
}

bool PetManager::delete_pet(const std::string& id, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!check_ownership(id, owner_id)) {
        return false;
    }
    
    return pets_.erase(id) > 0;
}

bool PetManager::feed_pet(const std::string& id, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!check_ownership(id, owner_id)) {
        return false;
    }
    
    auto it = pets_.find(id);
    if (it != pets_.end()) {
        auto now = std::chrono::system_clock::now();
        it->second.update(now);
        it->second.feed();
        return true;
    }
    return false;
}

bool PetManager::train_pet(const std::string& id, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!check_ownership(id, owner_id)) {
        return false;
    }
    
    auto it = pets_.find(id);
    if (it != pets_.end()) {
        auto now = std::chrono::system_clock::now();
        it->second.update(now);
        it->second.train();
        return true;
    }
    return false;
}

bool PetManager::play_with_pet(const std::string& id, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!check_ownership(id, owner_id)) {
        return false;
    }
    
    auto it = pets_.find(id);
    if (it != pets_.end()) {
        auto now = std::chrono::system_clock::now();
        it->second.update(now);
        it->second.play();
        return true;
    }
    return false;
}

bool PetManager::rest_pet(const std::string& id, const std::string& owner_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!check_ownership(id, owner_id)) {
        return false;
    }
    
    auto it = pets_.find(id);
    if (it != pets_.end()) {
        auto now = std::chrono::system_clock::now();
        it->second.update(now);
        it->second.rest();
        return true;
    }
    return false;
}

void PetManager::update_all_pets() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    for (auto& [id, pet] : pets_) {
        pet.update(now);
    }
}

bool PetManager::save_to_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        nlohmann::json j = nlohmann::json::array();
        
        for (const auto& [id, pet] : pets_) {
            j.push_back(pet.to_json());
        }
        
        std::ofstream file(filename);
        if (!file) return false;
        
        file << j.dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

bool PetManager::load_from_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        std::ifstream file(filename);
        if (!file) return false;
        
        nlohmann::json j;
        file >> j;
        
        pets_.clear();
        
        for (const auto& pet_json : j) {
            Pet pet = Pet::from_json(pet_json);
            pets_[pet.get_id()] = pet;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace digipets
