#pragma once
#include "vault.h"
#include <string>

class SDImporter {
public:
    explicit SDImporter(Vault& vault_ref);
    void watch_ports();

private:
    Vault& vault;
};
