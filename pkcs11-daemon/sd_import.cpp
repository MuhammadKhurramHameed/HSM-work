#include "sd_import.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

// Constructor definition
SDImporter::SDImporter(Vault& vault_ref) : vault(vault_ref) {
    std::cout << "SDImporter initialized with vault\n";
}

// Watch ports safely
void SDImporter::watch_ports() {
    while (true) {
        try {
            for (int port = 0; port < 3; port++) {
                std::string path = "/media/sdcard" + std::to_string(port);
                if (std::filesystem::exists(path)) {
                    std::cout << "SD card detected at " << path << "\n";
                    // TODO: safely import keys from SD card to vault
                }
            }
        } catch (std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << "\n";
        } catch (std::exception& e) {
            std::cerr << "SD watcher exception: " << e.what() << "\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
