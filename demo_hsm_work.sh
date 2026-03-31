#!/usr/bin/env bash
set -euo pipefail

WORK="$HOME/HSM work"
TOY_BUILD="$WORK/pkcs11-daemon/build"
HLS_BUILD="$WORK/hls-hsm/build"

TOY_DAEMON="$TOY_BUILD/pkcs11d"
TOY_MODULE="$TOY_BUILD/libpkcs11lib.so"
HLS_BIN="$HLS_BUILD/hls_hsm"

SOFT_MODULE="/usr/lib/x86_64-linux-gnu/softhsm/libsofthsm2.so"
SOFTHSM2_CONF_FILE="$WORK/softhsm2_demo.conf"
SOFTHSM_TOKENS_DIR="$WORK/softhsm-tokens-demo"

MSG_FILE="$WORK/demo_msg.txt"
PLAINTEXT_FILE="$WORK/demo_plaintext.txt"
SIG_FILE="$WORK/demo_sig.bin"
CIPHERTEXT_FILE="$WORK/demo_ciphertext.bin"
DECRYPTED_FILE="$WORK/demo_decrypted.txt"
RANDOM_FILE="$WORK/demo_random.bin"
READBACK_FILE="$WORK/demo_random_readback.bin"
DAEMON_LOG="$WORK/demo_pkcs11d.log"
HLS_LOG="$WORK/demo_hls_hsm.log"

IV="00000000000000000000000000000000"

section() {
  printf '\n===== %s =====\n' "$1"
}

need() {
  command -v "$1" >/dev/null 2>&1 || { echo "Missing command: $1"; exit 1; }
}

need pkcs11-tool
need softhsm2-util
need timeout

[ -x "$TOY_DAEMON" ] || { echo "Missing: $TOY_DAEMON"; exit 1; }
[ -f "$TOY_MODULE" ] || { echo "Missing: $TOY_MODULE"; exit 1; }
[ -x "$HLS_BIN" ] || { echo "Missing: $HLS_BIN"; exit 1; }
[ -f "$SOFT_MODULE" ] || { echo "Missing: $SOFT_MODULE"; exit 1; }

section "1) Toy PKCS#11 module"
pkcs11-tool --module "$TOY_MODULE" -L

section "2) Toy daemon"
rm -f "$DAEMON_LOG"
set +e
timeout 3 "$TOY_DAEMON" > "$DAEMON_LOG" 2>&1
RC=$?
set -e
if [ "$RC" -ne 0 ] && [ "$RC" -ne 124 ]; then
  echo "pkcs11d failed"
  cat "$DAEMON_LOG"
  exit 1
fi
sed -n '1,20p' "$DAEMON_LOG"

section "3) HLS-HSM standalone"
"$HLS_BIN" | tee "$HLS_LOG"

section "4) Fresh SoftHSM token"
rm -rf "$SOFTHSM_TOKENS_DIR"
mkdir -p "$SOFTHSM_TOKENS_DIR"
chmod 700 "$SOFTHSM_TOKENS_DIR"
printf 'directories.tokendir = %s\n' "$SOFTHSM_TOKENS_DIR" > "$SOFTHSM2_CONF_FILE"
export SOFTHSM2_CONF="$SOFTHSM2_CONF_FILE"
export PKCS11_MODULE_PATH="$SOFT_MODULE"

softhsm2-util --init-token --free --label "dev-token" --so-pin 0000 --pin 1234
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -L

section "5) Demo input files"
printf 'hello from HSM\n' > "$MSG_FILE"
printf 'Hello PKCS11 via AES key in SoftHSM\n' > "$PLAINTEXT_FILE"
head -c 32 /dev/urandom > "$RANDOM_FILE"

section "6) Generate RSA and AES keys in SoftHSM"
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --keypairgen --key-type RSA:2048 --id 01 --label HLS_RSA \
  --usage-sign --usage-decrypt

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --keygen --key-type AES:32 --id 20 --label HLS_AES --usage-decrypt

section "7) Store random bytes as SoftHSM data object"
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --write-object "$RANDOM_FILE" --type data \
  --application-label HLS_RANDOM_DATA

section "8) RSA sign and verify"
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --sign --mechanism SHA256-RSA-PKCS --type privkey --id 01 \
  --input-file "$MSG_FILE" --output-file "$SIG_FILE"

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --verify --mechanism SHA256-RSA-PKCS --type pubkey --id 01 \
  --input-file "$MSG_FILE" --signature-file "$SIG_FILE"

section "9) AES encrypt and decrypt"
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --encrypt --mechanism AES-CBC-PAD --type secrkey --id 20 --iv "$IV" \
  --input-file "$PLAINTEXT_FILE" --output-file "$CIPHERTEXT_FILE"

pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --decrypt --mechanism AES-CBC-PAD --type secrkey --id 20 --iv "$IV" \
  --input-file "$CIPHERTEXT_FILE" --output-file "$DECRYPTED_FILE"

section "10) Read back stored random data"
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 \
  --read-object --type data \
  --application-label HLS_RANDOM_DATA \
  --output-file "$READBACK_FILE"

section "11) Results"
echo "Decrypted text:"
cat "$DECRYPTED_FILE"
echo
if cmp -s "$RANDOM_FILE" "$READBACK_FILE"; then
  echo "Random data readback: OK"
else
  echo "Random data readback: DIFFERENT"
fi

section "12) Objects in SoftHSM"
pkcs11-tool --module "$PKCS11_MODULE_PATH" --token-label "dev-token" -l --pin 1234 -O 2>/dev/null

echo
echo "Demo complete."
echo "Logs:"
echo "  $DAEMON_LOG"
echo "  $HLS_LOG"
