#pragma once

#include <string>
#include <chrono>
#include <optional>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace digipets {

enum class PetStage {
    EGG,
    BABY,
    CHILD,
    ADULT,
    ULTIMATE
};

enum class PetSpecies {
    AGUMON,
    GABUMON,
    PATAMON,
    TAILMON
};

struct PetStats {
    int health = 100;
    int hunger = 0;      // 0 = full, 100 = starving
    int happiness = 100;
    int strength = 10;
    int intelligence = 10;
    int age_hours = 0;   // Age in hours
    int level = 1;
    int experience = 0;

    // Validation
    void clamp() {
        health = std::clamp(health, 0, 100);
        hunger = std::clamp(hunger, 0, 100);
        happiness = std::clamp(happiness, 0, 100);
        strength = std::max(0, strength);
        intelligence = std::max(0, intelligence);
        level = std::max(1, level);
        experience = std::max(0, experience);
    }
};

class Pet {
public:
    Pet(const std::string& name, PetSpecies species);
    Pet() = default;

    // Getters
    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    PetSpecies get_species() const { return species_; }
    PetStage get_stage() const { return stage_; }
    const PetStats& get_stats() const { return stats_; }
    bool is_alive() const { return stats_.health > 0; }
    std::chrono::system_clock::time_point get_birth_time() const { return birth_time_; }
    std::chrono::system_clock::time_point get_last_update() const { return last_update_; }

    // Actions
    void feed();
    void train();
    void play();
    void rest();
    void update(const std::chrono::system_clock::time_point& current_time);

    // Evolution
    void check_evolution();

    // Serialization
    nlohmann::json to_json() const;
    static Pet from_json(const nlohmann::json& j);

private:
    std::string id_;
    std::string name_;
    PetSpecies species_;
    PetStage stage_ = PetStage::EGG;
    PetStats stats_;
    std::chrono::system_clock::time_point birth_time_;
    std::chrono::system_clock::time_point last_update_;

    void apply_time_effects(int hours_passed);
    void level_up();
    std::string generate_id();
};

// Helper functions
std::string pet_stage_to_string(PetStage stage);
std::string pet_species_to_string(PetSpecies species);
PetStage pet_stage_from_string(const std::string& str);
PetSpecies pet_species_from_string(const std::string& str);

} // namespace digipets
