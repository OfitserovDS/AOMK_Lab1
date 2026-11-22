/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   EcoAdvancedMath
 * </summary>
 *
 * <description>
 *   Юнит-тест компонента EcoAdvancedMath (консольное приложение, точка входа EcoMain).
 *   Проверяет кубическую сплайн-интерполяцию:
 *     1) пример из документации MATLAB (сравнение с эталоном MATLAB/SciPy и с sin из math.h);
 *     2) все типы граничных условий и их сочетания;
 *     3) вырожденные случаи n = 2, 3, 4;
 *     4) точное воспроизведение кубического многочлена и точность перегрузок float/double/long double;
 *     5) гладкость: непрерывность S, S', S'' в узлах и условие not-a-knot;
 *     6) объект IEcoSpline: узлы, коэффициенты, производные, интегралы, подсчет ссылок;
 *     7) порядок сходимости при сгущении сетки (сравнение с exp из math.h);
 *     8) обработку ошибочных входных данных;
 *     9) метод прогонки против метода Гаусса и замеры производительности.
 *   Клиенту разрешено использовать стандартную библиотеку C (вывод, время, эталонные функции
 *   math.h); сам компонент не содержит ни одного обращения к libc.
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */


/* Eco OS */
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoFileSystemManagement1.h"
#include "IdEcoAdvancedMath.h"
#include "ErrEcoAdvancedMath.h"

/* Эталонные значения (SciPy CubicSpline = алгоритм spline MATLAB) */
#include "EcoAdvancedMathReference.h"

/* Стандартная библиотека C - только в тестовом клиенте */
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>


/* ========================================================================== */
/*   Вспомогательные функции теста                                            */
/* ========================================================================== */

/* Счетчики проверок */
static uint32_t g_nChecks = 0;
static uint32_t g_nFailed = 0;

/* Нуль, известный только во время выполнения: для получения NaN и бесконечности */
static double_t g_dZero = 0.0;

/* Минимальное время одного замера производительности, с */
#define BENCH_MIN_SECONDS 0.25

/*
 * <summary>
 *   Check
 * </summary>
 *
 * <description>
 *   Регистрирует результат проверки; при неудаче печатает сообщение.
 * </description>
 */
static void Check(int bOk, const char_t* pszMessage) {
    g_nChecks++;
    if (!bOk) {
        g_nFailed++;
        printf("    [ОШИБКА] %s\n", pszMessage);
    }
}

/*
 * <summary>
 *   PrintPadded
 * </summary>
 *
 * <description>
 *   Печатает строку и дополняет ее пробелами до width символов. printf("%-Ns") считает
 *   байты, а символ кириллицы в UTF-8 занимает 2 байта, поэтому ширина считается по
 *   символам (байты продолжения UTF-8 вида 10xxxxxx не учитываются).
 * </description>
 */
static void PrintPadded(const char_t* psz, int width) {
    const char_t* p = psz;
    int chars = 0;

    while (*p != 0) {
        if (((unsigned char)*p & 0xC0) != 0x80) {
            chars++;
        }
        p++;
    }
    printf("%s", psz);
    while (chars < width) {
        putchar(' ');
        chars++;
    }
}

/*
 * <summary>
 *   PrintHeader
 * </summary>
 *
 * <description>
 *   Печатает строку заголовка таблицы: ячейки через два пробела, выравнивание вправо
 *   по ширине в символах (с учетом UTF-8). Список ячеек завершается нулевым указателем.
 * </description>
 */
static void PrintHeader(const char_t* const* cells, const int* widths) {
    const char_t* p = 0;
    int chars = 0;
    int i = 0;

    printf("    ");
    for (i = 0; cells[i] != 0; i++) {
        chars = 0;
        for (p = cells[i]; *p != 0; p++) {
            if (((unsigned char)*p & 0xC0) != 0x80) {
                chars++;
            }
        }
        while (chars < widths[i]) {
            putchar(' ');
            chars++;
        }
        printf("%s%s", cells[i], cells[i + 1] != 0 ? "  " : "\n");
    }
}

/*
 * <summary>
 *   CheckCode
 * </summary>
 *
 * <description>
 *   Проверяет код возврата метода и печатает строку протокола.
 * </description>
 */
static void CheckCode(int16_t actual, int16_t expected, const char_t* pszMessage) {
    g_nChecks++;
    if (actual != expected) {
        g_nFailed++;
        printf("    [ОШИБКА] ");
        PrintPadded(pszMessage, 62);
        printf(" код %d, ожидался %d\n", (int)actual, (int)expected);
    }
    else {
        printf("    [OK] ");
        PrintPadded(pszMessage, 66);
        printf(" код %d\n", (int)actual);
    }
}

/*
 * <summary>
 *   MaxAbsDiff
 * </summary>
 *
 * <description>
 *   Максимум модуля разности двух массивов.
 * </description>
 */
static double_t MaxAbsDiff(const double_t* a, const double_t* b, uint32_t m) {
    double_t maxDiff = 0;
    double_t diff = 0;
    uint32_t j = 0;

    for (j = 0; j < m; j++) {
        diff = fabs(a[j] - b[j]);
        if (diff > maxDiff) {
            maxDiff = diff;
        }
    }
    return maxDiff;
}

/*
 * <summary>
 *   DecimalDigits
 * </summary>
 *
 * <description>
 *   Число совпадающих знаков после запятой при абсолютной погрешности err
 *   (floor(-log10(err)), не более 17).
 * </description>
 */
static int DecimalDigits(double_t err) {
    double_t digits = 0;

    if (err <= 0) {
        return 17;
    }
    digits = floor(-log10(err));
    if (digits < 0) {
        return 0;
    }
    if (digits > 17) {
        return 17;
    }
    return (int)digits;
}

/*
 * <summary>
 *   AbsL
 * </summary>
 *
 * <description>
 *   Модуль long double (fabsl относится к C99 и в режиме C89 недоступна).
 * </description>
 */
static ldouble_t AbsL(ldouble_t v) {
    return v < 0 ? -v : v;
}

/*
 * <summary>
 *   PrintSection
 * </summary>
 *
 * <description>
 *   Заголовок раздела протокола.
 * </description>
 */
static void PrintSection(const char_t* pszTitle) {
    printf("\n================================================================================\n");
    printf("%s\n", pszTitle);
    printf("================================================================================\n");
}

/*
 * <summary>
 *   SetEnds
 * </summary>
 *
 * <description>
 *   Заполнение структуры граничных условий.
 * </description>
 */
static void SetEnds(ECO_SPLINE_ENDS_T* pEnds, uint16_t leftType, double_t leftValue, uint16_t rightType, double_t rightValue) {
    pEnds->leftType = leftType;
    pEnds->rightType = rightType;
    pEnds->leftValue = leftValue;
    pEnds->rightValue = rightValue;
}

/*
 * <summary>
 *   EvalOne
 * </summary>
 *
 * <description>
 *   Значение производной порядка order объекта-сплайна в одной точке.
 * </description>
 */
static double_t EvalOne(IEcoSpline* pISpline, uint32_t order, double_t x) {
    double_t y = 0;

    pISpline->pVTbl->EvaluateDerivative(pISpline, order, &x, 1, &y);
    return y;
}

/*
 * <summary>
 *   PieceDerivative
 * </summary>
 *
 * <description>
 *   Производная порядка order (0..3) многочлена куска p = {a, b, c, d} в точке t.
 * </description>
 */
static double_t PieceDerivative(const double_t* p, uint32_t order, double_t t) {
    switch (order) {
    case 0:
        return ((p[0] * t + p[1]) * t + p[2]) * t + p[3];
    case 1:
        return (3 * p[0] * t + 2 * p[1]) * t + p[2];
    case 2:
        return 6 * p[0] * t + 2 * p[1];
    default:
        return 6 * p[0];
    }
}


/* ========================================================================== */
/*   Тест 1. Пример из документации MATLAB                                    */
/* ========================================================================== */

