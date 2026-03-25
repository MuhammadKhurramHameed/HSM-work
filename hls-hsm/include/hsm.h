#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <memory>
#include <iostream>

enum class KeyType { RSA, AES };

struct Key {
    std::string id;
    KeyType type;
    std::vector<uint8_t> data;
};

class HSM {
public:
    // Initialize HSM
    void init();

    // Key management
    std::string generate_rsa_key(int bits = 2048);
    std::string generate_aes_key(int bits = 256);
    Key get_key(const std::string& id);

    // Random number generation
    std::vector<uint8_t> get_random_bytes(size_t length);

private:
    std::map<std::string, Key> keys_;
    int key_counter_ = 0;
};
