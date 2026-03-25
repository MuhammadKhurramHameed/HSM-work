#pragma once
#include <string>

class Crypto {

public:
    static std::string generate_rsa();
    static std::string pqc_generate();
};