static void TestMatlabExample(IEcoAdvancedMath* pIMath) {
    double_t yq[REF_A_M];
    double_t err = 0;
    double_t errSin = 0;
    double_t maxErr = 0;
    double_t maxErrSin = 0;
    uint32_t j = 0;
    int16_t result = 0;

    PrintSection("Тест 1. Пример из документации MATLAB: x = 0:10; y = sin(x); yy = spline(x, y, 0:0.25:10)");

    result = pIMath->pVTbl->spline(pIMath, g_RefA_X, g_RefA_Y, REF_A_N, 0, g_RefA_XQ, REF_A_M, yq);
    CheckCode(result, ERR_ECO_SUCCESS, "spline(x, y, xq), граничное условие not-a-knot");

    printf("\n    %5s  %20s  %20s  %20s  %10s  %10s\n", "xq", "sin(xq) [math.h]", "MATLAB/SciPy spline", "EcoAdvancedMath", "|Eco-ref|", "|Eco-sin|");
    for (j = 0; j < REF_A_M; j++) {
        err = fabs(yq[j] - g_RefA_YQ[j]);
        errSin = fabs(yq[j] - sin(g_RefA_XQ[j]));
        if (err > maxErr) {
            maxErr = err;
        }
        if (errSin > maxErrSin) {
            maxErrSin = errSin;
        }
        if (j % 2 == 0) {
            printf("    %5.2f  %20.17f  %20.17f  %20.17f  %10.2e  %10.2e\n", g_RefA_XQ[j], sin(g_RefA_XQ[j]), g_RefA_YQ[j], yq[j], err, errSin);
        }
    }
    printf("\n    max |Eco - MATLAB/SciPy| = %.3e  (совпадает %d знаков после запятой)\n", maxErr, DecimalDigits(maxErr));
    printf("    max |Eco - sin(x)|       = %.3e  (погрешность самой интерполяции, шаг h = 1)\n", maxErrSin);
    Check(maxErr <= 1e-14, "Тест 1: расхождение с эталоном MATLAB/SciPy больше 1e-14");
    Check(maxErrSin <= 0.03, "Тест 1: погрешность интерполяции sin(x) слишком велика");
}


/* ========================================================================== */
/*   Тест 2. Граничные условия                                                */
/* ========================================================================== */

/*
 * <summary>
 *   EndResidual
 * </summary>
 *
 * <description>
 *   Невязка граничного условия на конце (bLeft - левый): для not-a-knot - скачок S'''
 *   во втором (предпоследнем) узле, для остальных - отклонение S' или S'' от заданного.
 * </description>
 */
static double_t EndResidual(IEcoSpline* pISpline, const double_t* x, uint32_t n, uint16_t type, double_t value, int bLeft) {
    double_t xe = bLeft ? x[0] : x[n - 1];

    switch (type) {
    case ECO_SPLINE_END_NOT_A_KNOT:
        if (bLeft) {
            return fabs(EvalOne(pISpline, 3, x[0]) - EvalOne(pISpline, 3, x[1]));
        }
        return fabs(EvalOne(pISpline, 3, x[n - 3]) - EvalOne(pISpline, 3, x[n - 2]));
    case ECO_SPLINE_END_NATURAL:
        return fabs(EvalOne(pISpline, 2, xe));
    case ECO_SPLINE_END_FIRST_DERIVATIVE:
        return fabs(EvalOne(pISpline, 1, xe) - value);
    default:
        return fabs(EvalOne(pISpline, 2, xe) - value);
    }
}

static void TestEndConditions(IEcoAdvancedMath* pIMath) {
    static const char_t* names[4] = {"not-a-knot", "natural", "S' задана", "S'' задана"};
    ECO_SPLINE_ENDS_T ends;
    IEcoSpline* pISpline = 0;
    double_t yq[REF_B_M];
    double_t err = 0;
    double_t resLeft = 0;
    double_t resRight = 0;
    uint32_t c = 0;
    int16_t result = 0;

    PrintSection("Тест 2. Граничные условия: неравномерные узлы, y = exp(-x/2)*cos(2x)");
    printf("\n    ");
    PrintPadded("случай", 8);
    printf("  ");
    PrintPadded("левый конец", 12);
    printf("  ");
    PrintPadded("правый конец", 12);
    printf("  %12s  %6s  %12s  %12s\n", "|Eco-SciPy|", "знаков", "невязка лев.", "невязка пр.");

    for (c = 0; c < REF_B_NCASES; c++) {
        SetEnds(&ends, g_RefB_LeftType[c], g_RefB_LeftValue[c], g_RefB_RightType[c], g_RefB_RightValue[c]);

        result = pIMath->pVTbl->spline(pIMath, g_RefB_X, g_RefB_Y, REF_B_N, &ends, g_RefB_XQ, REF_B_M, yq);
        Check(result == ERR_ECO_SUCCESS, "Тест 2: spline вернул ошибку");
        err = MaxAbsDiff(yq, g_RefB_YQ + c * REF_B_M, REF_B_M);

        result = pIMath->pVTbl->new_Spline(pIMath, g_RefB_X, g_RefB_Y, REF_B_N, &ends, &pISpline);
        Check(result == ERR_ECO_SUCCESS && pISpline != 0, "Тест 2: new_Spline вернул ошибку");
        if (pISpline == 0) {
            continue;
        }
        resLeft = EndResidual(pISpline, g_RefB_X, REF_B_N, ends.leftType, g_RefB_LeftValue[c], 1);
        resRight = EndResidual(pISpline, g_RefB_X, REF_B_N, ends.rightType, g_RefB_RightValue[c], 0);
        pISpline->pVTbl->Release(pISpline);
        pISpline = 0;

        printf("    ");
        PrintPadded(g_RefB_Name[c], 8);
        printf("  ");
        PrintPadded(names[ends.leftType], 12);
        printf("  ");
        PrintPadded(names[ends.rightType], 12);
        printf("  %12.3e  %6d  %12.3e  %12.3e\n", err, DecimalDigits(err), resLeft, resRight);
        Check(err <= 1e-13, "Тест 2: расхождение с эталоном SciPy больше 1e-13");
        Check(resLeft <= 1e-11 && resRight <= 1e-11, "Тест 2: граничное условие не выполнено");
    }
}


/* ========================================================================== */
/*   Тест 3. Вырожденные случаи малого числа узлов                            */
/* ========================================================================== */

static void TestDegenerateCases(IEcoAdvancedMath* pIMath) {
    ECO_SPLINE_ENDS_T ends;
    double_t yq[REF_C_M];
    double_t err = 0;
    uint32_t c = 0;
    int16_t result = 0;

    PrintSection("Тест 3. Вырожденные случаи n = 2, 3, 4 (как в MATLAB/SciPy)");
    printf("\n    ");
    PrintPadded("случай", 30);
    printf("  %12s  %6s\n", "|Eco-SciPy|", "знаков");

    for (c = 0; c < REF_C_NCASES; c++) {
        SetEnds(&ends, g_RefC_LeftType[c], g_RefC_LeftValue[c], g_RefC_RightType[c], g_RefC_RightValue[c]);
        result = pIMath->pVTbl->spline(pIMath, g_RefC_X + 4 * c, g_RefC_Y + 4 * c, g_RefC_N[c], &ends, g_RefC_XQ, REF_C_M, yq);
        Check(result == ERR_ECO_SUCCESS, "Тест 3: spline вернул ошибку");
        err = MaxAbsDiff(yq, g_RefC_YQ + c * REF_C_M, REF_C_M);
        printf("    %-30s  %12.3e  %6d\n", g_RefC_Name[c], err, DecimalDigits(err));
        Check(err <= 1e-13, "Тест 3: расхождение с эталоном SciPy больше 1e-13");
    }
}


/* ========================================================================== */
/*   Тест 4. Воспроизведение многочленов и точность перегрузок                */
/* ========================================================================== */

/* p(x) = 2x^3 - 3x^2 + x - 5 и его производные (вычисление в long double) */
static ldouble_t PolyL(ldouble_t x)   { return ((2 * x - 3) * x + 1) * x - 5; }
static ldouble_t PolyD1L(ldouble_t x) { return (6 * x - 6) * x + 1; }
static ldouble_t PolyD2L(ldouble_t x) { return 12 * x - 6; }

#define POLY_N 8
#define POLY_M 25

