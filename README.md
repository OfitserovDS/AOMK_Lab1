# Лабораторные работы АОМК (Eco.Framework): Eco.AdvancedMath и Eco.Lab2

Автор: Офицеров Д.С., группа 24ТИДИ. Компоненты на C89 без libc, Linux x86_64 (gcc).

- **Лабораторная работа 1** - компонент `Eco.AdvancedMath` (корень репозитория): кубическая
  сплайн-интерполяция (задача 37, аналог `spline` MATLAB) с ручной реализацией метода прогонки.
- **Лабораторная работа 2** - включение и агрегирование: `Eco.AdvancedMath` поддерживает
  агрегирование и предоставляет интерфейсы калькулятора компонентов курса A-E; новый компонент
  `Eco.Lab2` (каталог `Eco.Lab2/`) агрегирует `Eco.AdvancedMath` и включает калькулятор E.

## Компоненты и интерфейсы

| Компонент | CID | Интерфейсы |
|---|---|---|
| Eco.AdvancedMath | `{A2A2065C-8C3B-4926-BC6C-30A84CAA1B2F}` | `IEcoAdvancedMath` (сплайн: `spline` / `splinef` / `splinel`, `new_Spline`), объект `IEcoSpline` (pp-форма); `IEcoCalculatorX` - агрегирование B (без B - включение A или C); `IEcoCalculatorY` - включение D (без D - E, затем C) |
| Eco.Lab2 | `{1EF402B8-755C-4289-BBE1-795A2F9385F6}` | `IEcoLab2` (`Calculate`, `SplineMean`); `IEcoAdvancedMath` - агрегирование Eco.AdvancedMath; `IEcoCalculatorX` / `IEcoCalculatorY` - включение E (внутри E агрегирован B) |
| Eco.CalculatorA-E | см. `DependenciesFiles/README.md` | компоненты курса (001.InsideACOM), исходники без изменений |

Через любой интерфейс компонента методом `QueryInterface` можно получить любой другой.
Коды ошибок: `SharedFiles/ErrEcoAdvancedMath.h` (1001-1006), `Eco.Lab2/SharedFiles/ErrEcoLab2.h` (2001-2003).

## Структура репозитория

```
AssemblyFiles/Linux/gcc_v132/  Eco.AdvancedMath: Makefile (компонент), MakefileExe (тест), build.sh
BuildFiles/                    готовая сборка Eco.AdvancedMath: .a/.so и юнит-тест
DesignFiles/Report/            отчет по лабораторной работе 1, check_no_libc.sh
DesignFiles/Reference/         эталоны лабораторной работы 1 (SciPy, MATLAB)
DesignFiles/Prompt/            системный промт v1.2 и протокол его проверки
HeaderFiles/, SharedFiles/,    исходники Eco.AdvancedMath
SourceFiles/, UnitTestFiles/
DependenciesFiles/             компоненты-калькуляторы курса A-E (без изменений) и их сборка
Eco.Lab2/                      компонент лабораторной работы 2 (своя структура папок Eco):
  AssemblyFiles/Linux/gcc_v132/  Makefile, MakefileExe, build.sh (собирает всю цепочку)
  BuildFiles/                    готовая сборка Eco.Lab2: .a/.so и юнит-тест
  DesignFiles/Report/            отчет по лабораторной работе 2, check_no_libc.sh
  HeaderFiles/, SharedFiles/, SourceFiles/, UnitTestFiles/
```

## Сборка (Linux x86_64, gcc)

Нужны переменные окружения `ECO_FRAMEWORK` (каталог Eco.Framework) и `ECO_FRAMEWORK_RT`
(среда выполнения для динамической загрузки компонентов). Готовая сборка уже лежит
в `BuildFiles/` и `Eco.Lab2/BuildFiles/`; пересобрать:

```bash
AssemblyFiles/Linux/gcc_v132/build.sh            # лабораторная работа 1: компонент и тест, 4 конфигурации
Eco.Lab2/AssemblyFiles/Linux/gcc_v132/build.sh   # лабораторная работа 2: калькуляторы A-E,
                                                 # Eco.AdvancedMath, Eco.Lab2 и тест, 4 конфигурации
```

## Запуск юнит-тестов

```bash
BuildFiles/Linux/x86_64/StaticRelease/EcoAdvancedMath     # лабораторная работа 1: 136 проверок
Eco.Lab2/BuildFiles/Linux/x86_64/StaticRelease/EcoLab2    # лабораторная работа 2: 37 проверок
```

Тест печатает итог «Проверок: N, ... с ошибками: K» и возвращает 0 только при K = 0.
Тест лабораторной работы 1 сохраняет в текущий каталог два CSV для графиков отчета.

Динамические сборки загружают компоненты из `$ECO_FRAMEWORK_RT/00000000000000000000000086640300/`,
туда нужно скопировать `.so` (калькуляторы тест лабораторной работы 2 регистрирует сам):

```bash
cp BuildFiles/Linux/x86_64/DynamicRelease/A2A2065C8C3B4926BC6C30A84CAA1B2F.so \
   Eco.Lab2/BuildFiles/Linux/x86_64/DynamicRelease/1EF402B8755C4289BBE1795A2F9385F6.so \
   $ECO_FRAMEWORK_RT/00000000000000000000000086640300/
BuildFiles/Linux/x86_64/DynamicRelease/EcoAdvancedMath
Eco.Lab2/BuildFiles/Linux/x86_64/DynamicRelease/EcoLab2
```

## Проверки

```bash
DesignFiles/Report/check_no_libc.sh              # Eco.AdvancedMath: nm -u, readelf NEEDED, сборка под ARM
Eco.Lab2/DesignFiles/Report/check_no_libc.sh     # то же для Eco.Lab2
python3 DesignFiles/Reference/gen_reference.py   # эталон лабораторной работы 1 (SciPy CubicSpline)
python3 DesignFiles/Reference/bench_scipy.py     # замеры SciPy
```

`DesignFiles/Reference/spline_check.m` запускается в MATLAB (или GNU Octave).

## Результаты

- Сборка всех компонентов и тестов в 4 конфигурациях (Static|Dynamic x Release|Debug) - без
  предупреждений; юнит-тесты: лабораторная работа 1 - 136/136, лабораторная работа 2 - 37/37
  во всех конфигурациях.
- Лабораторная работа 1: совпадение с MATLAB/SciPy не хуже 4,44·10⁻¹⁵ (не менее 14 знаков после
  запятой); n = 10⁶ узлов - 60 мс (SciPy - 92 мс, MATLAB Online - 207 мс).
- Лабораторная работа 2: задействованы все компоненты курса (A, C, D, E - включение, B -
  агрегирование, C - вторая версия A под тем же CID) и Eco.AdvancedMath (агрегирование в Eco.Lab2).
- Компоненты Eco.AdvancedMath и Eco.Lab2 не зависят от libc: `nm -u` - только внутренние символы,
  у `.so` нет NEEDED.
