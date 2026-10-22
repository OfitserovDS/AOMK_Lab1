/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoLab2_2F9385F6
 * </summary>
 *
 * <description>
 *   This header describes the implementation of the CEcoLab2_2F9385F6 component.
 *   Внешний компонент: агрегирует Eco.AdvancedMath и включает компонент калькулятора E.
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __C_ECOLAB2_H__
#define __C_ECOLAB2_H__

#include "IEcoLab2.h"
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IEcoCalculatorX.h"
#include "IEcoCalculatorY.h"

typedef struct CEcoLab2_2F9385F6* CEcoLab2_2F9385F6Ptr_t;

typedef struct CEcoLab2_2F9385F6 {

    /* IEcoLab2 interface function table */
    IEcoLab2VTbl* m_pVTblIEcoLab2;

    /* Таблицы функций интерфейсов калькулятора, реализованных включением компонента E */
    IEcoCalculatorXVTbl* m_pVTblIEcoCalculatorX;
    IEcoCalculatorYVTbl* m_pVTblIEcoCalculatorY;


    /* Instance initialization */
    int16_t (ECOCALLMETHOD *Init)(/*in*/ CEcoLab2_2F9385F6Ptr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem);
    /* Instance creation */
    int16_t (ECOCALLMETHOD *Create)(/*in*/ CEcoLab2_2F9385F6Ptr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter);
    /* Deletion */
    void (ECOCALLMETHOD *Delete)(/*in*/ CEcoLab2_2F9385F6Ptr_t pCMe);


    /* Reference counter */
    uint32_t m_cRef;

    /* Interface for memory operations */
    IEcoMemoryAllocator1* m_pIMem;

    /* System interface */
    IEcoSystem1* m_pISys;

    /* Агрегированный компонент Eco.AdvancedMath: его неделегирующий IEcoUnknown */
    IEcoUnknown* m_pIUnkInnerMath;

    /* Включенный компонент E: интерфейс Y и полученный у него интерфейс X
       (это интерфейс агрегированного внутри E компонента B) */
    IEcoCalculatorY* m_pIY;
    IEcoCalculatorX* m_pIX;

} CEcoLab2_2F9385F6;

#endif /* __C_ECOLAB2_H__ */
