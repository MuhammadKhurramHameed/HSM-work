#include <iostream>
#include <thread>
#include <chrono>
#include "vault.h"
#include "crypto.h"
#include "sd_import.h"

int main() {

    std::cout << "Starting PKCS11 daemon\n";

    Vault vault("vault.db");
    vault.init();

    SDImporter importer(vault);

    std::thread sd_thread(
        &SDImporter::watch_ports,
        &importer);

    std::string rsa =
        Crypto::generate_rsa();

    vault.store_key(
        "default-rsa",
        "rsa",
        rsa);

    std::string pq =
        Crypto::pqc_generate();

    vault.store_key(
        "default-pq",
        "dilithium",
        pq);

    std::cout << "Keys initialized\n";

    while (true) {
        std::cout << "daemon alive\n";
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