static void TestPolynomialPrecision(IEcoAdvancedMath* pIMath) {
    /* Неравномерные узлы - двоичные дроби, точно представимые во всех трех типах */
    static const double_t knots[POLY_N] = {-2.0, -1.25, -0.875, 0.0, 0.5, 1.5, 2.25, 3.0};
    float_t xf[POLY_N], yf[POLY_N], xqf[POLY_M], yqf[POLY_M];
    double_t xd[POLY_N], yd[POLY_N], xqd[POLY_M], yqd[POLY_M];
    ldouble_t xl[POLY_N], yl[POLY_N], xql[POLY_M], yql[POLY_M];
    ldouble_t exact = 0;
    ldouble_t scale = 0;
    ldouble_t errF = 0, errD = 0, errL = 0;
    ldouble_t errFD = 0, errDL = 0;
    ldouble_t errClamped = 0, errSecond = 0, errNatural = 0, errNaturalLine = 0;
    ECO_SPLINE_ENDS_T ends;
    uint32_t i = 0;
    uint32_t j = 0;
    int16_t result = 0;

    PrintSection("Тест 4. Воспроизведение многочлена p(x) = 2x^3 - 3x^2 + x - 5 и точность перегрузок");

    for (i = 0; i < POLY_N; i++) {
        xl[i] = knots[i];
        yl[i] = PolyL(xl[i]);
        xd[i] = (double_t)xl[i];
        yd[i] = (double_t)yl[i];
        xf[i] = (float_t)xl[i];
        yf[i] = (float_t)yl[i];
    }
    for (j = 0; j < POLY_M; j++) {
        xql[j] = -2.5L + 0.25L * (ldouble_t)j;    /* включая экстраполяцию за [-2, 3] */
        xqd[j] = (double_t)xql[j];
        xqf[j] = (float_t)xql[j];
        if (AbsL(PolyL(xql[j])) > scale) {
            scale = AbsL(PolyL(xql[j]));
        }
    }

    /* Условие not-a-knot: кубический многочлен воспроизводится точно во всех перегрузках */
    result = pIMath->pVTbl->splinef(pIMath, xf, yf, POLY_N, 0, xqf, POLY_M, yqf);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: splinef вернул ошибку");
    result = pIMath->pVTbl->spline(pIMath, xd, yd, POLY_N, 0, xqd, POLY_M, yqd);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: spline вернул ошибку");
    result = pIMath->pVTbl->splinel(pIMath, xl, yl, POLY_N, 0, xql, POLY_M, yql);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: splinel вернул ошибку");
    for (j = 0; j < POLY_M; j++) {
        exact = PolyL(xql[j]);
        if (AbsL((ldouble_t)yqf[j] - exact) > errF) errF = AbsL((ldouble_t)yqf[j] - exact);
        if (AbsL((ldouble_t)yqd[j] - exact) > errD) errD = AbsL((ldouble_t)yqd[j] - exact);
        if (AbsL(yql[j] - exact) > errL) errL = AbsL(yql[j] - exact);
        if (AbsL((ldouble_t)yqf[j] - (ldouble_t)yqd[j]) > errFD) errFD = AbsL((ldouble_t)yqf[j] - (ldouble_t)yqd[j]);
        if (AbsL((ldouble_t)yqd[j] - yql[j]) > errDL) errDL = AbsL((ldouble_t)yqd[j] - yql[j]);
    }

    printf("\n    Перегрузка   тип            байт   машинный эпсилон   max |S - p|   отн. погрешность   в эпсилонах\n");
    printf("    splinef      float_t        %4d   %16.3e   %11.3e   %16.3e   %11.1f\n", (int)sizeof(float_t), (double)FLT_EPSILON, (double)errF, (double)(errF / scale), (double)(errF / scale / FLT_EPSILON));
    printf("    spline       double_t       %4d   %16.3e   %11.3e   %16.3e   %11.1f\n", (int)sizeof(double_t), (double)DBL_EPSILON, (double)errD, (double)(errD / scale), (double)(errD / scale / DBL_EPSILON));
    printf("    splinel      ldouble_t      %4d   %16.3Le   %11.3Le   %16.3Le   %11.1Lf\n", (int)sizeof(ldouble_t), (ldouble_t)LDBL_EPSILON, errL, errL / scale, errL / scale / LDBL_EPSILON);
    Check(errF / scale <= 100 * FLT_EPSILON, "Тест 4: float - многочлен не воспроизведен с точностью 100 eps");
    Check(errD / scale <= 100 * DBL_EPSILON, "Тест 4: double - многочлен не воспроизведен с точностью 100 eps");
    Check(errL / scale <= 100 * LDBL_EPSILON, "Тест 4: long double - многочлен не воспроизведен с точностью 100 eps");
    /* Перегрузки согласованы: float и double расходятся не более чем на погрешность float,
       double и long double - не более чем на погрешность double. Строгий рост точности
       от float к long double не требуется: на точно представимых данных float-версия может
       получить точный результат (обнаружено при проверке системного промта, прогон r2) */
    Check(errFD / scale <= 200 * FLT_EPSILON && errDL / scale <= 200 * DBL_EPSILON, "Тест 4: результаты перегрузок не согласованы между собой");

    /* Заданные точные S' на концах: кубический многочлен тоже воспроизводится точно */
    SetEnds(&ends, ECO_SPLINE_END_FIRST_DERIVATIVE, 0, ECO_SPLINE_END_FIRST_DERIVATIVE, 0);
    ends.leftValue = PolyD1L(xl[0]);
    ends.rightValue = PolyD1L(xl[POLY_N - 1]);
    result = pIMath->pVTbl->splinel(pIMath, xl, yl, POLY_N, &ends, xql, POLY_M, yql);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: splinel с заданными S' вернул ошибку");
    for (j = 0; j < POLY_M; j++) {
        if (AbsL(yql[j] - PolyL(xql[j])) > errClamped) errClamped = AbsL(yql[j] - PolyL(xql[j]));
    }
    /* Заданные точные S'' на концах */
    SetEnds(&ends, ECO_SPLINE_END_SECOND_DERIVATIVE, 0, ECO_SPLINE_END_SECOND_DERIVATIVE, 0);
    ends.leftValue = PolyD2L(xl[0]);
    ends.rightValue = PolyD2L(xl[POLY_N - 1]);
    result = pIMath->pVTbl->splinel(pIMath, xl, yl, POLY_N, &ends, xql, POLY_M, yql);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: splinel с заданными S'' вернул ошибку");
    for (j = 0; j < POLY_M; j++) {
        if (AbsL(yql[j] - PolyL(xql[j])) > errSecond) errSecond = AbsL(yql[j] - PolyL(xql[j]));
    }
    /* Естественный сплайн не воспроизводит кубику (p'' на концах не равна 0) ... */
    SetEnds(&ends, ECO_SPLINE_END_NATURAL, 0, ECO_SPLINE_END_NATURAL, 0);
    result = pIMath->pVTbl->splinel(pIMath, xl, yl, POLY_N, &ends, xql, POLY_M, yql);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: splinel для естественного сплайна (кубика) вернул ошибку");
    for (j = 0; j < POLY_M; j++) {
        if (AbsL(yql[j] - PolyL(xql[j])) > errNatural) errNatural = AbsL(yql[j] - PolyL(xql[j]));
    }
    /* ... но воспроизводит линейную функцию l(x) = 3x - 1 */
    for (i = 0; i < POLY_N; i++) {
        yl[i] = 3 * xl[i] - 1;
    }
    result = pIMath->pVTbl->splinel(pIMath, xl, yl, POLY_N, &ends, xql, POLY_M, yql);
    Check(result == ERR_ECO_SUCCESS, "Тест 4: splinel для естественного сплайна (прямая) вернул ошибку");
    for (j = 0; j < POLY_M; j++) {
        if (AbsL(yql[j] - (3 * xql[j] - 1)) > errNaturalLine) errNaturalLine = AbsL(yql[j] - (3 * xql[j] - 1));
    }

    printf("\n    Граничные условия (long double)        max |S - f|\n");
    printf("    S' = p' на концах,  f = p (кубика)     %11.3Le\n", errClamped);
    printf("    S'' = p'' на концах, f = p (кубика)    %11.3Le\n", errSecond);
    printf("    естественный, f = p (кубика)           %11.3Le   (не обязан воспроизводить: p''(-2) = -30, p''(3) = 30)\n", errNatural);
    printf("    естественный, f = 3x - 1 (прямая)      %11.3Le\n", errNaturalLine);
    Check(errClamped / scale <= 100 * LDBL_EPSILON, "Тест 4: сплайн с заданными S' не воспроизвел кубику");
    Check(errSecond / scale <= 100 * LDBL_EPSILON, "Тест 4: сплайн с заданными S'' не воспроизвел кубику");
    Check(errNatural > 1e-3, "Тест 4: естественный сплайн неожиданно воспроизвел кубику");
    Check(errNaturalLine <= 100 * LDBL_EPSILON * 10, "Тест 4: естественный сплайн не воспроизвел прямую");
}


/* ========================================================================== */
/*   Тест 5. Гладкость сплайна                                                */
/* ========================================================================== */

