#pragma once
#include <string>
#include <sqlite3.h>

class Vault {
private:
    sqlite3* db;

public:
    Vault(const std::string& path);
    void init();
    void store_key(
        const std::string& label,
        const std::string& type,
        const std::string& data);

    std::string get_key(const std::string& label);
};
