/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   Внутренний интерфейс рассылки событий построения сплайна.
 * </summary>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 */

#ifndef __C_ECOSPLINEEVENTS_H__
#define __C_ECOSPLINEEVENTS_H__

#include "IEcoAdvancedMath.h"
#include "IEcoEnumConnections.h"

typedef struct CEcoAdvancedMath_4CAA1B2F CEcoSplineEventSource;
typedef struct CEcoSplineTrace {
    IEcoEnumConnections* pEnum;
    IEcoAdvancedMath* pSource;
} CEcoSplineTrace;

void EcoSplineInitConnections(/* in */ CEcoSplineEventSource* pSource);
void EcoSplineClearConnections(/* in */ CEcoSplineEventSource* pSource);

/*
 * Снимок строится один раз для вызова. Изменение подписок из обработчика
 * вступает в силу при следующем вычислении. Без подписок снимок равен 0.
 */
int16_t EcoSplineBeginTrace(/* in */ CEcoSplineEventSource* pSource, /* out */ CEcoSplineTrace* pTrace);
void EcoSplineEndTrace(/* in */ CEcoSplineTrace* pTrace);

void EcoSplineNotifyRow(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t row, /* in */ ldouble_t lower, /* in */ ldouble_t diagonal, /* in */ ldouble_t upper, /* in */ ldouble_t rhs);
void EcoSplineNotifyForward(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t row, /* in */ ldouble_t alpha, /* in */ ldouble_t beta);
void EcoSplineNotifySlope(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t node, /* in */ ldouble_t slope);
void EcoSplineNotifyPiece(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t piece, /* in */ ldouble_t a, /* in */ ldouble_t b, /* in */ ldouble_t c, /* in */ ldouble_t d);

#endif /* __C_ECOSPLINEEVENTS_H__ */