static void TestSmoothness(IEcoAdvancedMath* pIMath) {
    static const char_t* titles[3] = {"not-a-knot", "естественный", "S' задана"};
    ECO_SPLINE_ENDS_T ends;
    IEcoSpline* pISpline = 0;
    double_t breaks[REF_B_N];
    double_t coefs[4 * (REF_B_N - 1)];
    double_t jump[4];
    double_t h = 0;
    double_t value = 0;
    uint32_t k = 0;
    uint32_t order = 0;
    uint32_t variant = 0;
    int16_t result = 0;

    PrintSection("Тест 5. Гладкость: скачки S, S', S'', S''' во внутренних узлах (по коэффициентам кусков)");
    printf("\n    ");
    PrintPadded("условие", 14);
    printf("  %12s  %12s  %12s  ", "max|[S]|", "max|[S']|", "max|[S'']|");
    PrintPadded("|[S''']| в x[1], x[n-2]", 24);
    printf("  ");
    PrintPadded("max|[S''']| прочие", 18);
    printf("\n");

    for (variant = 0; variant < 3; variant++) {
        SetEnds(&ends, (uint16_t)(variant == 0 ? ECO_SPLINE_END_NOT_A_KNOT : (variant == 1 ? ECO_SPLINE_END_NATURAL : ECO_SPLINE_END_FIRST_DERIVATIVE)), 0.5,
                       (uint16_t)(variant == 0 ? ECO_SPLINE_END_NOT_A_KNOT : (variant == 1 ? ECO_SPLINE_END_NATURAL : ECO_SPLINE_END_FIRST_DERIVATIVE)), -0.25);
        result = pIMath->pVTbl->new_Spline(pIMath, g_RefB_X, g_RefB_Y, REF_B_N, &ends, &pISpline);
        Check(result == ERR_ECO_SUCCESS && pISpline != 0, "Тест 5: new_Spline вернул ошибку");
        if (pISpline == 0) {
            continue;
        }
        pISpline->pVTbl->get_Breaks(pISpline, breaks, REF_B_N);
        pISpline->pVTbl->get_Coefs(pISpline, coefs, 4 * (REF_B_N - 1));

        /* Скачок производной порядка order в узле k = (правый кусок при t = 0) - (левый кусок при t = h) */
        jump[0] = jump[1] = jump[2] = jump[3] = 0;
        for (k = 1; k + 1 < REF_B_N; k++) {
            h = breaks[k] - breaks[k - 1];
            for (order = 0; order < 3; order++) {
                value = fabs(PieceDerivative(coefs + 4 * k, order, 0) - PieceDerivative(coefs + 4 * (k - 1), order, h));
                if (value > jump[order]) {
                    jump[order] = value;
                }
            }
            if (k != 1 && k != REF_B_N - 2) {
                value = fabs(PieceDerivative(coefs + 4 * k, 3, 0) - PieceDerivative(coefs + 4 * (k - 1), 3, h));
                if (value > jump[3]) {
                    jump[3] = value;
                }
            }
        }
        value = fabs(PieceDerivative(coefs + 4, 3, 0) - PieceDerivative(coefs, 3, 0));
        h = fabs(PieceDerivative(coefs + 4 * (REF_B_N - 2), 3, 0) - PieceDerivative(coefs + 4 * (REF_B_N - 3), 3, 0));
        if (h > value) {
            value = h;
        }
        printf("    ");
        PrintPadded(titles[variant], 14);
        printf("  %12.3e  %12.3e  %12.3e  %24.3e  %18.3e\n", jump[0], jump[1], jump[2], value, jump[3]);
        Check(jump[0] <= 1e-13 && jump[1] <= 1e-12 && jump[2] <= 1e-11, "Тест 5: сплайн не является дважды непрерывно дифференцируемым");
        if (variant == 0) {
            Check(value <= 1e-10, "Тест 5: условие not-a-knot (непрерывность S''') не выполнено");
        }
        Check(jump[3] > 1e-3, "Тест 5: S''' неожиданно непрерывна во всех узлах");

        pISpline->pVTbl->Release(pISpline);
        pISpline = 0;
    }
}


/* ========================================================================== */
/*   Тест 6. Объект данных IEcoSpline                                         */
/* ========================================================================== */

static void TestSplineObject(IEcoAdvancedMath* pIMath) {
    IEcoSpline* pISpline = 0;
    IEcoSpline* pISpline2 = 0;
    IEcoUnknown* pIUnk = 0;
    IEcoAdvancedMath* pIWrong = 0;
    double_t breaks[REF_A_N];
    double_t coefs[4 * (REF_A_N - 1)];
    double_t yq[REF_A_M];
    double_t yq2[REF_A_M];
    double_t value = 0;
    double_t errBreaks = 0, errCoefs = 0, errVal = 0, errSame = 0, errD1 = 0, errD2 = 0, errD3 = 0, errD4 = 0, errInt = 0;
    uint32_t refCount = 0;
    uint32_t j = 0;
    int16_t result = 0;

    PrintSection("Тест 6. Объект данных IEcoSpline (pp-форма): pp = spline(x, y) для примера MATLAB");

    result = pIMath->pVTbl->new_Spline(pIMath, g_RefA_X, g_RefA_Y, REF_A_N, 0, &pISpline);
    CheckCode(result, ERR_ECO_SUCCESS, "new_Spline(x, y) -> IEcoSpline");
    if (pISpline == 0) {
        return;
    }

    /* Структура pp-формы */
    Check(pISpline->pVTbl->get_Pieces(pISpline) == REF_A_N - 1, "Тест 6: get_Pieces вернул неверное число кусков");
    result = pISpline->pVTbl->get_Breaks(pISpline, breaks, REF_A_N);
    Check(result == ERR_ECO_SUCCESS, "Тест 6: get_Breaks вернул ошибку");
    result = pISpline->pVTbl->get_Coefs(pISpline, coefs, 4 * (REF_A_N - 1));
    Check(result == ERR_ECO_SUCCESS, "Тест 6: get_Coefs вернул ошибку");
    errBreaks = MaxAbsDiff(breaks, g_RefA_X, REF_A_N);
    errCoefs = MaxAbsDiff(coefs, g_RefA_Coefs, 4 * (REF_A_N - 1));

    /* Значения и производные */
    pISpline->pVTbl->Evaluate(pISpline, g_RefA_XQ, REF_A_M, yq);
    errVal = MaxAbsDiff(yq, g_RefA_YQ, REF_A_M);
    pIMath->pVTbl->spline(pIMath, g_RefA_X, g_RefA_Y, REF_A_N, 0, g_RefA_XQ, REF_A_M, yq2);
    errSame = MaxAbsDiff(yq, yq2, REF_A_M);
    pISpline->pVTbl->EvaluateDerivative(pISpline, 1, g_RefA_XQ, REF_A_M, yq);
    errD1 = MaxAbsDiff(yq, g_RefA_D1, REF_A_M);
    pISpline->pVTbl->EvaluateDerivative(pISpline, 2, g_RefA_XQ, REF_A_M, yq);
    errD2 = MaxAbsDiff(yq, g_RefA_D2, REF_A_M);
    pISpline->pVTbl->EvaluateDerivative(pISpline, 3, g_RefA_XQ, REF_A_M, yq);
    errD3 = MaxAbsDiff(yq, g_RefA_D3, REF_A_M);
    pISpline->pVTbl->EvaluateDerivative(pISpline, 4, g_RefA_XQ, REF_A_M, yq);
    for (j = 0; j < REF_A_M; j++) {
        if (fabs(yq[j]) > errD4) {
            errD4 = fabs(yq[j]);
        }
    }

    printf("\n    Величина                                   max |Eco - SciPy|   знаков\n");
    printf("    узлы (get_Breaks)                          %17.3e   %6d\n", errBreaks, DecimalDigits(errBreaks));
    printf("    коэффициенты кусков (get_Coefs)            %17.3e   %6d\n", errCoefs, DecimalDigits(errCoefs));
    printf("    значения S (Evaluate, аналог ppval)        %17.3e   %6d\n", errVal, DecimalDigits(errVal));
    printf("    S'  (EvaluateDerivative, order = 1)        %17.3e   %6d\n", errD1, DecimalDigits(errD1));
    printf("    S'' (EvaluateDerivative, order = 2)        %17.3e   %6d\n", errD2, DecimalDigits(errD2));
    printf("    S'''(EvaluateDerivative, order = 3)        %17.3e   %6d\n", errD3, DecimalDigits(errD3));
    printf("    S^(4) должна быть 0                        %17.3e\n", errD4);
    printf("    Evaluate объекта и сервис spline           %17.3e   (одинаковый алгоритм)\n", errSame);
    Check(errBreaks == 0, "Тест 6: узлы не совпадают с исходными");
    Check(errCoefs <= 1e-14 && errVal <= 1e-14 && errD1 <= 1e-13 && errD2 <= 1e-13 && errD3 <= 1e-12, "Тест 6: расхождение с эталоном SciPy");
    Check(errD4 == 0 && errSame == 0, "Тест 6: неверная производная 4-го порядка или различие Evaluate и spline");

    /* Определенные интегралы */
    printf("\n    %9s  %9s  %22s  %22s  %10s\n", "a", "b", "SciPy integrate", "Eco Integrate", "|разность|");
    for (j = 0; j < REF_A_NINT; j++) {
        result = pISpline->pVTbl->Integrate(pISpline, g_RefA_IntA[j], g_RefA_IntB[j], &value);
        Check(result == ERR_ECO_SUCCESS, "Тест 6: Integrate вернул ошибку");
        printf("    %9.5f  %9.5f  %22.17f  %22.17f  %10.2e\n", g_RefA_IntA[j], g_RefA_IntB[j], g_RefA_IntV[j], value, fabs(value - g_RefA_IntV[j]));
        if (fabs(value - g_RefA_IntV[j]) > errInt) {
            errInt = fabs(value - g_RefA_IntV[j]);
        }
    }
    printf("    Интеграл sin(x) от 0 до pi: точно 2, по сплайну %.15f (погрешность интерполяции %.2e)\n", value, fabs(value - 2.0));
    Check(errInt <= 1e-13, "Тест 6: интеграл расходится с эталоном SciPy");

    /* Подсчет ссылок и QueryInterface */
    refCount = pISpline->pVTbl->AddRef(pISpline);
    Check(refCount == 2, "Тест 6: AddRef вернул неверный счетчик");
    result = pISpline->pVTbl->QueryInterface(pISpline, &IID_IEcoSpline, (voidptr_t*)&pISpline2);
    Check(result == ERR_ECO_SUCCESS && pISpline2 == pISpline, "Тест 6: QueryInterface(IID_IEcoSpline)");
    result = pISpline->pVTbl->QueryInterface(pISpline, &IID_IEcoUnknown, (voidptr_t*)&pIUnk);
    Check(result == ERR_ECO_SUCCESS && pIUnk != 0, "Тест 6: QueryInterface(IID_IEcoUnknown)");
    result = pISpline->pVTbl->QueryInterface(pISpline, &IID_IEcoAdvancedMath, (voidptr_t*)&pIWrong);
    Check(result == ERR_ECO_NOINTERFACE && pIWrong == 0, "Тест 6: QueryInterface чужого интерфейса должен вернуть ERR_ECO_NOINTERFACE");
    pIUnk->pVTbl->Release(pIUnk);
    pISpline2->pVTbl->Release(pISpline2);
    refCount = pISpline->pVTbl->Release(pISpline);
    Check(refCount == 1, "Тест 6: Release вернул неверный счетчик");
    refCount = pISpline->pVTbl->Release(pISpline);
    Check(refCount == 0, "Тест 6: последний Release должен вернуть 0");
    printf("\n    Подсчет ссылок: AddRef -> 2, QueryInterface(IEcoSpline/IEcoUnknown) - успешно, чужой IID - ERR_ECO_NOINTERFACE, Release -> 1 -> 0\n");
}


