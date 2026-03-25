#include "vault.h"
#include <iostream>

Vault::Vault(const std::string& path) {
    sqlite3_open(path.c_str(), &db);
}

void Vault::init() {

    const char* sql =
        "CREATE TABLE IF NOT EXISTS keys("
        "label TEXT PRIMARY KEY,"
        "type TEXT,"
        "data TEXT);";

    sqlite3_exec(db, sql, 0, 0, 0);
}

void Vault::store_key(
    const std::string& label,
    const std::string& type,
    const std::string& data) {

    std::string sql =
        "INSERT OR REPLACE INTO keys VALUES('"
        + label + "','" + type + "','" + data + "');";

    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
}

std::string Vault::get_key(const std::string& label) {

    std::string result;
    sqlite3_stmt* stmt;

    std::string sql =
        "SELECT data FROM keys WHERE label='" + label + "';";

    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = (char*)sqlite3_column_text(stmt, 0);

    sqlite3_finalize(stmt);

    return result;
}
