/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoAdvancedMath_4CAA1B2F
 * </summary>
 *
 * <description>
 *   This header describes the implementation of the CEcoAdvancedMath_4CAA1B2F component.
 *   Компонент реализует сервисный интерфейс IEcoAdvancedMath (кубическая сплайн-интерполяция)
 *   и создает объекты данных IEcoSpline.
 *   Компонент поддерживает агрегирование внешним компонентом и предоставляет интерфейсы
 *   калькулятора IEcoCalculatorX / IEcoCalculatorY, повторно используя компоненты курса:
 *   B - агрегированием, A (или его вторую версию C), D, E - включением.
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __C_ECOADVANCEDMATH_H__
#define __C_ECOADVANCEDMATH_H__

#include "IEcoAdvancedMath.h"
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

typedef struct CEcoAdvancedMath_4CAA1B2F* CEcoAdvancedMath_4CAA1B2FPtr_t;

typedef struct CEcoAdvancedMath_4CAA1B2F {

    /* IEcoAdvancedMath interface function table */
    IEcoAdvancedMathVTbl* m_pVTblIEcoAdvancedMath;


    /* Instance initialization */
    int16_t (ECOCALLMETHOD *Init)(/*in*/ CEcoAdvancedMath_4CAA1B2FPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem);
    /* Instance creation */
    int16_t (ECOCALLMETHOD *Create)(/*in*/ CEcoAdvancedMath_4CAA1B2FPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter);
    /* Deletion */
    void (ECOCALLMETHOD *Delete)(/*in*/ CEcoAdvancedMath_4CAA1B2FPtr_t pCMe);


    /* Reference counter */
    uint32_t m_cRef;

    /* Interface for memory operations */
    IEcoMemoryAllocator1* m_pIMem;

    /* System interface */
    IEcoSystem1* m_pISys;

    /* Неделегирующий IEcoUnknown (поддержка агрегирования) */
    IEcoUnknownVTbl* m_pVTblINondelegatingUnk;

    /* Управляющий IEcoUnknown: внешний компонент при агрегировании, иначе - собственный неделегирующий */
    IEcoUnknown* m_pIUnkOuter;

    /* Таблицы функций интерфейсов калькулятора, реализованных включением */
    IEcoCalculatorXVTbl* m_pVTblIEcoCalculatorX;
    IEcoCalculatorYVTbl* m_pVTblIEcoCalculatorY;

    /* Внутренние компоненты калькулятора */
    IEcoUnknown* m_pIUnkInnerX;     /* B - агрегирование: IEcoCalculatorX передается клиенту напрямую */
    IEcoCalculatorX* m_pIX;         /* A или его вторая версия C - включение, если B недоступен */
    IEcoCalculatorY* m_pIY;         /* D, E или C - включение */

} CEcoAdvancedMath_4CAA1B2F;

#endif /* __C_ECOADVANCEDMATH_H__ */
