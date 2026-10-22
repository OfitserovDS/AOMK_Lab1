# DependenciesFiles - компоненты-калькуляторы курса

Исходные файлы компонентов `Eco.CalculatorA` ... `Eco.CalculatorE` из репозитория курса
<https://github.com/peerf-eco/Eco.Education> (коммит `7f1664b0489bcea6d5b211807ea5cb0ce0e1068e`,
каталог `Eco.Pro.Training/001.InsideACOM/Lessons`, уроки 02-07). Каталоги `SharedFiles`,
`HeaderFiles` и `SourceFiles` скопированы **без изменений**.

| Компонент | CID | Интерфейсы | Агрегирование | Внутренние компоненты |
|---|---|---|---|---|
| A | `4828F655-2E45-40E7-8121-EBD220DC360E` | IEcoCalculatorX | нет | - |
| B | `AE202E54-3CE5-4550-8996-03BD70C62565` | IEcoCalculatorX | поддерживает | - |
| C | `4828F655-2E45-40E7-8121-EBD220DC360E` (как у A) | IEcoCalculatorX, IEcoCalculatorY | нет | - (вторая версия A) |
| D | `3A8E4467-7E82-475C-B4A3-719ED8397E61` | IEcoCalculatorX, IEcoCalculatorY | нет | B или A (включение) |
| E | `872FEF1D-E331-4B87-AD44-D1E7C232C2F0` | IEcoCalculatorY (+ X от B) | нет | B (агрегирование) |

## Сборка

```bash
DependenciesFiles/AssemblyFiles/Linux/gcc_v132/build.sh     # все пять компонентов
```

Результат - статические библиотеки `Eco.Calculator*/BuildFiles/Linux/x86_64/StaticRelease/lib<CID>.a`.
Исходники написаны под предыдущую версию Eco.Framework, совместимость обеспечивается флагами
компилятора (см. `AssemblyFiles/Linux/gcc_v132/Makefile`):

- `-std=gnu89` - в исходниках есть комментарии вида `//`;
- `-DERR_ECO_SUCCESES=ERR_ECO_SUCCESS` - в предыдущей версии фреймворка код успеха назывался `ERR_ECO_SUCCESES`;
- для C - суффикс `_C` у трех глобальных символов фабрики: у A и C общий CID, а юнит-тест
  лабораторной работы 2 подставляет C на место A в одном исполняемом файле.
