#!/usr/bin/env bash
set -euo pipefail

export SOFTHSM2_CONF="$HOME/HSM work/softhsm2.conf"
export PKCS11_MODULE_PATH=/usr/lib/x86_64-linux-gnu/softhsm/libsofthsm2.so

LABEL="${1:?usage: store_record_in_softhsm.sh <label> <file> [id] }"
FILE="${2:?usage: store_record_in_softhsm.sh <label> <file> [id] }"
ID="${3:-$(openssl rand -hex 4)}"

pkcs11-tool --module "$PKCS11_MODULE_PATH" \
  --token-label "dev-token" -l --pin 1234 \
  --write-object "$FILE" --type data --id "$ID" --label "$LABEL"
