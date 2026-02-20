#!/usr/bin/env bash
# Copyright 2026 Dev Team
#
# Licensed under the Apache License, Version 2.0.

# Download zenoh-bridge-remote-api for the current platform.
# This bridge is required by @eclipse-zenoh/zenoh-ts to connect via WebSocket.

set -euo pipefail

VERSION="1.7.2"
ARCH="$(uname -m)"
OS="$(uname -s)"
BIN_DIR="$(cd "$(dirname "$0")" && pwd)/bin"

case "${OS}-${ARCH}" in
  Linux-aarch64) TARGET="aarch64-unknown-linux-gnu" ;;
  Linux-x86_64)  TARGET="x86_64-unknown-linux-gnu" ;;
  Darwin-arm64)  TARGET="aarch64-apple-darwin" ;;
  Darwin-x86_64) TARGET="x86_64-apple-darwin" ;;
  *) echo "Unsupported platform: ${OS}-${ARCH}" >&2; exit 1 ;;
esac

URL="https://github.com/eclipse-zenoh/zenoh-ts/releases/download/${VERSION}/zenoh-ts-${VERSION}-${TARGET}-standalone.zip"
DEST="${BIN_DIR}/zenoh-bridge-remote-api"

if [ -x "${DEST}" ] && "${DEST}" --version 2>&1 | grep -q "${VERSION}"; then
  echo "zenoh-bridge-remote-api v${VERSION} already installed at ${DEST}"
  exit 0
fi

echo "Downloading zenoh-bridge-remote-api v${VERSION} for ${TARGET}..."
TMP="$(mktemp -d)"
trap 'rm -rf "${TMP}"' EXIT

curl -sL "${URL}" -o "${TMP}/bridge.zip"
unzip -o "${TMP}/bridge.zip" -d "${TMP}/out" > /dev/null

mkdir -p "${BIN_DIR}"
cp "${TMP}/out/zenoh-bridge-remote-api" "${DEST}"
chmod +x "${DEST}"

echo "Installed: ${DEST}"
"${DEST}" --version
