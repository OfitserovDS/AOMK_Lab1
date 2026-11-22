/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   Точка подключения и перечислители событий Eco.AdvancedMath.
 * </summary>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 */

#include "CEcoAdvancedMath.h"
#include "CEcoSplineEvents.h"
#include "IEcoAdvancedMathEvents.h"
#include "IEcoEnumConnections.h"

extern CEcoAdvancedMath_4CAA1B2F g_xCEcoAdvancedMath_4CAA1B2F;

#define SOURCE_FROM(me, member) \
    ((CEcoSplineEventSource*)((char_t*)(me) - ((char_t*)&g_xCEcoAdvancedMath_4CAA1B2F.member - (char_t*)&g_xCEcoAdvancedMath_4CAA1B2F)))

static int16_t ECOCALLMETHOD Container_QueryInterface(/* in */ IEcoConnectionPointContainerPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    IEcoUnknown* pOuter = 0;

    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    pOuter = SOURCE_FROM(me, m_ConnectionContainer)->m_pIUnkOuter;
    return pOuter->pVTbl->QueryInterface(pOuter, riid, ppv);
}

static uint32_t ECOCALLMETHOD Container_AddRef(/* in */ IEcoConnectionPointContainerPtr_t me) {
    IEcoUnknown* pOuter = 0;

    if (me == 0) {
        return (uint32_t)-1;
    }
    pOuter = SOURCE_FROM(me, m_ConnectionContainer)->m_pIUnkOuter;
    return pOuter->pVTbl->AddRef(pOuter);
}

static uint32_t ECOCALLMETHOD Container_Release(/* in */ IEcoConnectionPointContainerPtr_t me) {
    IEcoUnknown* pOuter = 0;

    if (me == 0) {
        return (uint32_t)-1;
    }
    pOuter = SOURCE_FROM(me, m_ConnectionContainer)->m_pIUnkOuter;
    return pOuter->pVTbl->Release(pOuter);
}

static int16_t ECOCALLMETHOD Container_EnumPoints(/* in */ IEcoConnectionPointContainerPtr_t me, /* out */ IEcoEnumConnectionPointsPtr_t* ppEnum) {
    CEcoSplineEventSource* pSource = 0;

    if (ppEnum == 0) {
        return ERR_ECO_POINTER;
    }
    *ppEnum = 0;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    pSource = SOURCE_FROM(me, m_ConnectionContainer);
    return CEcoSplineEnumConnectionPoints_Create(pSource->m_pIMem,
        &pSource->m_ConnectionPoint.m_Interface, ppEnum);
}

static int16_t ECOCALLMETHOD Container_FindPoint(/* in */ IEcoConnectionPointContainerPtr_t me, /* in */ const UGUID* riid, /* out */ IEcoConnectionPointPtr_t* ppPoint) {
    if (ppPoint == 0) {
        return ERR_ECO_POINTER;
    }
    *ppPoint = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    if (!IsEqualUGUID(riid, &IID_IEcoAdvancedMathEvents)) {
        return ERR_ECO_NOINTERFACE;
    }
    *ppPoint = &SOURCE_FROM(me, m_ConnectionContainer)->m_ConnectionPoint.m_Interface;
    (*ppPoint)->pVTbl->AddRef(*ppPoint);
    return ERR_ECO_SUCCESS;
}

static IEcoConnectionPointContainerVTbl g_xSplineContainerVTbl = {
    Container_QueryInterface, Container_AddRef, Container_Release, Container_EnumPoints, Container_FindPoint
};

void EcoSplineInitConnections(/* in */ CEcoSplineEventSource* pSource) {
    pSource->m_ConnectionContainer.pVTbl = &g_xSplineContainerVTbl;
    CEcoSplineConnectionPoint_Init(&pSource->m_ConnectionPoint, &pSource->m_ConnectionContainer,
        pSource->m_pIMem);
}

void EcoSplineClearConnections(/* in */ CEcoSplineEventSource* pSource) {
    CEcoSplineConnectionPoint_Clear(&pSource->m_ConnectionPoint);
}