/* ========================================================================== */
/*   Тест 7. Порядок сходимости                                               */
/* ========================================================================== */

/*
 * <summary>
 *   InterpolationError
 * </summary>
 *
 * <description>
 *   Максимальная погрешность сплайна для f(x) = exp(x) на [0, 2] по n равномерным узлам,
 *   измеренная на сетке из 4001 точки (эталон - exp из math.h).
 * </description>
 */
static double_t InterpolationError(IEcoAdvancedMath* pIMath, IEcoMemoryAllocator1* pIMem, uint32_t n, const ECO_SPLINE_ENDS_T* pEnds) {
    const uint32_t m = 4001;
    double_t* x = 0;
    double_t* y = 0;
    double_t* xq = 0;
    double_t* yq = 0;
    double_t err = 0;
    uint32_t i = 0;

    x = (double_t*)pIMem->pVTbl->Alloc(pIMem, (2 * n + 2 * m) * (uint32_t)sizeof(double_t));
    if (x == 0) {
        return -1;
    }
    y = x + n;
    xq = y + n;
    yq = xq + m;
    for (i = 0; i < n; i++) {
        x[i] = 2.0 * (double_t)i / (double_t)(n - 1);
        y[i] = exp(x[i]);
    }
    for (i = 0; i < m; i++) {
        xq[i] = 2.0 * (double_t)i / (double_t)(m - 1);
    }
    pIMath->pVTbl->spline(pIMath, x, y, n, pEnds, xq, m, yq);
    for (i = 0; i < m; i++) {
        if (fabs(yq[i] - exp(xq[i])) > err) {
            err = fabs(yq[i] - exp(xq[i]));
        }
    }
    pIMem->pVTbl->Free(pIMem, x);
    return err;
}

static void TestConvergence(IEcoAdvancedMath* pIMath, IEcoMemoryAllocator1* pIMem) {
    ECO_SPLINE_ENDS_T endsNak, endsNat, endsCla;
    double_t err[3], prev[3];
    double_t ratio[3];
    uint32_t n = 0;
    uint32_t v = 0;

    PrintSection("Тест 7. Сходимость: f(x) = exp(x) на [0, 2], погрешность относительно exp из math.h");
    SetEnds(&endsNak, ECO_SPLINE_END_NOT_A_KNOT, 0, ECO_SPLINE_END_NOT_A_KNOT, 0);
    SetEnds(&endsNat, ECO_SPLINE_END_NATURAL, 0, ECO_SPLINE_END_NATURAL, 0);
    SetEnds(&endsCla, ECO_SPLINE_END_FIRST_DERIVATIVE, exp(0.0), ECO_SPLINE_END_FIRST_DERIVATIVE, exp(2.0));
    prev[0] = prev[1] = prev[2] = 0;
    ratio[0] = ratio[1] = ratio[2] = 0;

    {
        static const char_t* const cells[] = {"n", "h", "not-a-knot", "отн.", "S' = f'", "отн.", "естественный", "отн.", 0};
        static const int widths[] = {6, 9, 14, 6, 14, 6, 14, 6};
        printf("\n");
        PrintHeader(cells, widths);
    }
    for (n = 5; n <= 513; n = 2 * n - 1) {
        err[0] = InterpolationError(pIMath, pIMem, n, &endsNak);
        err[1] = InterpolationError(pIMath, pIMem, n, &endsCla);
        err[2] = InterpolationError(pIMath, pIMem, n, &endsNat);
        printf("    %6lu  %9.5f", (unsigned long)n, 2.0 / (double_t)(n - 1));
        for (v = 0; v < 3; v++) {
            ratio[v] = prev[v] > 0 ? prev[v] / err[v] : 0;
            if (prev[v] > 0) {
                printf("  %14.3e  %6.2f", err[v], ratio[v]);
            }
            else {
                printf("  %14.3e  %6s", err[v], "-");
            }
            prev[v] = err[v];
        }
        printf("\n");
    }
    printf("\n    При уменьшении шага вдвое погрешность уменьшается в 16 раз (O(h^4)) для not-a-knot и\n");
    printf("    заданных S', и примерно в 4 раза (O(h^2)) для естественного сплайна: условие S'' = 0\n");
    printf("    на концах неверно для exp(x).\n");
    Check(ratio[0] > 12 && ratio[0] < 20, "Тест 7: порядок сходимости not-a-knot не равен 4");
    Check(ratio[1] > 12 && ratio[1] < 20, "Тест 7: порядок сходимости сплайна с заданными S' не равен 4");
    Check(ratio[2] > 3 && ratio[2] < 5, "Тест 7: порядок сходимости естественного сплайна не равен 2");
}


/* ========================================================================== */
/*   Тест 8. Обработка ошибочных входных данных                              */
/* ========================================================================== */

