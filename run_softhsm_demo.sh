#!/usr/bin/env bash
set -u
export SOFTHSM2_CONF="$HOME/HSM work/softhsm2.conf"
export PKCS11_MODULE_PATH=/usr/lib/x86_64-linux-gnu/softhsm/libsofthsm2.so

rm -rf "$HOME/HSM work/softhsm-tokens"
mkdir -p "$HOME/HSM work/softhsm-tokens"
chmod 700 "$HOME/HSM work/softhsm-tokens"
printf 'directories.tokendir = %s\n' "$HOME/HSM work/softhsm-tokens" > "$SOFTHSM2_CONF"

softhsm2-util --init-token --free --label "dev-token" --so-pin 0000 --pin 1234 || exit 1

printf 'hello from HSM\n' > "$HOME/HSM work/msg.txt"
printf 'Hello PKCS11\n' > "$HOME/HSM work/plaintext.txt"
head -c 32 /dev/urandom > "$HOME/HSM work/random.bin"

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --keypairgen --key-type RSA:2048 --id 01 --label HLS_RSA --usage-sign --usage-decrypt || exit 1

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --keygen --key-type AES:32 --id 20 --label HLS_AES --usage-decrypt || exit 1

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --write-object "$HOME/HSM work/random.bin" --type data --id 30 --label HLS_RANDOM_DATA || exit 1

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --sign --mechanism SHA256-RSA-PKCS --type privkey --id 01 \
  --input-file "$HOME/HSM work/msg.txt" --output-file "$HOME/HSM work/sig.bin" || exit 1

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --verify --mechanism SHA256-RSA-PKCS --type pubkey --id 01 \
  --input-file "$HOME/HSM work/msg.txt" --signature-file "$HOME/HSM work/sig.bin" || exit 1

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --encrypt --mechanism RSA-PKCS --type pubkey --id 01 \
  --input-file "$HOME/HSM work/plaintext.txt" --output-file "$HOME/HSM work/ciphertext.bin" || exit 1

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --decrypt --mechanism RSA-PKCS --type privkey --id 01 \
  --input-file "$HOME/HSM work/ciphertext.bin" --output-file "$HOME/HSM work/decrypted.txt" || exit 1

echo "===== DECRYPTED TEXT ====="
cat "$HOME/HSM work/decrypted.txt"
echo
echo "===== OBJECTS IN SOFTHSM ====="
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 -O
