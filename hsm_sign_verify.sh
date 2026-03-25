#!/bin/bash
# hsm_sign_verify.sh
# Usage: ./hsm_sign_verify.sh <file_to_sign>

FILE="$1"
SLOT=80451484
PIN=1234
KEYID=10
MODULE="/usr/lib/x86_64-linux-gnu/softhsm/libsofthsm2.so"

if [ -z "$FILE" ]; then
  echo "Usage: $0 <file_to_sign>"
  exit 1
fi

# 1️⃣ Sign the file with SoftHSM
pkcs11-tool \
  --module $MODULE \
  --slot $SLOT \
  --login \
  --pin $PIN \
  --sign \
  --mechanism RSA-PKCS \
  --input-file "$FILE" \
  --output-file "${FILE}.sig" \
  --id $KEYID

# 2️⃣ Export public key
pkcs11-tool \
  --module $MODULE \
  --slot $SLOT \
  --login \
  --pin $PIN \
  --read-object \
  --type pubkey \
  --id $KEYID \
  --output-file pubkey.der

# 3️⃣ Convert DER → PEM
openssl rsa -pubin -inform DER -in pubkey.der -out pubkey.pem

# 4️⃣ Verify signature
openssl pkeyutl \
  -verify \
  -pubin \
  -inkey pubkey.pem \
  -in "$FILE" \
  -sigfile "${FILE}.sig" \
  -pkeyopt digest:sha256

echo "File signed and verified successfully (if no errors shown)"
