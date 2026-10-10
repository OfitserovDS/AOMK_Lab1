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

} CEcoAdvancedMath_4CAA1B2F;

#endif /* __C_ECOADVANCEDMATH_H__ */
