#!/usr/bin/env bash
# Сборка компонентов-калькуляторов A-E курса в статические библиотеки
echo "Build Eco.CalculatorA-E"
DIR=$(cd "$(dirname "$0")"; pwd)
cd "$DIR" || exit 1
for COMP in A B C D E; do
    make clean -f Makefile COMP=$COMP
    make -f Makefile COMP=$COMP || exit 1
done
