#include <oqs/oqs.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool write_bin(const std::string &path, const uint8_t *data, size_t len) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(len));
    return static_cast<bool>(out);
}

int main() {
    OQS_init();

    const char *alg = OQS_SIG_alg_ml_dsa_65;
    OQS_SIG *sig = OQS_SIG_new(alg);
    if (sig == nullptr) {
        std::cerr << "liboqs algorithm not available at compile/runtime: " << alg << "\n";
        OQS_destroy();
        return 1;
    }

    const std::string message = "hello from liboqs";
    std::vector<uint8_t> public_key(sig->length_public_key);
    std::vector<uint8_t> secret_key(sig->length_secret_key);
    std::vector<uint8_t> signature(sig->length_signature);
    size_t signature_len = 0;

    if (OQS_SIG_keypair(sig, public_key.data(), secret_key.data()) != OQS_SUCCESS) {
        std::cerr << "OQS_SIG_keypair failed\n";
        OQS_SIG_free(sig);
        OQS_destroy();
        return 1;
    }

    if (OQS_SIG_sign(sig,
                     signature.data(),
                     &signature_len,
                     reinterpret_cast<const uint8_t *>(message.data()),
                     message.size(),
                     secret_key.data()) != OQS_SUCCESS) {
        std::cerr << "OQS_SIG_sign failed\n";
        OQS_MEM_cleanse(secret_key.data(), secret_key.size());
        OQS_SIG_free(sig);
        OQS_destroy();
        return 1;
    }

    if (OQS_SIG_verify(sig,
                       reinterpret_cast<const uint8_t *>(message.data()),
                       message.size(),
                       signature.data(),
                       signature_len,
                       public_key.data()) != OQS_SUCCESS) {
        std::cerr << "OQS_SIG_verify failed\n";
        OQS_MEM_cleanse(secret_key.data(), secret_key.size());
        OQS_SIG_free(sig);
        OQS_destroy();
        return 1;
    }

    if (!write_bin("pqc_pub.bin", public_key.data(), public_key.size()) ||
        !write_bin("pqc_sig.bin", signature.data(), signature_len)) {
        std::cerr << "failed to write output files\n";
        OQS_MEM_cleanse(secret_key.data(), secret_key.size());
        OQS_SIG_free(sig);
        OQS_destroy();
        return 1;
    }

    {
        std::ofstream msg("pqc_message.txt");
        msg << message;
    }

    {
        std::ofstream rec("pqc_sign_record.json");
        rec << "{\n"
            << "  \"op_id\": \"pqc-sign-0001\",\n"
            << "  \"op\": \"pqc_sign_verify\",\n"
            << "  \"algorithm\": \"" << alg << "\",\n"
            << "  \"message_file\": \"pqc_message.txt\",\n"
            << "  \"public_key_file\": \"pqc_pub.bin\",\n"
            << "  \"signature_file\": \"pqc_sig.bin\",\n"
            << "  \"public_key_len\": " << public_key.size() << ",\n"
            << "  \"signature_len\": " << signature_len << ",\n"
            << "  \"status\": \"ok\"\n"
            << "}\n";
    }

    OQS_MEM_cleanse(secret_key.data(), secret_key.size());
    OQS_SIG_free(sig);
    OQS_destroy();

    std::cout << "PQC sign/verify completed successfully\n";
    return 0;
}
