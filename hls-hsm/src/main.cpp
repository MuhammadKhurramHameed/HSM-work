#include "hsm.h"
#include <iostream>

int main() {
    HSM hsm;
    hsm.init();

    auto rsa_id = hsm.generate_rsa_key(2048);
    auto aes_id = hsm.generate_aes_key(256);

    auto key = hsm.get_key(rsa_id);
    std::cout << "Retrieved key: " << key.id << " with size " << key.data.size() << " bytes\n";

    auto random = hsm.get_random_bytes(64);
    std::cout << "Random bytes: ";
    for(auto b : random) std::cout << std::hex << (int)b << " ";
    std::cout << std::dec << "\n";

    return 0;
}
