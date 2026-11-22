/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoSpline_4CAA1B2F
 * </summary>
 *
 * <description>
 *   Объект данных компонента EcoAdvancedMath, реализующий интерфейс IEcoSpline
 *   (кубический сплайн в кусочно-полиномиальной форме, double).
 *
 *   Объект и его массивы размещаются в одном блоке памяти, выделенном через
 *   IEcoMemoryAllocator1: [структура][узлы n][коэффициенты 4*(n-1)][первообразная n].
 *   Объект хранит собственную ссылку на распределитель памяти, поэтому может жить
 *   дольше создавшего его компонента.
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __C_ECOSPLINE_H__
#define __C_ECOSPLINE_H__

#include "IEcoSpline.h"
#include "IEcoAdvancedMath.h"
#include "IEcoMemoryAllocator1.h"

typedef struct CEcoSpline_4CAA1B2F* CEcoSpline_4CAA1B2FPtr_t;

typedef struct CEcoSpline_4CAA1B2F {

    /* IEcoSpline interface function table */
    IEcoSplineVTbl* m_pVTblIEcoSpline;

    /* Reference counter */
    uint32_t m_cRef;

    /* Interface for memory operations */
    IEcoMemoryAllocator1* m_pIMem;

    /* Instance data */
    uint32_t m_nBreaks;        /* число узлов n (кусков n - 1) */
    double_t* m_pBreaks;       /* узлы x[0..n-1] */
    double_t* m_pCoefs;        /* коэффициенты кусков {a, b, c, d} * (n - 1) */
    double_t* m_pIntegrals;    /* значения первообразной в узлах: F[k] = интеграл S от x[0] до x[k] */

} CEcoSpline_4CAA1B2F;

/*
 * <summary>
 *   createCEcoSpline_4CAA1B2F
 * </summary>
 *
 * <description>
 *   Строит сплайн по узлам (x, y) с граничными условиями pEnds (0 - not-a-knot)
 *   и возвращает объект через ppISpline (счетчик ссылок = 1).
 *   Вызывается из IEcoAdvancedMath::new_Spline.
 * </description>
 */
#include "CEcoSplineEvents.h"

int16_t createCEcoSpline_4CAA1B2F(/* in */ IEcoMemoryAllocator1* pIMem, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* out */ IEcoSpline** ppISpline, /* in */ CEcoSplineEventSource* pSource);

#endif /* __C_ECOSPLINE_H__ */
