#!/bin/sh
set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <firmware-version>"
    echo "Example: $0 9.0-v2"
    exit 1
fi

VERSION="$1"

if [ "${#VERSION}" -gt 10 ]; then
    echo "Firmware version is too long (maximum 10 characters)."
    exit 1
fi

mkdir -p compiled-firmware

echo "Building TrueMDC UV-K5 V2 Gen${VERSION} with Docker..."

docker build -t truemdc-uvk5-v2 .

docker run --rm \
    -v "$(pwd)/compiled-firmware:/app/compiled-firmware" \
    truemdc-uvk5-v2 /bin/bash -c "cd /app && \
        make clean && \
        make build \
            TARGET_UVK5_V2=1 \
            FIRMWARE_VERSION='${VERSION}' \
            ENABLE_MDC1200=1 \
            ENABLE_MDC1200_EDIT=1 \
            ENABLE_MDC1200_CONTACT=1 \
            ENABLE_FLEETSYNC=1 \
            ENABLE_FMRADIO=0 \
            ENABLE_SPECTRUM=0 && \
        cp firmware.bin compiled-firmware/TrueMDC.UVK5V2.Gen${VERSION}.bin && \
        cp archive/TrueMDC.Gen${VERSION}.packed.bin compiled-firmware/"

echo
echo "Build complete:"
echo "  compiled-firmware/TrueMDC.UVK5V2.Gen${VERSION}.bin"
echo "  compiled-firmware/TrueMDC.UVK5V2.Gen${VERSION}.elf.bin"
echo "  compiled-firmware/TrueMDC.Gen${VERSION}.packed.bin"
