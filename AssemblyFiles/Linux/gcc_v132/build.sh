#!/usr/bin/env bash
set -e
cd -- "$(dirname -- "$0")"
echo "Build Eco.AdvancedMath"

for TARGET in 0 1; do
    for DEBUG in 0 1; do
        for BUILD_FILE in Makefile MakefileExe; do
            make -f "$BUILD_FILE" clean TARGET="$TARGET" DEBUG="$DEBUG" ARCH=x86_64
            make -f "$BUILD_FILE" TARGET="$TARGET" DEBUG="$DEBUG" ARCH=x86_64
        done
    done
done
