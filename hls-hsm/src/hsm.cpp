#include "hsm.h"
#include <vector>
#include <random>
#include <cstdint>
#include <string>
#include <map>
#include <iostream>
#include <algorithm> // for std::generate

// Store keys in memory for testing
static std::map<std::string, Key> key_store;

void HSM::init() {
    std::cout << "HSM initialized\n";
}

std::string HSM::generate_rsa_key(int bits) {
    std::string key_id = "RSA_" + std::to_string(bits);
    Key k; // create empty Key stub
    k.data = std::vector<uint8_t>(bits/8, 0); // dummy bytes
    key_store[key_id] = k;
    return key_id;
}

std::string HSM::generate_aes_key(int bits) {
    std::string key_id = "AES_" + std::to_string(bits);
    Key k;
    k.data = std::vector<uint8_t>(bits/8, 0);
    key_store[key_id] = k;
    return key_id;
}

Key HSM::get_key(const std::string& id) {
    if (key_store.find(id) != key_store.end()) {
        return key_store[id];
    }
    return Key{};
}

std::vector<uint8_t> HSM::get_random_bytes(size_t length) {
    std::vector<uint8_t> buf(length);
    std::random_device rd;
    std::generate(buf.begin(), buf.end(), [&rd]() { return rd() & 0xFF; });
    return buf;
}
