/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IEcoSpline
 * </summary>
 *
 * <description>
 *   Интерфейс данных IEcoSpline - кубический сплайн в кусочно-полиномиальной форме
 *   (аналог структуры pp, которую возвращает pp = spline(x, y) в MATLAB).
 *
 *   Сплайн задан узлами (breaks) x[0] < x[1] < ... < x[n-1] и n-1 кубическими кусками.
 *   На k-м куске (x[k] <= x < x[k+1], k = 0..n-2) с локальной переменной t = x - x[k]:
 *       S(x) = a[k]*t^3 + b[k]*t^2 + c[k]*t + d[k].
 *   Коэффициенты хранятся построчно: coefs[4*k + 0..3] = {a[k], b[k], c[k], d[k]}
 *   (тот же порядок, что в pp.coefs MATLAB - от старшей степени к младшей).
 *   Вне отрезка [x[0], x[n-1]] сплайн продолжается крайними кусками (экстраполяция,
 *   как в ppval MATLAB).
 *
 *   Объект создается методом IEcoAdvancedMath::new_Spline и освобождается вызовом Release.
 * </description>
 *
 * <reference>
 *   https://www.mathworks.com/help/matlab/ref/ppval.html
 * </reference>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __I_ECOSPLINE_H__
#define __I_ECOSPLINE_H__

#include "IEcoBase1.h"

/* IEcoSpline IID = {B0ED03B2-1831-4747-8635-F0C9191A7C2B} */
#ifndef __IID_IEcoSpline
static const UGUID IID_IEcoSpline = {0x01, 0x10, {0xB0, 0xED, 0x03, 0xB2, 0x18, 0x31, 0x47, 0x47, 0x86, 0x35, 0xF0, 0xC9, 0x19, 0x1A, 0x7C, 0x2B}};
#endif /* __IID_IEcoSpline */

typedef struct IEcoSpline* IEcoSplinePtr_t;

typedef struct IEcoSplineVTbl {

    /* IEcoUnknown */
    int16_t (ECOCALLMETHOD *QueryInterface)(/* in */ IEcoSplinePtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv);
    uint32_t (ECOCALLMETHOD *AddRef)(/* in */ IEcoSplinePtr_t me);
    uint32_t (ECOCALLMETHOD *Release)(/* in */ IEcoSplinePtr_t me);

    /* IEcoSpline */

    /* Число кубических кусков (n - 1) */
    uint32_t (ECOCALLMETHOD *get_Pieces)(/* in */ IEcoSplinePtr_t me);

    /* Копирует n узлов сплайна в pBreaks (size - размер буфера в элементах, size >= n) */
    int16_t (ECOCALLMETHOD *get_Breaks)(/* in */ IEcoSplinePtr_t me, /* out */ double_t* pBreaks, /* in */ uint32_t size);

    /* Копирует 4*(n-1) коэффициентов в pCoefs (size >= 4*(n-1)) */
    int16_t (ECOCALLMETHOD *get_Coefs)(/* in */ IEcoSplinePtr_t me, /* out */ double_t* pCoefs, /* in */ uint32_t size);

    /* yq[j] = S(xq[j]), j = 0..m-1 (аналог ppval) */
    int16_t (ECOCALLMETHOD *Evaluate)(/* in */ IEcoSplinePtr_t me, /* in */ const double_t* xq, /* in */ uint32_t m, /* out */ double_t* yq);

    /* yq[j] = S^(order)(xq[j]) - производная порядка order (0 - значение, при order > 3 результат 0) */
    int16_t (ECOCALLMETHOD *EvaluateDerivative)(/* in */ IEcoSplinePtr_t me, /* in */ uint32_t order, /* in */ const double_t* xq, /* in */ uint32_t m, /* out */ double_t* yq);

    /* *pResult = интеграл S(x) dx от a до b (точное интегрирование кубических кусков) */
    int16_t (ECOCALLMETHOD *Integrate)(/* in */ IEcoSplinePtr_t me, /* in */ double_t a, /* in */ double_t b, /* out */ double_t* pResult);

} IEcoSplineVTbl, *IEcoSplineVTblPtr_t;

interface IEcoSpline {
    struct IEcoSplineVTbl *pVTbl;
} IEcoSpline;

#endif /* __I_ECOSPLINE_H__ */
