/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoSplineConnectionPoint
 * </summary>
 *
 * <description>
 *   This header describes the implementation of the CEcoSplineConnectionPoint component
 * </description>
 *
 * <author>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </author>
 *
 * Adapted from Eco Wizard 0.2.0 (NewProject/CROOTConnectionPoint.h).
 *
 */

#ifndef __C_ECO_SPLINE_CONNECTION_POINT_H__
#define __C_ECO_SPLINE_CONNECTION_POINT_H__

#include "IEcoConnectionPoint.h"
#include "IEcoConnectionPointContainer.h"
#include "IEcoEnumConnectionPoints.h"
#include "IEcoEnumConnections.h"
#include "IEcoMemoryAllocator1.h"

typedef struct CEcoSplineSubscription {
    EcoConnectionData m_Connection;
    struct CEcoSplineSubscription* m_pNext;
} CEcoSplineSubscription;

typedef struct CEcoSplineConnectionPoint {
    IEcoConnectionPoint m_Interface;
    IEcoConnectionPointContainer* m_pICPC;
    const UGUID* m_piid;
    uint32_t m_cNextCookie;
    CEcoSplineSubscription* m_pSinkList;
    IEcoMemoryAllocator1* m_pIMem;
} CEcoSplineConnectionPoint;

void CEcoSplineConnectionPoint_Init(/* in */ CEcoSplineConnectionPoint* pCMe,
    /* in */ IEcoConnectionPointContainer* pICPC, /* in */ IEcoMemoryAllocator1* pIMem);
void CEcoSplineConnectionPoint_Clear(/* in */ CEcoSplineConnectionPoint* pCMe);

int16_t CEcoSplineEnumConnectionPoints_Create(/* in */ IEcoMemoryAllocator1* pIMem,
    /* in */ IEcoConnectionPointPtr_t pPoint, /* out */ IEcoEnumConnectionPointsPtr_t* ppEnum);
int16_t CEcoSplineEnumConnections_Create(/* in */ IEcoMemoryAllocator1* pIMem,
    /* in */ const CEcoSplineSubscription* pList, /* out */ IEcoEnumConnectionsPtr_t* ppEnum);

#endif /* __C_ECO_SPLINE_CONNECTION_POINT_H__ */
