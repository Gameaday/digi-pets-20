#include <gtest/gtest.h>
#include "pet.hpp"
#include <chrono>
#include <thread>

using namespace digipets;

TEST(PetTest, Creation) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_EQ(pet.get_name(), "Agumon");
    EXPECT_EQ(pet.get_species(), PetSpecies::AGUMON);
    EXPECT_EQ(pet.get_stage(), PetStage::EGG);
    EXPECT_TRUE(pet.is_alive());
    EXPECT_FALSE(pet.get_id().empty());
}

TEST(PetTest, Feed) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    // Manually increase hunger
    auto stats = pet.get_stats();
    int initial_hunger = stats.hunger;
    
    pet.feed();
    
    stats = pet.get_stats();
    EXPECT_LE(stats.hunger, initial_hunger);
    EXPECT_GE(stats.happiness, 0);
}

TEST(PetTest, Train) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    auto initial_stats = pet.get_stats();
    
    pet.train();
    
    auto new_stats = pet.get_stats();
    EXPECT_GT(new_stats.strength, initial_stats.strength);
    EXPECT_GT(new_stats.experience, initial_stats.experience);
}

TEST(PetTest, Play) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    auto initial_stats = pet.get_stats();
    
    pet.play();
    
    auto new_stats = pet.get_stats();
    EXPECT_GE(new_stats.happiness, initial_stats.happiness);
    EXPECT_GT(new_stats.experience, initial_stats.experience);
}

TEST(PetTest, Rest) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    pet.rest();
    
    auto stats = pet.get_stats();
    EXPECT_GT(stats.health, 0);
}

TEST(PetTest, Evolution) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_EQ(pet.get_stage(), PetStage::EGG);
    
    // Simulate 2 hours passing to evolve from egg
    auto future_time = std::chrono::system_clock::now() + std::chrono::hours(2);
    pet.update(future_time);
    
    EXPECT_EQ(pet.get_stage(), PetStage::BABY);
}

TEST(PetTest, Serialization) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    pet.feed();
    pet.train();
    
    // Serialize
    auto json = pet.to_json();
    
    // Deserialize
    Pet restored = Pet::from_json(json);
    
    EXPECT_EQ(restored.get_id(), pet.get_id());
    EXPECT_EQ(restored.get_name(), pet.get_name());
    EXPECT_EQ(restored.get_species(), pet.get_species());
    EXPECT_EQ(restored.get_stage(), pet.get_stage());
    EXPECT_EQ(restored.get_stats().health, pet.get_stats().health);
    EXPECT_EQ(restored.get_stats().hunger, pet.get_stats().hunger);
}

TEST(PetTest, TimeEffects) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    auto initial_stats = pet.get_stats();
    
    // Simulate 5 hours passing
    auto future_time = std::chrono::system_clock::now() + std::chrono::hours(5);
    pet.update(future_time);
    
    auto new_stats = pet.get_stats();
    
    // Hunger should increase
    EXPECT_GT(new_stats.hunger, initial_stats.hunger);
    
    // Age should increase
    EXPECT_GT(new_stats.age_hours, initial_stats.age_hours);
}

TEST(PetTest, StatsValidation) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    // Feed multiple times
    for (int i = 0; i < 10; i++) {
        pet.feed();
    }
    
    auto stats = pet.get_stats();
    
    // Stats should be clamped to valid ranges
    EXPECT_LE(stats.health, 100);
    EXPECT_GE(stats.health, 0);
    EXPECT_LE(stats.hunger, 100);
    EXPECT_GE(stats.hunger, 0);
    EXPECT_LE(stats.happiness, 100);
    EXPECT_GE(stats.happiness, 0);
}

TEST(PetTest, LevelUp) {
    Pet pet("Agumon", PetSpecies::AGUMON);
    
    auto initial_level = pet.get_stats().level;
    
    // Train many times to gain experience
    for (int i = 0; i < 15; i++) {
        pet.train();
    }
    
    auto new_level = pet.get_stats().level;
    
    EXPECT_GT(new_level, initial_level);
}
