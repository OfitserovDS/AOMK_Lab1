#!/usr/bin/env bash
# Проверка отсутствия зависимостей компонента EcoAdvancedMath от стандартной библиотеки C (libc).
#   1) nm -u     - неразрешенные символы статической библиотеки (Linux x86_64);
#   2) readelf   - зависимости (NEEDED) динамической библиотеки .so;
#   3) arm-none-eabi-gcc - сборка исходников компонента для Eco OS (ARM Cortex-M4, -ffreestanding).
# Запуск из корня проекта после сборки: DesignFiles/Report/check_no_libc.sh
# Результат печатается в стандартный вывод (сохранить: check_no_libc.sh > no_libc.txt)

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
LIB="$ROOT/BuildFiles/Linux/x86_64/StaticRelease/libA2A2065C8C3B4926BC6C30A84CAA1B2F.a"
SO="$ROOT/BuildFiles/Linux/x86_64/DynamicRelease/A2A2065C8C3B4926BC6C30A84CAA1B2F.so"
FW="${ECO_FRAMEWORK:?ECO_FRAMEWORK is not set}"

{
    echo "\$ nm -u libA2A2065C8C3B4926BC6C30A84CAA1B2F.a   (Linux x86_64, StaticRelease)"
    nm -u "$LIB" | grep -v -E '^$'
    echo
    echo "\$ readelf -d A2A2065C8C3B4926BC6C30A84CAA1B2F.so | grep NEEDED   (Linux x86_64, DynamicRelease)"
    NEEDED=$(readelf -d "$SO" | grep NEEDED)
    if [ -z "$NEEDED" ]; then echo "(пусто: библиотека не зависит даже от libc.so)"; else echo "$NEEDED"; fi
    echo
    if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
        TMPD=$(mktemp -d)
        INC="-I $FW/Eco.Core1/SharedFiles -I $FW/Eco.InterfaceBus1/SharedFiles -I $FW/Eco.MemoryManager1/SharedFiles -I $FW/Eco.FileSystemManagement1/SharedFiles -I $FW/Eco.System1/SharedFiles -I $ROOT/HeaderFiles -I $ROOT/SharedFiles"
        for c in A B D E; do INC="$INC -I $ROOT/DependenciesFiles/Eco.Calculator$c/SharedFiles"; done
        for f in "$ROOT"/SourceFiles/*.c; do
            arm-none-eabi-gcc -std=gnu89 -Wall -fms-extensions -ffreestanding -O2 -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
                -DECO_LIB -DUGUID_UTILITY -DECO_OS -DECO_AARCH32 $INC -c "$f" -o "$TMPD/$(basename "$f" .c).o"
        done
        arm-none-eabi-ar rcs "$TMPD/libA2A2065C8C3B4926BC6C30A84CAA1B2F.a" "$TMPD"/*.o
        echo "\$ arm-none-eabi-nm -u libA2A2065C8C3B4926BC6C30A84CAA1B2F.a   (Eco OS, ARM Cortex-M4, -ffreestanding -O2)"
        arm-none-eabi-nm -u "$TMPD/libA2A2065C8C3B4926BC6C30A84CAA1B2F.a" | grep -v -E '^$'
        echo
        echo "\$ arm-none-eabi-size *.o"
        (cd "$TMPD" && arm-none-eabi-size CEcoAdvancedMath.o CEcoAdvancedMathFactory.o CEcoSpline.o)
        rm -r -- "${TMPD:?}"
    else
        echo "(arm-none-eabi-gcc не найден - проверка для Eco OS пропущена)"
    fi
}
