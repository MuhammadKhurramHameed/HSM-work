#include "crypto.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <memory>
#include <iostream>
#include <vector>

std::string Crypto::generate_rsa() {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        std::cerr << "EVP_PKEY_CTX_new_id failed\n";
        return "";
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        std::cerr << "EVP_PKEY_keygen_init failed\n";
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        std::cerr << "EVP_PKEY_CTX_set_rsa_keygen_bits failed\n";
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        std::cerr << "EVP_PKEY_keygen failed\n";
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    EVP_PKEY_CTX_free(ctx);

    // Write private key to PEM string
    BIO* bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        std::cerr << "PEM_write_bio_PrivateKey failed\n";
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return "";
    }

    char* data;
    long len = BIO_get_mem_data(bio, &data);
    std::string pem(data, len);

    EVP_PKEY_free(pkey);
    BIO_free(bio);

    return pem;
}

// TODO: implement pqc_generate() and other crypto functions

std::string Crypto::pqc_generate() {
    std::cerr << "pqc_generate() not implemented yet\n";
    return "";
}
