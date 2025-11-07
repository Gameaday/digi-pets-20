#include <gtest/gtest.h>
#include "pet_manager.hpp"

using namespace digipets;

TEST(PetManagerTest, CreatePet) {
    PetManager manager;
    
    std::string id = manager.create_pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_FALSE(id.empty());
    
    auto pet_opt = manager.get_pet(id);
    ASSERT_TRUE(pet_opt.has_value());
    EXPECT_EQ(pet_opt->get_name(), "Agumon");
}

TEST(PetManagerTest, GetAllPets) {
    PetManager manager;
    
    manager.create_pet("Agumon", PetSpecies::AGUMON);
    manager.create_pet("Gabumon", PetSpecies::GABUMON);
    
    auto pets = manager.get_all_pets();
    
    EXPECT_EQ(pets.size(), 2);
}

TEST(PetManagerTest, DeletePet) {
    PetManager manager;
    
    std::string id = manager.create_pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_TRUE(manager.delete_pet(id));
    
    auto pet_opt = manager.get_pet(id);
    EXPECT_FALSE(pet_opt.has_value());
}

TEST(PetManagerTest, FeedPet) {
    PetManager manager;
    
    std::string id = manager.create_pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_TRUE(manager.feed_pet(id));
    
    auto pet_opt = manager.get_pet(id);
    ASSERT_TRUE(pet_opt.has_value());
}

TEST(PetManagerTest, TrainPet) {
    PetManager manager;
    
    std::string id = manager.create_pet("Agumon", PetSpecies::AGUMON);
    
    auto before = manager.get_pet(id);
    ASSERT_TRUE(before.has_value());
    auto initial_strength = before->get_stats().strength;
    
    EXPECT_TRUE(manager.train_pet(id));
    
    auto after = manager.get_pet(id);
    ASSERT_TRUE(after.has_value());
    EXPECT_GT(after->get_stats().strength, initial_strength);
}

TEST(PetManagerTest, PlayWithPet) {
    PetManager manager;
    
    std::string id = manager.create_pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_TRUE(manager.play_with_pet(id));
}

TEST(PetManagerTest, RestPet) {
    PetManager manager;
    
    std::string id = manager.create_pet("Agumon", PetSpecies::AGUMON);
    
    EXPECT_TRUE(manager.rest_pet(id));
}

TEST(PetManagerTest, UpdateAllPets) {
    PetManager manager;
    
    manager.create_pet("Agumon", PetSpecies::AGUMON);
    manager.create_pet("Gabumon", PetSpecies::GABUMON);
    
    // Should not crash
    manager.update_all_pets();
    
    auto pets = manager.get_all_pets();
    EXPECT_EQ(pets.size(), 2);
}

TEST(PetManagerTest, Persistence) {
    const std::string test_file = "test_pets.json";
    
    {
        PetManager manager;
        manager.create_pet("Agumon", PetSpecies::AGUMON);
        manager.create_pet("Gabumon", PetSpecies::GABUMON);
        
        EXPECT_TRUE(manager.save_to_file(test_file));
    }
    
    {
        PetManager manager;
        EXPECT_TRUE(manager.load_from_file(test_file));
        
        auto pets = manager.get_all_pets();
        EXPECT_EQ(pets.size(), 2);
    }
    
    // Cleanup
    std::remove(test_file.c_str());
}

TEST(PetManagerTest, NonExistentPet) {
    PetManager manager;
    
    auto pet_opt = manager.get_pet("non-existent-id");
    EXPECT_FALSE(pet_opt.has_value());
    
    EXPECT_FALSE(manager.feed_pet("non-existent-id"));
    EXPECT_FALSE(manager.delete_pet("non-existent-id"));
}
