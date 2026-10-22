#!/usr/bin/env bash
# Сборка лабораторной работы 2: калькуляторы A-E курса, компонент Eco.AdvancedMath (лабораторная
# работа 1), компонент Eco.Lab2 и его юнит-тест во всех конфигурациях (Static|Dynamic x Release|Debug)
echo "Build Eco.Lab2"
DIR=$(cd "$(dirname "$0")"; pwd)
ROOT=$(cd "$DIR/../../../.."; pwd)
echo $DIR

# Внутренние компоненты: калькуляторы A-E (статические библиотеки)
"$ROOT/DependenciesFiles/AssemblyFiles/Linux/gcc_v132/build.sh" || exit 1

# Внутренний компонент Eco.AdvancedMath
cd "$ROOT/AssemblyFiles/Linux/gcc_v132" || exit 1
for TARGET in 0 1; do
    for DEBUG in 0 1; do
        make clean -f Makefile TARGET=$TARGET DEBUG=$DEBUG ARCH=x86_64
        make -f Makefile TARGET=$TARGET DEBUG=$DEBUG ARCH=x86_64 || exit 1
    done
done

# Компонент Eco.Lab2 и юнит-тест
cd "$DIR" || exit 1
for TARGET in 0 1; do
    for DEBUG in 0 1; do
        make clean -f Makefile TARGET=$TARGET DEBUG=$DEBUG ARCH=x86_64
        make -f Makefile TARGET=$TARGET DEBUG=$DEBUG ARCH=x86_64 || exit 1
        make clean -f MakefileExe TARGET=$TARGET DEBUG=$DEBUG ARCH=x86_64
        make -f MakefileExe TARGET=$TARGET DEBUG=$DEBUG ARCH=x86_64 || exit 1
    done
done
