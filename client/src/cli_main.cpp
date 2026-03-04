#include "digipets_client.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace digipets;

void print_pet(const Pet& pet) {
    std::cout << "\n=== " << pet.get_name() << " ===\n";
    std::cout << "ID: " << pet.get_id() << "\n";
    std::cout << "Species: " << pet_species_to_string(pet.get_species()) << "\n";
    std::cout << "Stage: " << pet_stage_to_string(pet.get_stage()) << "\n";
    std::cout << "Status: " << (pet.is_alive() ? "Alive" : "Dead") << "\n";
    
    const auto& stats = pet.get_stats();
    std::cout << "\nStats:\n";
    std::cout << "  Level: " << stats.level << " (XP: " << stats.experience << "/" << stats.level * 100 << ")\n";
    std::cout << "  Health: " << stats.health << "/100\n";
    std::cout << "  Hunger: " << stats.hunger << "/100\n";
    std::cout << "  Happiness: " << stats.happiness << "/100\n";
    std::cout << "  Strength: " << stats.strength << "\n";
    std::cout << "  Intelligence: " << stats.intelligence << "\n";
    std::cout << "  Age: " << stats.age_hours << " hours\n";
    std::cout << std::endl;
}

void print_menu() {
    std::cout << "\n=== DigiPets CLI ===\n";
    std::cout << "1. List all pets\n";
    std::cout << "2. Create new pet\n";
    std::cout << "3. View pet details\n";
    std::cout << "4. Feed pet\n";
    std::cout << "5. Train pet\n";
    std::cout << "6. Play with pet\n";
    std::cout << "7. Rest pet\n";
    std::cout << "8. Delete pet\n";
    std::cout << "9. Check server health\n";
    std::cout << "0. Exit\n";
    std::cout << "Choose an option: ";
}

int main(int argc, char* argv[]) {
    std::string server_url = (argc > 1) ? argv[1] : "http://localhost:8080";
    
    DigiPetsClient client(server_url);
    
    std::cout << "DigiPets CLI Client\n";
    std::cout << "Connected to: " << server_url << "\n";
    
    if (!client.check_health()) {
        std::cout << "ERROR: Cannot connect to server!\n";
        std::cout << "Error: " << client.get_last_error() << "\n";
        std::cout << "Please make sure the server is running.\n";
        return 1;
    }
    
    std::cout << "Server connection OK!\n";
    
    while (true) {
        print_menu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        if (choice == 0) {
            std::cout << "Goodbye!\n";
            break;
        }
        
        switch (choice) {
            case 1: { // List all pets
                auto pets = client.get_all_pets();
                std::cout << "\nFound " << pets.size() << " pet(s):\n";
                for (const auto& pet : pets) {
                    print_pet(pet);
                }
                break;
            }
            
            case 2: { // Create new pet
                std::cout << "Enter pet name: ";
                std::string name;
                std::getline(std::cin, name);
                
                std::cout << "Choose species (agumon/gabumon/patamon/tailmon): ";
                std::string species_str;
                std::getline(std::cin, species_str);
                
                PetSpecies species = pet_species_from_string(species_str);
                auto pet_opt = client.create_pet(name, species);
                
                if (pet_opt) {
                    std::cout << "Pet created successfully!\n";
                    print_pet(*pet_opt);
                } else {
                    std::cout << "Failed to create pet: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            case 3: { // View pet details
                std::cout << "Enter pet ID: ";
                std::string id;
                std::getline(std::cin, id);
                
                auto pet_opt = client.get_pet(id);
                if (pet_opt) {
                    print_pet(*pet_opt);
                } else {
                    std::cout << "Pet not found: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            case 4: { // Feed pet
                std::cout << "Enter pet ID: ";
                std::string id;
                std::getline(std::cin, id);
                
                auto pet_opt = client.feed_pet(id);
                if (pet_opt) {
                    std::cout << "Fed pet successfully!\n";
                    print_pet(*pet_opt);
                } else {
                    std::cout << "Failed to feed pet: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            case 5: { // Train pet
                std::cout << "Enter pet ID: ";
                std::string id;
                std::getline(std::cin, id);
                
                auto pet_opt = client.train_pet(id);
                if (pet_opt) {
                    std::cout << "Trained pet successfully!\n";
                    print_pet(*pet_opt);
                } else {
                    std::cout << "Failed to train pet: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            case 6: { // Play with pet
                std::cout << "Enter pet ID: ";
                std::string id;
                std::getline(std::cin, id);
                
                auto pet_opt = client.play_with_pet(id);
                if (pet_opt) {
                    std::cout << "Played with pet successfully!\n";
                    print_pet(*pet_opt);
                } else {
                    std::cout << "Failed to play with pet: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            case 7: { // Rest pet
                std::cout << "Enter pet ID: ";
                std::string id;
                std::getline(std::cin, id);
                
                auto pet_opt = client.rest_pet(id);
                if (pet_opt) {
                    std::cout << "Pet is resting!\n";
                    print_pet(*pet_opt);
                } else {
                    std::cout << "Failed to rest pet: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            case 8: { // Delete pet
                std::cout << "Enter pet ID: ";
                std::string id;
                std::getline(std::cin, id);
                
                std::cout << "Are you sure? (yes/no): ";
                std::string confirm;
                std::getline(std::cin, confirm);
                
                if (confirm == "yes") {
                    if (client.delete_pet(id)) {
                        std::cout << "Pet deleted.\n";
                    } else {
                        std::cout << "Failed to delete pet: " << client.get_last_error() << "\n";
                    }
                }
                break;
            }
            
            case 9: { // Check server health
                if (client.check_health()) {
                    std::cout << "Server is healthy!\n";
                } else {
                    std::cout << "Server health check failed: " << client.get_last_error() << "\n";
                }
                break;
            }
            
            default:
                std::cout << "Invalid option!\n";
                break;
        }
    }
    
    return 0;
}
