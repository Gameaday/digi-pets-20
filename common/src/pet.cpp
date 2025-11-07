#include "pet.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace digipets {

namespace {
    std::string generate_uuid() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 4; i++) ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 12; i++) ss << dis(gen);
        return ss.str();
    }
}

Pet::Pet(const std::string& name, PetSpecies species)
    : id_(generate_uuid())
    , name_(name)
    , species_(species)
    , stage_(PetStage::EGG)
    , birth_time_(std::chrono::system_clock::now())
    , last_update_(std::chrono::system_clock::now())
{
    stats_.clamp();
}

std::string Pet::generate_id() {
    return generate_uuid();
}

void Pet::feed() {
    if (!is_alive()) return;

    stats_.hunger = std::max(0, stats_.hunger - 30);
    stats_.happiness = std::min(100, stats_.happiness + 5);
    stats_.health = std::min(100, stats_.health + 5);
    stats_.experience += 2;
    
    if (stats_.experience >= stats_.level * 100) {
        level_up();
    }
    
    stats_.clamp();
}

void Pet::train() {
    if (!is_alive()) return;

    stats_.strength += 3;
    stats_.intelligence += 1;
    stats_.hunger = std::min(100, stats_.hunger + 15);
    stats_.happiness = std::max(0, stats_.happiness - 5);
    stats_.experience += 10;
    
    if (stats_.experience >= stats_.level * 100) {
        level_up();
    }
    
    stats_.clamp();
}

void Pet::play() {
    if (!is_alive()) return;

    stats_.happiness = std::min(100, stats_.happiness + 20);
    stats_.hunger = std::min(100, stats_.hunger + 10);
    stats_.experience += 5;
    
    if (stats_.experience >= stats_.level * 100) {
        level_up();
    }
    
    stats_.clamp();
}

void Pet::rest() {
    if (!is_alive()) return;

    stats_.health = std::min(100, stats_.health + 20);
    stats_.happiness = std::min(100, stats_.happiness + 5);
    stats_.hunger = std::min(100, stats_.hunger + 5);
    
    stats_.clamp();
}

void Pet::level_up() {
    stats_.level++;
    stats_.experience = 0;
    stats_.health = 100;
    stats_.strength += 5;
    stats_.intelligence += 5;
    check_evolution();
}

void Pet::check_evolution() {
    PetStage new_stage = stage_;

    switch (stage_) {
        case PetStage::EGG:
            if (stats_.age_hours >= 1) {
                new_stage = PetStage::BABY;
            }
            break;
        case PetStage::BABY:
            if (stats_.level >= 5) {
                new_stage = PetStage::CHILD;
            }
            break;
        case PetStage::CHILD:
            if (stats_.level >= 15) {
                new_stage = PetStage::ADULT;
            }
            break;
        case PetStage::ADULT:
            if (stats_.level >= 30) {
                new_stage = PetStage::ULTIMATE;
            }
            break;
        case PetStage::ULTIMATE:
            // Already at max stage
            break;
    }

    if (new_stage != stage_) {
        stage_ = new_stage;
        stats_.health = 100;
        stats_.happiness = std::min(100, stats_.happiness + 20);
    }
}

void Pet::apply_time_effects(int hours_passed) {
    if (hours_passed <= 0) return;

    stats_.age_hours += hours_passed;
    
    // Hunger increases over time
    stats_.hunger = std::min(100, stats_.hunger + hours_passed * 5);
    
    // Happiness decreases if neglected
    stats_.happiness = std::max(0, stats_.happiness - hours_passed * 3);
    
    // Health decreases if hungry or unhappy
    if (stats_.hunger > 70 || stats_.happiness < 30) {
        stats_.health = std::max(0, stats_.health - hours_passed * 10);
    }
    
    stats_.clamp();
    check_evolution();
}

void Pet::update(const std::chrono::system_clock::time_point& current_time) {
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        current_time - last_update_
    );
    
    int hours_passed = static_cast<int>(duration.count());
    if (hours_passed > 0) {
        apply_time_effects(hours_passed);
        last_update_ = current_time;
    }
}

nlohmann::json Pet::to_json() const {
    using namespace std::chrono;
    
    return {
        {"id", id_},
        {"name", name_},
        {"species", pet_species_to_string(species_)},
        {"stage", pet_stage_to_string(stage_)},
        {"stats", {
            {"health", stats_.health},
            {"hunger", stats_.hunger},
            {"happiness", stats_.happiness},
            {"strength", stats_.strength},
            {"intelligence", stats_.intelligence},
            {"age_hours", stats_.age_hours},
            {"level", stats_.level},
            {"experience", stats_.experience}
        }},
        {"birth_time", duration_cast<seconds>(birth_time_.time_since_epoch()).count()},
        {"last_update", duration_cast<seconds>(last_update_.time_since_epoch()).count()},
        {"is_alive", is_alive()}
    };
}

Pet Pet::from_json(const nlohmann::json& j) {
    using namespace std::chrono;
    
    Pet pet;
    pet.id_ = j.at("id").get<std::string>();
    pet.name_ = j.at("name").get<std::string>();
    pet.species_ = pet_species_from_string(j.at("species").get<std::string>());
    pet.stage_ = pet_stage_from_string(j.at("stage").get<std::string>());
    
    const auto& stats = j.at("stats");
    pet.stats_.health = stats.at("health").get<int>();
    pet.stats_.hunger = stats.at("hunger").get<int>();
    pet.stats_.happiness = stats.at("happiness").get<int>();
    pet.stats_.strength = stats.at("strength").get<int>();
    pet.stats_.intelligence = stats.at("intelligence").get<int>();
    pet.stats_.age_hours = stats.at("age_hours").get<int>();
    pet.stats_.level = stats.at("level").get<int>();
    pet.stats_.experience = stats.at("experience").get<int>();
    
    pet.birth_time_ = system_clock::time_point(
        seconds(j.at("birth_time").get<int64_t>())
    );
    pet.last_update_ = system_clock::time_point(
        seconds(j.at("last_update").get<int64_t>())
    );
    
    return pet;
}

std::string pet_stage_to_string(PetStage stage) {
    switch (stage) {
        case PetStage::EGG: return "egg";
        case PetStage::BABY: return "baby";
        case PetStage::CHILD: return "child";
        case PetStage::ADULT: return "adult";
        case PetStage::ULTIMATE: return "ultimate";
    }
    return "unknown";
}

std::string pet_species_to_string(PetSpecies species) {
    switch (species) {
        case PetSpecies::AGUMON: return "agumon";
        case PetSpecies::GABUMON: return "gabumon";
        case PetSpecies::PATAMON: return "patamon";
        case PetSpecies::TAILMON: return "tailmon";
    }
    return "unknown";
}

PetStage pet_stage_from_string(const std::string& str) {
    if (str == "egg") return PetStage::EGG;
    if (str == "baby") return PetStage::BABY;
    if (str == "child") return PetStage::CHILD;
    if (str == "adult") return PetStage::ADULT;
    if (str == "ultimate") return PetStage::ULTIMATE;
    return PetStage::EGG;
}

PetSpecies pet_species_from_string(const std::string& str) {
    if (str == "agumon") return PetSpecies::AGUMON;
    if (str == "gabumon") return PetSpecies::GABUMON;
    if (str == "patamon") return PetSpecies::PATAMON;
    if (str == "tailmon") return PetSpecies::TAILMON;
    return PetSpecies::AGUMON;
}

} // namespace digipets