static void TestErrors(IEcoAdvancedMath* pIMath) {
    double_t x[4] = {0.0, 1.0, 2.0, 3.0};
    double_t y[4] = {1.0, 2.0, 0.0, 1.0};
    double_t xBad[4] = {0.0, 1.0, 1.0, 3.0};
    double_t xDesc[4] = {3.0, 2.0, 1.0, 0.0};
    double_t yNan[4];
    double_t xInf[4];
    double_t xq[2] = {0.5, 1.5};
    double_t yq[2] = {0.0, 0.0};
    double_t buffer[2];
    double_t value = 0;
    float_t xf[1] = {1.0f};
    float_t yf[1] = {1.0f};
    float_t qf[1] = {1.0f};
    float_t rf[1] = {0.0f};
    ldouble_t xl[2] = {0.0L, 0.0L};
    ldouble_t yl[2] = {1.0L, 2.0L};
    ldouble_t ql[1] = {0.0L};
    ldouble_t rl[1] = {0.0L};
    ECO_SPLINE_ENDS_T ends;
    IEcoSpline* pISpline = 0;
    uint32_t i = 0;

    PrintSection("Тест 8. Обработка ошибочных входных данных");

    for (i = 0; i < 4; i++) {
        yNan[i] = y[i];
        xInf[i] = x[i];
    }
    yNan[2] = g_dZero / g_dZero;      /* NaN */
    xInf[3] = 1.0 / g_dZero;          /* +бесконечность */

    CheckCode(pIMath->pVTbl->spline(0, x, y, 4, 0, xq, 2, yq), ERR_ECO_POINTER, "spline: me = 0");
    CheckCode(pIMath->pVTbl->spline(pIMath, 0, y, 4, 0, xq, 2, yq), ERR_ECO_POINTER, "spline: x = 0");
    CheckCode(pIMath->pVTbl->spline(pIMath, x, 0, 4, 0, xq, 2, yq), ERR_ECO_POINTER, "spline: y = 0");
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 4, 0, 0, 2, yq), ERR_ECO_POINTER, "spline: xq = 0 при m = 2");
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 4, 0, xq, 2, 0), ERR_ECO_POINTER, "spline: yq = 0 при m = 2");
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 4, 0, 0, 0, 0), ERR_ECO_SUCCESS, "spline: m = 0, xq = yq = 0 (пустой запрос)");
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 0, 0, xq, 2, yq), ERR_ECOADVANCEDMATH_TOO_FEW_POINTS, "spline: n = 0");
    CheckCode(pIMath->pVTbl->splinef(pIMath, xf, yf, 1, 0, qf, 1, rf), ERR_ECOADVANCEDMATH_TOO_FEW_POINTS, "splinef: n = 1");
    CheckCode(pIMath->pVTbl->spline(pIMath, xBad, y, 4, 0, xq, 2, yq), ERR_ECOADVANCEDMATH_NOT_INCREASING, "spline: повторяющийся узел x = {0, 1, 1, 3}");
    CheckCode(pIMath->pVTbl->spline(pIMath, xDesc, y, 4, 0, xq, 2, yq), ERR_ECOADVANCEDMATH_NOT_INCREASING, "spline: убывающие узлы");
    CheckCode(pIMath->pVTbl->splinel(pIMath, xl, yl, 2, 0, ql, 1, rl), ERR_ECOADVANCEDMATH_NOT_INCREASING, "splinel: x[0] = x[1]");
    CheckCode(pIMath->pVTbl->spline(pIMath, x, yNan, 4, 0, xq, 2, yq), ERR_ECOADVANCEDMATH_NOT_FINITE, "spline: NaN в y");
    CheckCode(pIMath->pVTbl->spline(pIMath, xInf, y, 4, 0, xq, 2, yq), ERR_ECOADVANCEDMATH_NOT_FINITE, "spline: бесконечность в x");

    SetEnds(&ends, 7, 0, ECO_SPLINE_END_NATURAL, 0);
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 4, &ends, xq, 2, yq), ERR_ECOADVANCEDMATH_BAD_END_CONDITION, "spline: неизвестный тип граничного условия (7)");
    SetEnds(&ends, ECO_SPLINE_END_FIRST_DERIVATIVE, g_dZero / g_dZero, ECO_SPLINE_END_NATURAL, 0);
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 4, &ends, xq, 2, yq), ERR_ECOADVANCEDMATH_NOT_FINITE, "spline: S'(x[0]) = NaN");
    SetEnds(&ends, ECO_SPLINE_END_NATURAL, g_dZero / g_dZero, ECO_SPLINE_END_NOT_A_KNOT, 1.0 / g_dZero);
    CheckCode(pIMath->pVTbl->spline(pIMath, x, y, 4, &ends, xq, 2, yq), ERR_ECO_SUCCESS, "spline: NaN в неиспользуемых значениях условий допустим");

    pISpline = (IEcoSpline*)&value;   /* заведомо неверное значение: должно быть обнулено */
    CheckCode(pIMath->pVTbl->new_Spline(pIMath, xBad, y, 4, 0, &pISpline), ERR_ECOADVANCEDMATH_NOT_INCREASING, "new_Spline: повторяющийся узел");
    Check(pISpline == 0, "Тест 8: при ошибке new_Spline должен вернуть нулевой указатель");
    CheckCode(pIMath->pVTbl->new_Spline(pIMath, x, y, 4, 0, 0), ERR_ECO_POINTER, "new_Spline: ppISpline = 0");

    CheckCode(pIMath->pVTbl->new_Spline(pIMath, x, y, 4, 0, &pISpline), ERR_ECO_SUCCESS, "new_Spline: корректные данные");
    if (pISpline != 0) {
        CheckCode(pISpline->pVTbl->get_Breaks(pISpline, buffer, 2), ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL, "get_Breaks: буфер на 2 элемента при n = 4");
        CheckCode(pISpline->pVTbl->get_Coefs(pISpline, buffer, 2), ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL, "get_Coefs: буфер на 2 элемента при 12 коэффициентах");
        CheckCode(pISpline->pVTbl->get_Breaks(pISpline, 0, 4), ERR_ECO_POINTER, "get_Breaks: pBreaks = 0");
        CheckCode(pISpline->pVTbl->Evaluate(pISpline, 0, 1, yq), ERR_ECO_POINTER, "Evaluate: xq = 0");
        CheckCode(pISpline->pVTbl->Integrate(pISpline, 0.0, g_dZero / g_dZero, &value), ERR_ECOADVANCEDMATH_NOT_FINITE, "Integrate: b = NaN");
        CheckCode(pISpline->pVTbl->Integrate(pISpline, 0.0, 1.0, 0), ERR_ECO_POINTER, "Integrate: pResult = 0");
        pISpline->pVTbl->Release(pISpline);
    }
}


/* ========================================================================== */
/*   Тест 9. Метод прогонки против метода Гаусса; производительность          */
/* ========================================================================== */

/*
 * <summary>
 *   GaussSlopes
 * </summary>
 *
 * <description>
 *   Независимое решение той же СЛАУ для наклонов (условие not-a-knot) методом Гаусса
 *   с выбором главного элемента по столбцу в плотной матрице n x n: O(n^3) операций
 *   и O(n^2) памяти. Используется для проверки и сравнения с методом прогонки (O(n)).
 * </description>
 */
static int GaussSlopes(IEcoMemoryAllocator1* pIMem, const double_t* x, const double_t* y, uint32_t n, double_t* s) {
    double_t* A = 0;
    double_t* f = 0;
    double_t h0, h1, d0, d1, span, factor, tmp, best;
    uint32_t i, j, k, piv;

    A = (double_t*)pIMem->pVTbl->Alloc(pIMem, (n * n + n) * (uint32_t)sizeof(double_t));
    if (A == 0) {
        return -1;
    }
    f = A + n * n;
    for (i = 0; i < n * n; i++) {
        A[i] = 0;
    }
    /* Внутренние строки */
    for (i = 1; i + 1 < n; i++) {
        h0 = x[i] - x[i - 1];
        h1 = x[i + 1] - x[i];
        d0 = (y[i] - y[i - 1]) / h0;
        d1 = (y[i + 1] - y[i]) / h1;
        A[i * n + i - 1] = h1;
        A[i * n + i] = 2 * (h0 + h1);
        A[i * n + i + 1] = h0;
        f[i] = 3 * (h1 * d0 + h0 * d1);
    }
    /* not-a-knot слева и справа */
    h0 = x[1] - x[0]; h1 = x[2] - x[1]; d0 = (y[1] - y[0]) / h0; d1 = (y[2] - y[1]) / h1; span = x[2] - x[0];
    A[0] = h1;
    A[1] = span;
    f[0] = ((h0 + 2 * span) * h1 * d0 + h0 * h0 * d1) / span;
    h0 = x[n - 2] - x[n - 3]; h1 = x[n - 1] - x[n - 2]; d0 = (y[n - 2] - y[n - 3]) / h0; d1 = (y[n - 1] - y[n - 2]) / h1; span = x[n - 1] - x[n - 3];
    A[(n - 1) * n + n - 2] = span;
    A[(n - 1) * n + n - 1] = h0;
    f[n - 1] = (h1 * h1 * d0 + (2 * span + h1) * h0 * d1) / span;

    /* Прямой ход с выбором главного элемента */
    for (k = 0; k < n; k++) {
        piv = k;
        best = fabs(A[k * n + k]);
        for (i = k + 1; i < n; i++) {
            if (fabs(A[i * n + k]) > best) {
                best = fabs(A[i * n + k]);
                piv = i;
            }
        }
        if (piv != k) {
            for (j = 0; j < n; j++) {
                tmp = A[k * n + j]; A[k * n + j] = A[piv * n + j]; A[piv * n + j] = tmp;
            }
            tmp = f[k]; f[k] = f[piv]; f[piv] = tmp;
        }
        for (i = k + 1; i < n; i++) {
            factor = A[i * n + k] / A[k * n + k];
            if (factor != 0) {
                for (j = k; j < n; j++) {
                    A[i * n + j] -= factor * A[k * n + j];
                }
                f[i] -= factor * f[k];
            }
        }
    }
    /* Обратный ход */
    for (i = n; i-- > 0; ) {
        tmp = f[i];
        for (j = i + 1; j < n; j++) {
            tmp -= A[i * n + j] * s[j];
        }
        s[i] = tmp / A[i * n + i];
    }
    pIMem->pVTbl->Free(pIMem, A);
    return 0;
}