int16_t EcoSplineBeginTrace(/* in */ CEcoSplineEventSource* pSource, /* out */ CEcoSplineTrace* pTrace) {
    IEcoConnectionPoint* pPoint = &pSource->m_ConnectionPoint.m_Interface;
    int16_t result = ERR_ECO_SUCCESS;

    pTrace->pEnum = 0;
    pTrace->pSource = 0;
    if (pSource->m_ConnectionPoint.m_pSinkList != 0) {
        result = pPoint->pVTbl->EnumConnections(pPoint, &pTrace->pEnum);
        if (result == ERR_ECO_SUCCESS) {
            pTrace->pSource = (IEcoAdvancedMath*)&pSource->m_pVTblIEcoAdvancedMath;
            pTrace->pSource->pVTbl->AddRef(pTrace->pSource);
        }
    }
    return result;
}

void EcoSplineEndTrace(/* in */ CEcoSplineTrace* pTrace) {
    if (pTrace->pEnum != 0) {
        pTrace->pEnum->pVTbl->Release(pTrace->pEnum);
        pTrace->pSource->pVTbl->Release(pTrace->pSource);
    }
}

/* A successful Next owns the returned Unknown; QueryInterface owns the callback interface. */
static int16_t NextSink(/* in */ IEcoEnumConnections* pEnum, /* out */ IEcoAdvancedMathEvents** ppEvents) {
    EcoConnectionData connection;
    int16_t result = 0;

    *ppEvents = 0;
    result = pEnum->pVTbl->Next(pEnum, 1, &connection, 0);
    if (result == ERR_ECO_SUCCESS) {
        if (connection.pUnk->pVTbl->QueryInterface(connection.pUnk,
            &IID_IEcoAdvancedMathEvents, (voidptr_t*)ppEvents) != ERR_ECO_SUCCESS) {
            *ppEvents = 0;
        }
        connection.pUnk->pVTbl->Release(connection.pUnk);
    }
    return result;
}

void EcoSplineNotifyRow(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t row, /* in */ ldouble_t lower, /* in */ ldouble_t diagonal, /* in */ ldouble_t upper, /* in */ ldouble_t rhs) {
    IEcoAdvancedMathEvents* pEvents = 0;
    IEcoEnumConnections* pEnum = pTrace->pEnum;

    pEnum->pVTbl->Reset(pEnum);
    while (NextSink(pEnum, &pEvents) == ERR_ECO_SUCCESS) {
        if (pEvents != 0) {
            pEvents->pVTbl->OnSystemRow(pEvents, pTrace->pSource, row, lower, diagonal, upper, rhs);
            pEvents->pVTbl->Release(pEvents);
        }
    }
}

void EcoSplineNotifyForward(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t row, /* in */ ldouble_t alpha, /* in */ ldouble_t beta) {
    IEcoAdvancedMathEvents* pEvents = 0;
    IEcoEnumConnections* pEnum = pTrace->pEnum;

    pEnum->pVTbl->Reset(pEnum);
    while (NextSink(pEnum, &pEvents) == ERR_ECO_SUCCESS) {
        if (pEvents != 0) {
            pEvents->pVTbl->OnForwardStep(pEvents, pTrace->pSource, row, alpha, beta);
            pEvents->pVTbl->Release(pEvents);
        }
    }
}

void EcoSplineNotifySlope(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t node, /* in */ ldouble_t slope) {
    IEcoAdvancedMathEvents* pEvents = 0;
    IEcoEnumConnections* pEnum = pTrace->pEnum;

    pEnum->pVTbl->Reset(pEnum);
    while (NextSink(pEnum, &pEvents) == ERR_ECO_SUCCESS) {
        if (pEvents != 0) {
            pEvents->pVTbl->OnSlopeReady(pEvents, pTrace->pSource, node, slope);
            pEvents->pVTbl->Release(pEvents);
        }
    }
}

void EcoSplineNotifyPiece(/* in */ CEcoSplineTrace* pTrace, /* in */ uint32_t piece, /* in */ ldouble_t a, /* in */ ldouble_t b, /* in */ ldouble_t c, /* in */ ldouble_t d) {
    IEcoAdvancedMathEvents* pEvents = 0;
    IEcoEnumConnections* pEnum = pTrace->pEnum;

    pEnum->pVTbl->Reset(pEnum);
    while (NextSink(pEnum, &pEvents) == ERR_ECO_SUCCESS) {
        if (pEvents != 0) {
            pEvents->pVTbl->OnPieceReady(pEvents, pTrace->pSource, piece, a, b, c, d);
            pEvents->pVTbl->Release(pEvents);
        }
    }
}
