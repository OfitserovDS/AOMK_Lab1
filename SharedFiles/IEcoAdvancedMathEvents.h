/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IEcoAdvancedMathEvents — события построения кубического сплайна.
 * </summary>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 */

#ifndef __IECOADVANCEDMATHEVENTS_H__
#define __IECOADVANCEDMATHEVENTS_H__

#include "IEcoAdvancedMath.h"

/* IID = 2AB6DF5D-1F33-4D79-AB1A-C5AA48630538 */
static const UGUID IID_IEcoAdvancedMathEvents = {
    0x01, 0x10, {0x2A, 0xB6, 0xDF, 0x5D, 0x1F, 0x33, 0x4D, 0x79,
                 0xAB, 0x1A, 0xC5, 0xAA, 0x48, 0x63, 0x05, 0x38}
};

typedef struct IEcoAdvancedMathEvents* IEcoAdvancedMathEventsPtr_t;

/*
 * Обратные вызовы синхронны. pSource заимствован на время вызова.
 * Индексы начинаются с 0. OnSystemRow передаёт исходную строку СЛАУ,
 * OnForwardStep — коэффициенты alpha и beta после исключения,
 * OnSlopeReady — окончательный наклон, OnPieceReady — коэффициенты при
 * t^3, t^2, t и 1, где t = x - x[piece]. Для прямой и параболы при
 * двустороннем not-a-knot публикуются только готовые наклоны и куски.
 * Коды из обработчиков не влияют на вычисление и рассылку.
 * Состав получателей фиксируется на один вызов. Если снимок нельзя
 * выделить, вызов возвращает ERR_ECO_OUTOFMEMORY до начала расчёта.
 * Одновременные вызовы из разных потоков требуют внешней синхронизации.
 */
typedef struct IEcoAdvancedMathEventsVTbl {
    int16_t (ECOCALLMETHOD *QueryInterface)(/* in */ IEcoAdvancedMathEventsPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv);
    uint32_t (ECOCALLMETHOD *AddRef)(/* in */ IEcoAdvancedMathEventsPtr_t me);
    uint32_t (ECOCALLMETHOD *Release)(/* in */ IEcoAdvancedMathEventsPtr_t me);

    int16_t (ECOCALLMETHOD *OnSystemRow)(/* in */ IEcoAdvancedMathEventsPtr_t me, /* in */ IEcoAdvancedMath* pSource, /* in */ uint32_t row,
        /* in */ ldouble_t lower, /* in */ ldouble_t diagonal, /* in */ ldouble_t upper, /* in */ ldouble_t rhs);
    int16_t (ECOCALLMETHOD *OnForwardStep)(/* in */ IEcoAdvancedMathEventsPtr_t me, /* in */ IEcoAdvancedMath* pSource, /* in */ uint32_t row,
        /* in */ ldouble_t alpha, /* in */ ldouble_t beta);
    int16_t (ECOCALLMETHOD *OnSlopeReady)(/* in */ IEcoAdvancedMathEventsPtr_t me, /* in */ IEcoAdvancedMath* pSource, /* in */ uint32_t node,
        /* in */ ldouble_t slope);
    int16_t (ECOCALLMETHOD *OnPieceReady)(/* in */ IEcoAdvancedMathEventsPtr_t me, /* in */ IEcoAdvancedMath* pSource, /* in */ uint32_t piece,
        /* in */ ldouble_t a, /* in */ ldouble_t b, /* in */ ldouble_t c, /* in */ ldouble_t d);
} IEcoAdvancedMathEventsVTbl;

interface IEcoAdvancedMathEvents {
    IEcoAdvancedMathEventsVTbl* pVTbl;
} IEcoAdvancedMathEvents;

#endif /* __IECOADVANCEDMATHEVENTS_H__ */