/* Тестовые данные для замеров: f(x) = sin(x) * exp(-x/10) на слегка неравномерной сетке */
static void FillBenchData(uint32_t n, double_t* x, double_t* y) {
    uint32_t i = 0;

    for (i = 0; i < n; i++) {
        x[i] = 10.0 * ((double_t)i + 0.3 * sin(0.7 * (double_t)i)) / (double_t)n;
        y[i] = sin(x[i]) * exp(-x[i] / 10.0);
    }
}

static void TestThomasVsGauss(IEcoAdvancedMath* pIMath, IEcoMemoryAllocator1* pIMem) {
    static const uint32_t sizes[5] = {100, 200, 400, 800, 1600};
    IEcoSpline* pISpline = 0;
    double_t* x = 0;
    double_t* y = 0;
    double_t* sT = 0;
    double_t* sG = 0;
    double_t err = 0;
    double_t tThomas = 0;
    double_t tGauss = 0;
    clock_t start;
    uint32_t reps = 0;
    uint32_t c = 0;
    uint32_t i = 0;
    uint32_t n = 0;

    PrintSection("Тест 9.1. Метод прогонки (компонент) против метода Гаусса (плотная матрица) на той же СЛАУ");
    {
        static const char_t* const cells[] = {"n", "max|s_прогонка - s_Гаусс|", "построение сплайна, мкс", "Гаусс, мкс", "ускорение", 0};
        static const int widths[] = {6, 25, 23, 16, 10};
        printf("\n");
        PrintHeader(cells, widths);
    }

    for (c = 0; c < 5; c++) {
        n = sizes[c];
        x = (double_t*)pIMem->pVTbl->Alloc(pIMem, 4 * n * (uint32_t)sizeof(double_t));
        if (x == 0) {
            printf("    недостаточно памяти для n = %lu\n", (unsigned long)n);
            break;
        }
        y = x + n;
        sT = y + n;
        sG = sT + n;
        FillBenchData(n, x, y);

        /* Наклоны компонента: S'(x[i]) объекта, построенного методом прогонки */
        start = clock();
        reps = 0;
        do {
            pIMath->pVTbl->new_Spline(pIMath, x, y, n, 0, &pISpline);
            if (pISpline != 0) {
                pISpline->pVTbl->EvaluateDerivative(pISpline, 1, x, n, sT);
                pISpline->pVTbl->Release(pISpline);
                pISpline = 0;
            }
            reps++;
        } while ((double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS);
        tThomas = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;

        start = clock();
        reps = 0;
        do {
            GaussSlopes(pIMem, x, y, n, sG);
            reps++;
        } while ((double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS);
        tGauss = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;

        err = 0;
        for (i = 0; i < n; i++) {
            if (fabs(sT[i] - sG[i]) > err) {
                err = fabs(sT[i] - sG[i]);
            }
        }
        printf("    %6lu  %25.3e  %23.2f  %16.2f  %9.0fx\n", (unsigned long)n, err, tThomas * 1e6, tGauss * 1e6, tGauss / tThomas);
        Check(err <= 1e-11, "Тест 9.1: решения метода прогонки и метода Гаусса не совпадают");
        pIMem->pVTbl->Free(pIMem, x);
    }
    printf("\n    Время компонента - построение сплайна целиком (прогонка, коэффициенты, первообразная),\n");
    printf("    время метода Гаусса - только решение СЛАУ. Первое растет линейно (O(n)), второе - кубически (O(n^3)).\n");
}

static void TestPerformance(IEcoAdvancedMath* pIMath, IEcoMemoryAllocator1* pIMem) {
    static const uint32_t sizes[4] = {1000, 10000, 100000, 1000000};
    IEcoSpline* pISpline = 0;
    double_t* x = 0;
    double_t* y = 0;
    double_t* xq = 0;
    double_t* yq = 0;
    float_t* xf = 0;
    float_t* yf = 0;
    float_t* xqf = 0;
    float_t* yqf = 0;
    ldouble_t* xl = 0;
    ldouble_t* yl = 0;
    ldouble_t* xql = 0;
    ldouble_t* yql = 0;
    double_t t[5];
    clock_t start;
    uint32_t reps = 0;
    uint32_t c = 0;
    uint32_t i = 0;
    uint32_t n = 0;
    uint32_t m = 0;
    int16_t result = 0;

    PrintSection("Тест 9.2. Производительность (CPU-время clock(), m = n точек вычисления, -O2)");
    {
        static const char_t* const cells[] = {"n", "spline, мс", "splinef, мс", "splinel, мс", "new_Spline, мс", "Evaluate, мс", "нс/точку", 0};
        static const int widths[] = {8, 14, 14, 14, 14, 14, 10};
        printf("\n");
        PrintHeader(cells, widths);
    }

    for (c = 0; c < 4; c++) {
        n = sizes[c];
        m = n;
        x = (double_t*)pIMem->pVTbl->Alloc(pIMem, (2 * n + 2 * m) * (uint32_t)sizeof(double_t));
        xf = (float_t*)pIMem->pVTbl->Alloc(pIMem, (2 * n + 2 * m) * (uint32_t)sizeof(float_t));
        xl = (ldouble_t*)pIMem->pVTbl->Alloc(pIMem, (2 * n + 2 * m) * (uint32_t)sizeof(ldouble_t));
        if (x == 0 || xf == 0 || xl == 0) {
            printf("    %8lu  недостаточно памяти\n", (unsigned long)n);
            if (x != 0) pIMem->pVTbl->Free(pIMem, x);
            if (xf != 0) pIMem->pVTbl->Free(pIMem, xf);
            if (xl != 0) pIMem->pVTbl->Free(pIMem, xl);
            break;
        }
        y = x + n; xq = y + n; yq = xq + m;
        yf = xf + n; xqf = yf + n; yqf = xqf + m;
        yl = xl + n; xql = yl + n; yql = xql + m;
        FillBenchData(n, x, y);
        for (i = 0; i < m; i++) {
            xq[i] = 10.0 * (double_t)i / (double_t)m;
        }
        for (i = 0; i < n; i++) {
            xf[i] = (float_t)x[i]; yf[i] = (float_t)y[i];
            xl[i] = x[i]; yl[i] = y[i];
        }
        for (i = 0; i < m; i++) {
            xqf[i] = (float_t)xq[i];
            xql[i] = xq[i];
        }

        /* spline (double): построение + вычисление */
        start = clock(); reps = 0;
        do {
            result = pIMath->pVTbl->spline(pIMath, x, y, n, 0, xq, m, yq);
            reps++;
        } while ((double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS);
        t[0] = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;
        Check(result == ERR_ECO_SUCCESS, "Тест 9.2: spline вернул ошибку");

        /* splinef (float) */
        start = clock(); reps = 0;
        do {
            result = pIMath->pVTbl->splinef(pIMath, xf, yf, n, 0, xqf, m, yqf);
            reps++;
        } while ((double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS);
        t[1] = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;
        Check(result == ERR_ECO_SUCCESS, "Тест 9.2: splinef вернул ошибку");

        /* splinel (long double) */
        start = clock(); reps = 0;
        do {
            result = pIMath->pVTbl->splinel(pIMath, xl, yl, n, 0, xql, m, yql);
            reps++;
        } while ((double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS);
        t[2] = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;
        Check(result == ERR_ECO_SUCCESS, "Тест 9.2: splinel вернул ошибку");

        /* new_Spline: только построение объекта */
        start = clock(); reps = 0;
        do {
            result = pIMath->pVTbl->new_Spline(pIMath, x, y, n, 0, &pISpline);
            if (pISpline != 0 && (double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS) {
                pISpline->pVTbl->Release(pISpline);
                pISpline = 0;
            }
            reps++;
        } while (pISpline == 0 && result == ERR_ECO_SUCCESS);
        t[3] = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;
        Check(result == ERR_ECO_SUCCESS && pISpline != 0, "Тест 9.2: new_Spline вернул ошибку");

        /* Evaluate: только вычисление в m точках по готовому объекту */
        t[4] = 0;
        if (pISpline != 0) {
            start = clock(); reps = 0;
            do {
                pISpline->pVTbl->Evaluate(pISpline, xq, m, yq);
                reps++;
            } while ((double_t)(clock() - start) / CLOCKS_PER_SEC < BENCH_MIN_SECONDS);
            t[4] = (double_t)(clock() - start) / CLOCKS_PER_SEC / reps;
            pISpline->pVTbl->Release(pISpline);
            pISpline = 0;
        }

        printf("    %8lu  %14.3f  %14.3f  %14.3f  %14.3f  %14.3f  %10.1f\n", (unsigned long)n, t[0] * 1e3, t[1] * 1e3, t[2] * 1e3, t[3] * 1e3, t[4] * 1e3, t[0] * 1e9 / (double_t)n);

        pIMem->pVTbl->Free(pIMem, x);
        pIMem->pVTbl->Free(pIMem, xf);
        pIMem->pVTbl->Free(pIMem, xl);
    }
    printf("\n    spline/splinef/splinel - построение сплайна (O(n)) и вычисление в m точках (O(m log n)).\n");
}


/* ========================================================================== */
/*   Экспорт данных для графиков отчета                                       */
/* ========================================================================== */

/*
 * <summary>
 *   ExportPlotData
 * </summary>
 *
 * <description>
 *   Сохраняет в CSV значения сплайнов на плотной сетке (через объект IEcoSpline):
 *     EcoAdvancedMath_spline_sin.csv  - пример MATLAB: x, sin(x), S(x), S'(x), S''(x);
 *     EcoAdvancedMath_spline_ends.csv - набор B: x, S для четырех типов граничных условий.
 *   Файлы создаются в текущем каталоге и используются для построения графиков отчета.
 * </description>
 */
static void ExportPlotData(IEcoAdvancedMath* pIMath) {
    static const uint16_t types[4] = {ECO_SPLINE_END_NOT_A_KNOT, ECO_SPLINE_END_NATURAL, ECO_SPLINE_END_FIRST_DERIVATIVE, ECO_SPLINE_END_SECOND_DERIVATIVE};
    static const double_t leftValues[4] = {0.0, 0.0, 1.0, 2.0};
    static const double_t rightValues[4] = {0.0, 0.0, -0.5, -1.0};
    const uint32_t m = 1101;
    IEcoSpline* pISpline = 0;
    IEcoSpline* pIEnds[4] = {0, 0, 0, 0};
    ECO_SPLINE_ENDS_T ends;
    FILE* fp = 0;
    double_t x = 0;
    uint32_t i = 0;
    uint32_t c = 0;

    PrintSection("Тест 10. Экспорт данных для графиков отчета (CSV)");

    /* Пример MATLAB: значения, первая и вторая производные */
    if (pIMath->pVTbl->new_Spline(pIMath, g_RefA_X, g_RefA_Y, REF_A_N, 0, &pISpline) == ERR_ECO_SUCCESS) {
        fp = fopen("EcoAdvancedMath_spline_sin.csv", "w");
        if (fp != 0) {
            fprintf(fp, "x,sin,S,dS,d2S\n");
            for (i = 0; i < m; i++) {
                x = -0.5 + 11.0 * (double_t)i / (double_t)(m - 1);
                fprintf(fp, "%.17g,%.17g,%.17g,%.17g,%.17g\n", x, sin(x), EvalOne(pISpline, 0, x), EvalOne(pISpline, 1, x), EvalOne(pISpline, 2, x));
            }
            fclose(fp);
            printf("    сохранено: EcoAdvancedMath_spline_sin.csv  (%lu точек: x, sin, S, S', S'')\n", (unsigned long)m);
        }
        else {
            printf("    не удалось создать EcoAdvancedMath_spline_sin.csv\n");
        }
        pISpline->pVTbl->Release(pISpline);
    }

    /* Набор B: четыре типа граничных условий */
    for (c = 0; c < 4; c++) {
        SetEnds(&ends, types[c], leftValues[c], types[c], rightValues[c]);
        pIMath->pVTbl->new_Spline(pIMath, g_RefB_X, g_RefB_Y, REF_B_N, &ends, &pIEnds[c]);
    }
    if (pIEnds[0] != 0 && pIEnds[1] != 0 && pIEnds[2] != 0 && pIEnds[3] != 0) {
        fp = fopen("EcoAdvancedMath_spline_ends.csv", "w");
        if (fp != 0) {
            fprintf(fp, "x,not_a_knot,natural,clamped,second\n");
            for (i = 0; i < m; i++) {
                x = -0.5 + 6.0 * (double_t)i / (double_t)(m - 1);
                fprintf(fp, "%.17g,%.17g,%.17g,%.17g,%.17g\n", x, EvalOne(pIEnds[0], 0, x), EvalOne(pIEnds[1], 0, x), EvalOne(pIEnds[2], 0, x), EvalOne(pIEnds[3], 0, x));
            }
            fclose(fp);
            printf("    сохранено: EcoAdvancedMath_spline_ends.csv (%lu точек, 4 типа граничных условий)\n", (unsigned long)m);
        }
        else {
            printf("    не удалось создать EcoAdvancedMath_spline_ends.csv\n");
        }
    }
    for (c = 0; c < 4; c++) {
        if (pIEnds[c] != 0) {
            pIEnds[c]->pVTbl->Release(pIEnds[c]);
        }
    }
}


/*
 *
 * <summary>
 *   EcoMain Function
 * </summary>
 *
 * <description>
 *   EcoMain function - entry point
 * </description>
 *
 */
void TestSplineEvents(IEcoInterfaceBus1* pBus, IEcoAdvancedMath* pFirst, void (*Check)(int, const char_t*));

int16_t EcoMain(IEcoUnknown* pIUnk) {
    int16_t result = -1;
    /* Pointer to the system interface */
    IEcoSystem1* pISys = 0;
    /* Pointer to the interface for working with the system interface bus */
    IEcoInterfaceBus1* pIBus = 0;
    /* Pointer to the memory management interface */
    IEcoMemoryAllocator1* pIMem = 0;
    /* Pointer to the tested interface */
    IEcoAdvancedMath* pIEcoAdvancedMath = 0;

    /* System interface check and creation */
    if (pISys == 0) {
        result = pIUnk->pVTbl->QueryInterface(pIUnk, &GID_IEcoSystem, (void **)&pISys);
        if (result != 0 || pISys == 0) {
        /* Free the system interface in case of an error */
            goto Release;
        }
    }

    /* Getting the interface for working with the interface bus */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);
    if (result != 0 || pIBus == 0) {
        /* Free in case of an error */
        goto Release;
    }
#ifdef ECO_LIB
    /* Registration of a static component EcoAdvancedMath */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoAdvancedMath, (IEcoUnknown*)GetIEcoComponentFactoryPtr_A2A2065C8C3B4926BC6C30A84CAA1B2F);
    if (result != 0 ) {
        /* Free in case of an error */
        goto Release;
    }
#endif
    /* Getting the memory management interface */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoMemoryManager1, 0, &IID_IEcoMemoryAllocator1, (void**) &pIMem);

    /* Check */
    if (result != 0 || pIMem == 0) {
        /* Free the system interface in case of an error */
        goto Release;
    }

    /* Getting the tested interface */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoAdvancedMath, 0, &IID_IEcoAdvancedMath, (void**) &pIEcoAdvancedMath);
    if (result != 0 || pIEcoAdvancedMath == 0) {
        /* Free interfaces in case of an error */
        printf("Не удалось получить интерфейс IEcoAdvancedMath (код %d)\n", (int)result);
        goto Release;
    }

    printf("Юнит-тест компонента EcoAdvancedMath: кубическая сплайн-интерполяция (задача 37)\n");
    printf("Интерфейс IEcoAdvancedMath получен через интерфейсную шину (CID_EcoAdvancedMath)\n");

    TestMatlabExample(pIEcoAdvancedMath);
    TestEndConditions(pIEcoAdvancedMath);
    TestDegenerateCases(pIEcoAdvancedMath);
    TestPolynomialPrecision(pIEcoAdvancedMath);
    TestSmoothness(pIEcoAdvancedMath);
    TestSplineObject(pIEcoAdvancedMath);
    TestConvergence(pIEcoAdvancedMath, pIMem);
    TestErrors(pIEcoAdvancedMath);
    TestThomasVsGauss(pIEcoAdvancedMath, pIMem);
    TestPerformance(pIEcoAdvancedMath, pIMem);
    ExportPlotData(pIEcoAdvancedMath);
    TestSplineEvents(pIBus, pIEcoAdvancedMath, Check);

    PrintSection("Итог");
    printf("Проверок: %lu, успешно: %lu, с ошибками: %lu\n", (unsigned long)g_nChecks, (unsigned long)(g_nChecks - g_nFailed), (unsigned long)g_nFailed);
    printf("%s\n", g_nFailed == 0 ? "ВСЕ ТЕСТЫ ПРОЙДЕНЫ" : "ЕСТЬ ОШИБКИ");
    result = g_nFailed == 0 ? ERR_ECO_SUCCESS : ERR_ECO_FAIL;

Release:

    /* Free the interface for working with the interface bus */
    if (pIBus != 0) {
        pIBus->pVTbl->Release(pIBus);
    }

    /* Free the memory management interface */
    if (pIMem != 0) {
        pIMem->pVTbl->Release(pIMem);
    }

    /* Free the tested interface */
    if (pIEcoAdvancedMath != 0) {
        pIEcoAdvancedMath->pVTbl->Release(pIEcoAdvancedMath);
    }


    /* Free the system interface */
    if (pISys != 0) {
        pISys->pVTbl->Release(pISys);
    }

    return result;
}
