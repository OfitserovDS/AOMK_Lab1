# Eco.AdvancedMath — кубическая сплайн-интерполяция (АОМК, задача 37)

Компонент Eco.Framework (АОМК) на C89 без libc: кубический сплайн (аналог `spline` MATLAB)
с ручной реализацией метода прогонки для трехдиагональной СЛАУ.
Раздел 2 «Продвинутые спецфункции и численный анализ», категория «Высшая сложность».
Автор: Офицеров Д.С.

## Интерфейсы

| Интерфейс | Назначение |
|---|---|
| `IEcoAdvancedMath` (сервис, без состояния) | `spline` / `splinef` / `splinel` — `yq = spline(x, y, xq)` для double / float / long double; `new_Spline` — создать объект `IEcoSpline` |
| `IEcoSpline` (объект данных, pp-форма) | `get_Pieces`, `get_Breaks`, `get_Coefs`, `Evaluate` (ppval), `EvaluateDerivative` (S', S'', S'''), `Integrate` (точный интеграл) |

Граничные условия (`ECO_SPLINE_ENDS_T`, независимо слева и справа): not-a-knot (по умолчанию,
как в MATLAB), естественное (S'' = 0), заданная S', заданная S''.
Коды ошибок — `SharedFiles/ErrEcoAdvancedMath.h` (1001–1006).

CID компонента: `{A2A2065C-8C3B-4926-BC6C-30A84CAA1B2F}`.

## Структура проекта

```
AssemblyFiles/Linux/gcc_v132/  Makefile (компонент), MakefileExe (юнит-тест), build.sh
BuildFiles/                    готовая сборка Linux x86_64: библиотеки .a/.so и юнит-тест
DesignFiles/Report/            отчет EcoAdvancedMath_Report.docx, check_no_libc.sh
DesignFiles/Reference/         эталоны: gen_reference.py (SciPy), bench_scipy.py,
                               spline_check.m и его вывод в MATLAB Online
DesignFiles/Prompt/            системный промт v1.2 и протокол его проверки
HeaderFiles/                   закрытые заголовки; CEcoSplineCore.h - обобщенное ядро алгоритма
SharedFiles/                   IDL, IEcoAdvancedMath.h, IEcoSpline.h, CID, коды ошибок
SourceFiles/                   CEcoAdvancedMath.c, CEcoAdvancedMathFactory.c, CEcoSpline.c
UnitTestFiles/                 юнит-тест (EcoMain) и эталонные значения SciPy
```

## Сборка и запуск (Linux x86_64, gcc)

Нужны переменные окружения `ECO_FRAMEWORK` (каталог Eco.Framework) и `ECO_FRAMEWORK_RT`
(среда выполнения для динамической загрузки компонентов).

Готовая сборка лежит в `BuildFiles/Linux/x86_64`; пересобрать:

```bash
# компонент и юнит-тест во всех конфигурациях: Static|Dynamic x Release|Debug
AssemblyFiles/Linux/gcc_v132/build.sh

# юнит-тест (статическая сборка)
BuildFiles/Linux/x86_64/StaticRelease/EcoAdvancedMath

# динамическая сборка: компонент загружается из среды выполнения Eco
cp BuildFiles/Linux/x86_64/DynamicRelease/A2A2065C8C3B4926BC6C30A84CAA1B2F.so \
   $ECO_FRAMEWORK_RT/00000000000000000000000086640300/
BuildFiles/Linux/x86_64/DynamicRelease/EcoAdvancedMath
```

Юнит-тест выполняет 136 проверок и печатает итог «Проверок: N, ... с ошибками: K»
(код возврата 0 только при K = 0). В текущий каталог он сохраняет два CSV для графиков отчета.

## Проверки

```bash
DesignFiles/Report/check_no_libc.sh          # nm -u, readelf NEEDED, сборка под Eco OS (ARM)
python3 DesignFiles/Reference/gen_reference.py   # перегенерировать эталон (SciPy CubicSpline)
python3 DesignFiles/Reference/bench_scipy.py     # замеры SciPy на данных теста 9.2
```

`DesignFiles/Reference/spline_check.m` запускается в MATLAB (или GNU Octave): пример из
документации `spline`, граничные условия и замеры времени.

## Результаты (10.10.2026)

- Сборка: 8 конфигураций без предупреждений (`-Wall`); юнит-тест 136/136 во всех четырех
  вариантах (StaticRelease, StaticDebug, DynamicRelease, DynamicDebug).
- Точность: совпадение с MATLAB/SciPy не хуже 4,44·10⁻¹⁵ (не менее 14 знаков после запятой).
- Производительность: n = 10⁶ узлов — 60 мс (SciPy — 92 мс, MATLAB Online — 207 мс).
- Компонент не зависит от libc: `nm -u` — только внутренние символы, у `.so` нет NEEDED.
