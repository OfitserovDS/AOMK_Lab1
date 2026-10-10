/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IEcoAdvancedMath
 * </summary>
 *
 * <description>
 *   Интерфейс IEcoAdvancedMath (раздел 2 «Продвинутые спецфункции и численный анализ»).
 *   Задача 37: кубическая сплайн-интерполяция (аналог функции spline в MATLAB)
 *   с ручной реализацией метода прогонки для трехдиагональной матрицы.
 *
 *   Кубический сплайн S(x) - функция из кубических многочленов на отрезках [x[k], x[k+1]],
 *   проходящая через все узлы (S(x[i]) = y[i]) и имеющая непрерывные S, S', S''.
 *   Недостающие два условия задаются на концах (ECO_SPLINE_ENDS_T). По умолчанию
 *   (pEnds = 0) используется условие «not-a-knot», как в MATLAB spline.
 *
 *   Требования к входным данным: n >= 2, узлы x строго возрастают, все значения конечны.
 *   Выходной массив yq не должен перекрываться с массивами x и y.
 * </description>
 *
 * <reference>
 *   https://www.mathworks.com/help/matlab/ref/spline.html
 *   https://en.wikipedia.org/wiki/Spline_interpolation
 *   https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
 * </reference>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __I_ECOADVANCEDMATH_H__
#define __I_ECOADVANCEDMATH_H__

#include "IEcoBase1.h"
#include "IEcoSpline.h"

/* Тип long double: фреймворк определяет ldouble_t только для Eco OS, поэтому определяем его здесь */
#ifndef ECO_LDOUBLE_T_DEFINED
typedef long double ldouble_t;
#define ECO_LDOUBLE_T_DEFINED
#endif

/* Типы граничных условий сплайна (поля leftType / rightType структуры ECO_SPLINE_ENDS_T) */
typedef enum ECO_SPLINE_END_TYPE {
    ECO_SPLINE_END_NOT_A_KNOT        = 0,   /* S''' непрерывна в x[1] и x[n-2] (по умолчанию, как в MATLAB) */
    ECO_SPLINE_END_NATURAL           = 1,   /* S'' = 0 на конце (естественный сплайн) */
    ECO_SPLINE_END_FIRST_DERIVATIVE  = 2,   /* задано значение S' на конце («зажатый» сплайн, clamped) */
    ECO_SPLINE_END_SECOND_DERIVATIVE = 3    /* задано значение S'' на конце */
} ECO_SPLINE_END_TYPE;

/* Граничные условия на левом (x[0]) и правом (x[n-1]) концах.
 * Значения leftValue/rightValue используются только для типов FIRST_DERIVATIVE и SECOND_DERIVATIVE.
 * Значения хранятся в long double, поэтому одна структура подходит для всех перегрузок. */
typedef struct ECO_SPLINE_ENDS_T {
    uint16_t  leftType;
    uint16_t  rightType;
    ldouble_t leftValue;
    ldouble_t rightValue;
} ECO_SPLINE_ENDS_T;

/* IEcoAdvancedMath IID = {A8A4C017-2F92-46D4-A3F7-870EAC1CDCB6} */
#ifndef __IID_IEcoAdvancedMath
static const UGUID IID_IEcoAdvancedMath = {0x01, 0x10, {0xA8, 0xA4, 0xC0, 0x17, 0x2F, 0x92, 0x46, 0xD4, 0xA3, 0xF7, 0x87, 0x0E, 0xAC, 0x1C, 0xDC, 0xB6}};
#endif /* __IID_IEcoAdvancedMath */

typedef struct IEcoAdvancedMath* IEcoAdvancedMathPtr_t;

typedef struct IEcoAdvancedMathVTbl {

    /* IEcoUnknown */
    int16_t (ECOCALLMETHOD *QueryInterface)(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv);
    uint32_t (ECOCALLMETHOD *AddRef)(/* in */ IEcoAdvancedMathPtr_t me);
    uint32_t (ECOCALLMETHOD *Release)(/* in */ IEcoAdvancedMathPtr_t me);

    /* IEcoAdvancedMath */

    /* yq = spline(x, y, xq): сплайн по n узлам (x, y), вычисленный в m точках xq (double) */
    int16_t (ECOCALLMETHOD *spline)(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const double_t* xq, /* in */ uint32_t m, /* out */ double_t* yq);

    /* То же для float */
    int16_t (ECOCALLMETHOD *splinef)(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const float_t* x, /* in */ const float_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const float_t* xq, /* in */ uint32_t m, /* out */ float_t* yq);

    /* То же для long double */
    int16_t (ECOCALLMETHOD *splinel)(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const ldouble_t* x, /* in */ const ldouble_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const ldouble_t* xq, /* in */ uint32_t m, /* out */ ldouble_t* yq);

    /* pp = spline(x, y): создает объект IEcoSpline (кусочно-полиномиальная форма, double) */
    int16_t (ECOCALLMETHOD *new_Spline)(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* out */ IEcoSpline** ppISpline);

} IEcoAdvancedMathVTbl, *IEcoAdvancedMathVTblPtr_t;

interface IEcoAdvancedMath {
    struct IEcoAdvancedMathVTbl *pVTbl;
} IEcoAdvancedMath;


#endif /* __I_ECOADVANCEDMATH_H__ */
