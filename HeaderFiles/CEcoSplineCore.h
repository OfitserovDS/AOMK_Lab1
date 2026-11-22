/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoSplineCore
 * </summary>
 *
 * <description>
 *   Обобщенная («шаблонная») реализация алгоритма кубической сплайн-интерполяции.
 *
 *   В C89 нет шаблонов и перегрузки функций, поэтому один и тот же код алгоритма
 *   используется для float, double и long double так: файл подключается в .c-файл
 *   несколько раз, и перед каждым подключением определяются макросы
 *       ECO_SPLINE_REAL      - вещественный тип (float_t, double_t, ldouble_t);
 *       ECO_SPLINE_FN(name)  - правило именования функций для этого типа (name##_f и т.п.).
 *   Набор функций выбирается макросами разделов:
 *       ECO_SPLINE_CORE_SOLVER - проверка данных, СЛАУ для наклонов, метод прогонки, коэффициенты;
 *       ECO_SPLINE_CORE_EVAL   - поиск куска и вычисление значений;
 *       ECO_SPLINE_CORE_DRIVER - полный цикл «построить и вычислить» (нужен IEcoMemoryAllocator1).
 *   Все функции static, поэтому в библиотеке не появляется внешних символов.
 *
 *   Математическая модель (та же, что в spline.m MATLAB и CubicSpline SciPy).
 *   Неизвестные - наклоны s[i] = S'(x[i]). На куске [x[k], x[k+1]] сплайн - кубический
 *   многочлен Эрмита, заданный значениями y[k], y[k+1] и наклонами s[k], s[k+1].
 *   Непрерывность S'' во внутренних узлах дает трехдиагональную СЛАУ:
 *       h[i]*s[i-1] + 2*(h[i-1] + h[i])*s[i] + h[i-1]*s[i+1] = 3*(h[i]*d[i-1] + h[i-1]*d[i]),
 *       h[i] = x[i+1] - x[i],  d[i] = (y[i+1] - y[i]) / h[i],
 *   первая и последняя строки задаются граничными условиями. Система решается методом
 *   прогонки за O(n) операций.
 *
 *   В коде нет вызовов функций стандартной библиотеки C (libc): память выделяется только
 *   через IEcoMemoryAllocator1, все вычисления - на арифметике языка.
 * </description>
 *
 * <reference>
 *   https://en.wikipedia.org/wiki/Spline_interpolation
 *   https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
 *   C. de Boor. A Practical Guide to Splines. Springer, 1978 (2001).
 *   А.А. Самарский, Е.С. Николаев. Методы решения сеточных уравнений. М.: Наука, 1978.
 * </reference>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

/* Файл намеренно не имеет защиты от повторного включения (см. описание выше). */

#if !defined(ECO_SPLINE_REAL) || !defined(ECO_SPLINE_FN)
#error "ECO_SPLINE_REAL and ECO_SPLINE_FN must be defined before including CEcoSplineCore.h"
#endif


/* ========================================================================== */
/*   Общая часть (не зависит от вещественного типа, включается один раз)      */
/* ========================================================================== */

#ifndef ECO_SPLINE_CORE_COMMON_DEFINED
#define ECO_SPLINE_CORE_COMMON_DEFINED

#include "IEcoAdvancedMath.h"
#include "ErrEcoAdvancedMath.h"
#include "CEcoSplineEvents.h"

/*
 * <summary>
 *   EcoSplineIsFiniteL
 * </summary>
 *
 * <description>
 *   Проверяет, что значение конечно (не NaN и не бесконечность) без функций libc:
 *   для конечного v разность v - v равна 0, а для NaN и бесконечности дает NaN,
 *   который по стандарту IEEE 754 не равен сам себе.
 * </description>
 */
static bool_t EcoSplineIsFiniteL(/* in */ ldouble_t v) {
    ldouble_t d = v - v;

    return (bool_t)(d == d);
}

/*
 * <summary>
 *   EcoSplineCheckEnds
 * </summary>
 *
 * <description>
 *   Проверяет граничные условия: известный тип и конечное значение (значение
 *   проверяется только для типов, которые его используют). pEnds = 0 допустим
 *   (условие not-a-knot на обоих концах).
 *   Возвращает ERR_ECO_SUCCESS, ERR_ECOADVANCEDMATH_BAD_END_CONDITION или
 *   ERR_ECOADVANCEDMATH_NOT_FINITE.
 * </description>
 */
static int16_t EcoSplineCheckEnds(/* in */ const ECO_SPLINE_ENDS_T* pEnds) {

    if (pEnds == 0) {
        return ERR_ECO_SUCCESS;
    }
    if (pEnds->leftType > ECO_SPLINE_END_SECOND_DERIVATIVE || pEnds->rightType > ECO_SPLINE_END_SECOND_DERIVATIVE) {
        return ERR_ECOADVANCEDMATH_BAD_END_CONDITION;
    }
    if (pEnds->leftType >= ECO_SPLINE_END_FIRST_DERIVATIVE && !EcoSplineIsFiniteL(pEnds->leftValue)) {
        return ERR_ECOADVANCEDMATH_NOT_FINITE;
    }
    if (pEnds->rightType >= ECO_SPLINE_END_FIRST_DERIVATIVE && !EcoSplineIsFiniteL(pEnds->rightValue)) {
        return ERR_ECOADVANCEDMATH_NOT_FINITE;
    }
    return ERR_ECO_SUCCESS;
}

#endif /* ECO_SPLINE_CORE_COMMON_DEFINED */


/* ========================================================================== */
/*   Раздел SOLVER: проверка данных, построение СЛАУ, метод прогонки          */
/* ========================================================================== */

#ifdef ECO_SPLINE_CORE_SOLVER

/*
 * <summary>
 *   EcoSplineIsFinite
 * </summary>
 *
 * <description>
 *   Проверка конечности значения (см. EcoSplineIsFiniteL) для типа ECO_SPLINE_REAL.
 * </description>
 */
static bool_t ECO_SPLINE_FN(EcoSplineIsFinite)(/* in */ ECO_SPLINE_REAL v) {
    ECO_SPLINE_REAL d = v - v;

    return (bool_t)(d == d);
}

/*
 * <summary>
 *   EcoSplineCheckData
 * </summary>
 *
 * <description>
 *   Проверяет узлы интерполяции: n >= 2, все x[i] и y[i] конечны, x строго возрастают.
 *   Сравнение записано как !(x[i-1] < x[i]), поэтому повторяющиеся узлы и NaN
 *   отбраковываются.
 *   Возвращает ERR_ECO_SUCCESS или код ошибки ERR_ECOADVANCEDMATH_*.
 * </description>
 */
static int16_t ECO_SPLINE_FN(EcoSplineCheckData)(/* in */ const ECO_SPLINE_REAL* x, /* in */ const ECO_SPLINE_REAL* y, /* in */ uint32_t n) {
    uint32_t i = 0;

    if (n < 2) {
        return ERR_ECOADVANCEDMATH_TOO_FEW_POINTS;
    }
    for (i = 0; i < n; i++) {
        if (!ECO_SPLINE_FN(EcoSplineIsFinite)(x[i]) || !ECO_SPLINE_FN(EcoSplineIsFinite)(y[i])) {
            return ERR_ECOADVANCEDMATH_NOT_FINITE;
        }
        if (i > 0 && !(x[i - 1] < x[i])) {
            return ERR_ECOADVANCEDMATH_NOT_INCREASING;
        }
    }
    return ERR_ECO_SUCCESS;
}

/*
 * <summary>
 *   EcoSplineThomas - метод прогонки (алгоритм Томаса)
 * </summary>
 *
 * <description>
 *   Решает трехдиагональную систему из n уравнений
 *       a[i]*s[i-1] + b[i]*s[i] + c[i]*s[i+1] = f[i],   i = 0..n-1
 *   (a[0] и c[n-1] не используются). Правая часть f передается в массиве s и
 *   на выходе заменяется решением.
 *
 *   Прямой ход - вычисление прогоночных коэффициентов:
 *       alpha[0] = -c[0] / b[0],   beta[0] = f[0] / b[0],
 *       alpha[i] = -c[i] / (b[i] + a[i]*alpha[i-1]),
 *       beta[i]  = (f[i] - a[i]*beta[i-1]) / (b[i] + a[i]*alpha[i-1]).
 *   Обратный ход:
 *       s[n-1] = beta[n-1],   s[i] = alpha[i]*s[i+1] + beta[i].
 *   alpha хранится на месте c, beta - на месте s: дополнительная память не нужна.
 *   Сложность O(n): 3(n-1) умножений/делений в прямом ходе и n-1 в обратном.
 *
 *   Возвращает ERR_ECOADVANCEDMATH_SINGULAR, если знаменатель обратился в 0.
 * </description>
 */
static int16_t ECO_SPLINE_FN(EcoSplineThomas)(/* in */ const ECO_SPLINE_REAL* a, /* in */ const ECO_SPLINE_REAL* b, /* in, out */ ECO_SPLINE_REAL* c, /* in, out */ ECO_SPLINE_REAL* s, /* in */ uint32_t n, /* in */ CEcoSplineTrace* pTrace) {
    ECO_SPLINE_REAL denom = 0;
    uint32_t i = 0;

    /* Прямой ход прогонки */
    denom = b[0];
    if (denom == 0) {
        return ERR_ECOADVANCEDMATH_SINGULAR;
    }
    c[0] = -c[0] / denom;
    s[0] = s[0] / denom;
    if (pTrace != 0) {
        EcoSplineNotifyForward(pTrace, 0, c[0], s[0]);
    }
    for (i = 1; i < n; i++) {
        denom = b[i] + a[i] * c[i - 1];
        if (denom == 0) {
            return ERR_ECOADVANCEDMATH_SINGULAR;
        }
        c[i] = -c[i] / denom;
        s[i] = (s[i] - a[i] * s[i - 1]) / denom;
        if (pTrace != 0) {
            EcoSplineNotifyForward(pTrace, i, c[i], s[i]);
        }
    }

    /* Обратный ход прогонки: s[n-1] = beta[n-1] уже на месте */
    if (pTrace != 0) {
        EcoSplineNotifySlope(pTrace, n - 1, s[n - 1]);
    }
    for (i = n - 1; i > 0; i--) {
        s[i - 1] = c[i - 1] * s[i] + s[i - 1];
        if (pTrace != 0) {
            EcoSplineNotifySlope(pTrace, i - 1, s[i - 1]);
        }
    }
    return ERR_ECO_SUCCESS;
}

/*
 * <summary>
 *   EcoSplineSolveSlopes - вычисление наклонов s[i] = S'(x[i])
 * </summary>
 *
 * <description>
 *   Формирует трехдиагональную СЛАУ для наклонов и решает ее методом прогонки.
 *   pWork - рабочий массив из 3*n элементов (диагонали a, b, c), результат - в s[0..n-1].
 *   Входные данные должны быть предварительно проверены (EcoSplineCheckData/CheckEnds).
 *
 *   Граничные условия (строка 0 - левый конец, строка n-1 - правый):
 *     not-a-knot:     h[1]*s[0] + (x[2]-x[0])*s[1] = ((h[0] + 2(x[2]-x[0]))*h[1]*d[0] + h[0]^2*d[1]) / (x[2]-x[0])
 *                     (непрерывность S''' в x[1]; справа - симметрично, формулы как в spline.m);
 *     естественное:   2*s[0] + s[1] = 3*d[0]                       (S''(x[0]) = 0);
 *     задана S':      s[0] = value;
 *     задана S'':     2*s[0] + s[1] = 3*d[0] - value*h[0]/2.
 *   Вырожденные случаи not-a-knot на обоих концах, как в MATLAB: n = 2 - прямая,
 *   n = 3 - парабола через три точки. При n = 2 и not-a-knot только на одном конце
 *   на этом конце берется наклон хорды (как в SciPy CubicSpline).
 * </description>
 */
static int16_t ECO_SPLINE_FN(EcoSplineSolveSlopes)(/* in */ const ECO_SPLINE_REAL* x, /* in */ const ECO_SPLINE_REAL* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* out */ ECO_SPLINE_REAL* s, /* in */ ECO_SPLINE_REAL* pWork, /* in */ CEcoSplineTrace* pTrace) {
    ECO_SPLINE_REAL* a = pWork;           /* поддиагональ:     a[i] при s[i-1] */
    ECO_SPLINE_REAL* b = pWork + n;       /* главная диагональ: b[i] при s[i]  */
    ECO_SPLINE_REAL* c = pWork + 2 * n;   /* наддиагональ:     c[i] при s[i+1] */
    ECO_SPLINE_REAL hPrev = 0;
    ECO_SPLINE_REAL hNext = 0;
    ECO_SPLINE_REAL dPrev = 0;
    ECO_SPLINE_REAL dNext = 0;
    ECO_SPLINE_REAL span = 0;
    ECO_SPLINE_REAL c2 = 0;
    uint16_t leftType = ECO_SPLINE_END_NOT_A_KNOT;
    uint16_t rightType = ECO_SPLINE_END_NOT_A_KNOT;
    uint32_t i = 0;

    if (pEnds != 0) {
        leftType = pEnds->leftType;
        rightType = pEnds->rightType;
    }

    /* Вырожденные случаи not-a-knot на обоих концах (n = 2 и n = 3) */
    if (leftType == ECO_SPLINE_END_NOT_A_KNOT && rightType == ECO_SPLINE_END_NOT_A_KNOT && n <= 3) {
        hPrev = x[1] - x[0];
        dPrev = (y[1] - y[0]) / hPrev;
        if (n == 2) {
            /* Прямая через две точки */
            s[0] = dPrev;
            s[1] = dPrev;
        }
        else {
            /* Парабола p(x) = y[0] + d[0]*(x - x[0]) + c2*(x - x[0])*(x - x[1]) */
            hNext = x[2] - x[1];
            dNext = (y[2] - y[1]) / hNext;
            c2 = (dNext - dPrev) / (x[2] - x[0]);
            s[0] = dPrev - c2 * hPrev;
            s[1] = dPrev + c2 * hPrev;
            s[2] = dPrev + c2 * (hPrev + 2 * hNext);
        }
        if (pTrace != 0) {
            for (i = n; i > 0; --i) {
                EcoSplineNotifySlope(pTrace, i - 1, s[i - 1]);
            }
        }
        return ERR_ECO_SUCCESS;
    }

    /* Внутренние строки i = 1..n-2 (правая часть записывается в s) */
    hPrev = x[1] - x[0];
    dPrev = (y[1] - y[0]) / hPrev;
    for (i = 1; i + 1 < n; i++) {
        hNext = x[i + 1] - x[i];
        dNext = (y[i + 1] - y[i]) / hNext;
        a[i] = hNext;
        b[i] = 2 * (hNext + hPrev);
        c[i] = hPrev;
        s[i] = 3 * (hNext * dPrev + hPrev * dNext);
        hPrev = hNext;
        dPrev = dNext;
    }

    /* Строка 0 - левое граничное условие */
    hPrev = x[1] - x[0];
    dPrev = (y[1] - y[0]) / hPrev;
    a[0] = 0;
    switch (leftType) {
    case ECO_SPLINE_END_NOT_A_KNOT:
        if (n == 2) {
            b[0] = 1;
            c[0] = 0;
            s[0] = dPrev;
        }
        else {
            hNext = x[2] - x[1];
            dNext = (y[2] - y[1]) / hNext;
            span = x[2] - x[0];
            b[0] = hNext;
            c[0] = span;
            s[0] = ((hPrev + 2 * span) * hNext * dPrev + hPrev * hPrev * dNext) / span;
        }
        break;
    case ECO_SPLINE_END_NATURAL:
        b[0] = 2;
        c[0] = 1;
        s[0] = 3 * dPrev;
        break;
    case ECO_SPLINE_END_FIRST_DERIVATIVE:
        b[0] = 1;
        c[0] = 0;
        s[0] = (ECO_SPLINE_REAL)pEnds->leftValue;
        break;
    case ECO_SPLINE_END_SECOND_DERIVATIVE:
        b[0] = 2;
        c[0] = 1;
        s[0] = 3 * dPrev - (ECO_SPLINE_REAL)pEnds->leftValue * hPrev / 2;
        break;
    default:
        return ERR_ECOADVANCEDMATH_BAD_END_CONDITION;
    }

    /* Строка n-1 - правое граничное условие */
    hNext = x[n - 1] - x[n - 2];
    dNext = (y[n - 1] - y[n - 2]) / hNext;
    c[n - 1] = 0;
    switch (rightType) {
    case ECO_SPLINE_END_NOT_A_KNOT:
        if (n == 2) {
            a[n - 1] = 0;
            b[n - 1] = 1;
            s[n - 1] = dNext;
        }
        else {
            hPrev = x[n - 2] - x[n - 3];
            dPrev = (y[n - 2] - y[n - 3]) / hPrev;
            span = x[n - 1] - x[n - 3];
            a[n - 1] = span;
            b[n - 1] = hPrev;
            s[n - 1] = (hNext * hNext * dPrev + (2 * span + hNext) * hPrev * dNext) / span;
        }
        break;
    case ECO_SPLINE_END_NATURAL:
        a[n - 1] = 1;
        b[n - 1] = 2;
        s[n - 1] = 3 * dNext;
        break;
    case ECO_SPLINE_END_FIRST_DERIVATIVE:
        a[n - 1] = 0;
        b[n - 1] = 1;
        s[n - 1] = (ECO_SPLINE_REAL)pEnds->rightValue;
        break;
    case ECO_SPLINE_END_SECOND_DERIVATIVE:
        a[n - 1] = 1;
        b[n - 1] = 2;
        s[n - 1] = 3 * dNext + (ECO_SPLINE_REAL)pEnds->rightValue * hNext / 2;
        break;
    default:
        return ERR_ECOADVANCEDMATH_BAD_END_CONDITION;
    }

    /* Решение СЛАУ методом прогонки */
    if (pTrace != 0) {
        for (i = 0; i < n; ++i) {
            EcoSplineNotifyRow(pTrace, i, a[i], b[i], c[i], s[i]);
        }
    }
    return ECO_SPLINE_FN(EcoSplineThomas)(a, b, c, s, n, pTrace);
}

/*
 * <summary>
 *   EcoSplineBuildCoefs - переход от наклонов к кусочно-полиномиальной форме
 * </summary>
 *
 * <description>
 *   Для каждого куска k = 0..n-2 вычисляет коэффициенты многочлена
 *       S(x) = a*t^3 + b*t^2 + c*t + d,   t = x - x[k],
 *   по значениям y[k], y[k+1] и наклонам s[k], s[k+1] (кубический многочлен Эрмита):
 *       c1 = (d[k] - s[k]) / h,   c2 = (s[k+1] - d[k]) / h,
 *       a = (c2 - c1) / h,   b = 2*c1 - c2,   c = s[k],   d = y[k]
 *   (порядок операций тот же, что в функции pwch MATLAB).
 *   Результат: coefs[4*k + 0..3] = {a, b, c, d}.
 * </description>
 */
static void ECO_SPLINE_FN(EcoSplineBuildCoefs)(/* in */ const ECO_SPLINE_REAL* x, /* in */ const ECO_SPLINE_REAL* y, /* in */ const ECO_SPLINE_REAL* s, /* in */ uint32_t n, /* out */ ECO_SPLINE_REAL* coefs, /* in */ CEcoSplineTrace* pTrace) {
    ECO_SPLINE_REAL h = 0;
    ECO_SPLINE_REAL delta = 0;
    ECO_SPLINE_REAL c1 = 0;
    ECO_SPLINE_REAL c2 = 0;
    uint32_t k = 0;

    for (k = 0; k + 1 < n; k++) {
        h = x[k + 1] - x[k];
        delta = (y[k + 1] - y[k]) / h;
        c1 = (delta - s[k]) / h;
        c2 = (s[k + 1] - delta) / h;
        coefs[4 * k + 0] = (c2 - c1) / h;
        coefs[4 * k + 1] = 2 * c1 - c2;
        coefs[4 * k + 2] = s[k];
        coefs[4 * k + 3] = y[k];
        if (pTrace != 0) {
            EcoSplineNotifyPiece(pTrace, k, coefs[4 * k], coefs[4 * k + 1], coefs[4 * k + 2], coefs[4 * k + 3]);
        }
    }
}

#endif /* ECO_SPLINE_CORE_SOLVER */


/* ========================================================================== */
/*   Раздел EVAL: поиск куска и вычисление значений                           */
/* ========================================================================== */

#ifdef ECO_SPLINE_CORE_EVAL

/*
 * <summary>
 *   EcoSplineFindPiece - двоичный поиск куска
 * </summary>
 *
 * <description>
 *   Возвращает номер куска k (0..n-2), такого что breaks[k] <= xq < breaks[k+1].
 *   Точки левее breaks[0] относятся к куску 0, правее breaks[n-1] - к куску n-2
 *   (экстраполяция крайними кусками, как в ppval MATLAB). Сложность O(log n).
 * </description>
 */
static uint32_t ECO_SPLINE_FN(EcoSplineFindPiece)(/* in */ const ECO_SPLINE_REAL* breaks, /* in */ uint32_t n, /* in */ ECO_SPLINE_REAL xq) {
    uint32_t lo = 0;
    uint32_t hi = n - 1;
    uint32_t mid = 0;

    while (hi - lo > 1) {
        mid = lo + (hi - lo) / 2;
        if (xq >= breaks[mid]) {
            lo = mid;
        }
        else {
            hi = mid;
        }
    }
    return lo;
}

/*
 * <summary>
 *   EcoSplineEvaluate - вычисление сплайна в точках
 * </summary>
 *
 * <description>
 *   yq[j] = S(xq[j]), j = 0..m-1, по узлам breaks[0..n-1] и коэффициентам coefs
 *   (формат EcoSplineBuildCoefs). Многочлен куска вычисляется по схеме Горнера:
 *       S = ((a*t + b)*t + c)*t + d,   t = xq - breaks[k].
 * </description>
 */
static void ECO_SPLINE_FN(EcoSplineEvaluate)(/* in */ const ECO_SPLINE_REAL* breaks, /* in */ const ECO_SPLINE_REAL* coefs, /* in */ uint32_t n, /* in */ const ECO_SPLINE_REAL* xq, /* in */ uint32_t m, /* out */ ECO_SPLINE_REAL* yq) {
    const ECO_SPLINE_REAL* p = 0;
    ECO_SPLINE_REAL t = 0;
    uint32_t j = 0;
    uint32_t k = 0;

    for (j = 0; j < m; j++) {
        k = ECO_SPLINE_FN(EcoSplineFindPiece)(breaks, n, xq[j]);
        p = coefs + 4 * k;
        t = xq[j] - breaks[k];
        yq[j] = ((p[0] * t + p[1]) * t + p[2]) * t + p[3];
    }
}

#endif /* ECO_SPLINE_CORE_EVAL */


/* ========================================================================== */
/*   Раздел DRIVER: построение сплайна и вычисление в точках за один вызов     */
/* ========================================================================== */

#ifdef ECO_SPLINE_CORE_DRIVER

/*
 * <summary>
 *   EcoSplineInterpolate - yq = spline(x, y, xq)
 * </summary>
 *
 * <description>
 *   Полный цикл: проверка входных данных, решение СЛАУ для наклонов методом прогонки,
 *   построение коэффициентов и вычисление сплайна в m точках xq.
 *   Временная память (5*n элементов: наклоны s и рабочий массив 4*n, который сначала
 *   хранит диагонали прогонки, а затем коэффициенты кусков) выделяется одним блоком
 *   через IEcoMemoryAllocator1 и освобождается до возврата.
 *   Возвращает ERR_ECO_SUCCESS, ERR_ECO_POINTER, ERR_ECO_OUTOFMEMORY или ERR_ECOADVANCEDMATH_*.
 * </description>
 */
static int16_t ECO_SPLINE_FN(EcoSplineInterpolate)(/* in */ IEcoMemoryAllocator1* pIMem, /* in */ const ECO_SPLINE_REAL* x, /* in */ const ECO_SPLINE_REAL* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const ECO_SPLINE_REAL* xq, /* in */ uint32_t m, /* out */ ECO_SPLINE_REAL* yq, /* in */ CEcoSplineEventSource* pSource) {
    ECO_SPLINE_REAL* pBlock = 0;
    ECO_SPLINE_REAL* s = 0;
    ECO_SPLINE_REAL* pWork = 0;
    CEcoSplineTrace trace;
    CEcoSplineTrace* pTrace = 0;
    int16_t result = ERR_ECO_SUCCESS;

    /* Проверка указателей (xq и yq не нужны, если точек вычисления нет) */
    if (pIMem == 0 || x == 0 || y == 0) {
        return ERR_ECO_POINTER;
    }
    if (m > 0 && (xq == 0 || yq == 0)) {
        return ERR_ECO_POINTER;
    }

    /* Проверка входных данных */
    result = ECO_SPLINE_FN(EcoSplineCheckData)(x, y, n);
    if (result != ERR_ECO_SUCCESS) {
        return result;
    }
    result = EcoSplineCheckEnds(pEnds);
    if (result != ERR_ECO_SUCCESS) {
        return result;
    }
    if (m == 0) {
        return ERR_ECO_SUCCESS;
    }

    /* Размер блока 5*n элементов должен помещаться в uint32_t (аргумент Alloc) */
    if (n > (uint32_t)(0xFFFFFFFFUL / (5UL * sizeof(ECO_SPLINE_REAL)))) {
        return ERR_ECO_OUTOFMEMORY;
    }
    result = EcoSplineBeginTrace(pSource, &trace);
    if (result != ERR_ECO_SUCCESS) {
        return result;
    }
    if (trace.pEnum != 0) {
        pTrace = &trace;
    }
    pBlock = (ECO_SPLINE_REAL*)pIMem->pVTbl->Alloc(pIMem, (uint32_t)(5UL * n * sizeof(ECO_SPLINE_REAL)));
    if (pBlock == 0) {
        EcoSplineEndTrace(&trace);
        return ERR_ECO_OUTOFMEMORY;
    }
    s = pBlock;
    pWork = pBlock + n;

    result = ECO_SPLINE_FN(EcoSplineSolveSlopes)(x, y, n, pEnds, s, pWork, pTrace);
    if (result == ERR_ECO_SUCCESS) {
        /* Диагонали прогонки больше не нужны: на их место записываются коэффициенты 4*(n-1) */
        ECO_SPLINE_FN(EcoSplineBuildCoefs)(x, y, s, n, pWork, pTrace);
        ECO_SPLINE_FN(EcoSplineEvaluate)(x, pWork, n, xq, m, yq);
    }

    pIMem->pVTbl->Free(pIMem, pBlock);
    EcoSplineEndTrace(&trace);
    return result;
}

#endif /* ECO_SPLINE_CORE_DRIVER */
