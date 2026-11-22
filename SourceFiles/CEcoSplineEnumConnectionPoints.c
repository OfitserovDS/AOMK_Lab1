/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoSplineEnumConnectionPoints
 * </summary>
 *
 * <description>
 *   This header describes the implementation of the CEcoSplineEnumConnectionPoints component
 * </description>
 *
 * <author>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </author>
 *
 * Adapted for Eco.AdvancedMath from Eco Wizard 0.2.0 (NewProject/CROOTEnumConnectionPoints.c).
 */

#include "CEcoSplineConnectionPoint.h"

typedef struct CEcoSplineEnumConnectionPoints {
    IEcoEnumConnectionPoints m_Interface;
    uint32_t m_cRef;
    uint32_t m_cIndex;
    IEcoConnectionPoint* m_pPoint;
    IEcoMemoryAllocator1* m_pIMem;
} CEcoSplineEnumConnectionPoints;

/*
 *
 * <summary>
 *   QueryInterface Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_QueryInterface(/* in */ IEcoEnumConnectionPointsPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    if (IsEqualUGUID(riid, &IID_IEcoUnknown) || IsEqualUGUID(riid, &IID_IEcoEnumConnectionPoints)) {
        *ppv = me;
        me->pVTbl->AddRef(me);
        return ERR_ECO_SUCCESS;
    }
    return ERR_ECO_NOINTERFACE;
}

/*
 *
 * <summary>
 *   AddRef Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_AddRef(/* in */ IEcoEnumConnectionPointsPtr_t me) {
    CEcoSplineEnumConnectionPoints* pCMe = (CEcoSplineEnumConnectionPoints*)me;
    if (me == 0) {
        return (uint32_t)-1;
    }
    return ++pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   Release Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_Release(/* in */ IEcoEnumConnectionPointsPtr_t me) {
    CEcoSplineEnumConnectionPoints* pCMe = (CEcoSplineEnumConnectionPoints*)me;
    IEcoConnectionPoint* pPoint = 0;

    if (me == 0) {
        return (uint32_t)-1;
    }
    if (--pCMe->m_cRef != 0) {
        return pCMe->m_cRef;
    }
    pPoint = pCMe->m_pPoint;
    pCMe->m_pIMem->pVTbl->Free(pCMe->m_pIMem, pCMe);
    pPoint->pVTbl->Release(pPoint);
    return 0;
}

/*
 *
 * <summary>
 *   Next Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_Next(/* in */ IEcoEnumConnectionPointsPtr_t me, /* in */ uint32_t cConnections, /* out */ IEcoConnectionPointPtr_t* ppCP,
    /* out */ uint32_t* pcFetched) {
    CEcoSplineEnumConnectionPoints* pCMe = (CEcoSplineEnumConnectionPoints*)me;
    uint32_t count = 0;

    if (pcFetched != 0) {
        *pcFetched = 0;
    }
    if (ppCP != 0 && cConnections != 0) {
        ppCP[0] = 0;
    }
    if (me == 0 || ppCP == 0 || (pcFetched == 0 && cConnections > 1)) {
        return ERR_ECO_POINTER;
    }
    if (cConnections != 0 && pCMe->m_cIndex == 0) {
        *ppCP = pCMe->m_pPoint;
        pCMe->m_pPoint->pVTbl->AddRef(pCMe->m_pPoint);
        pCMe->m_cIndex = 1;
        count = 1;
    }
    if (pcFetched != 0) {
        *pcFetched = count;
    }
    if (count < cConnections) {
        ppCP[count] = 0;
        return ERR_ECO_FAIL;
    }
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Skip Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_Skip(/* in */ IEcoEnumConnectionPointsPtr_t me, /* in */ uint32_t cConnections) {
    CEcoSplineEnumConnectionPoints* pCMe = (CEcoSplineEnumConnectionPoints*)me;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    if (cConnections > 1 - pCMe->m_cIndex) {
        pCMe->m_cIndex = 1;
        return ERR_ECO_FAIL;
    }
    pCMe->m_cIndex += cConnections;
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Reset Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_Reset(/* in */ IEcoEnumConnectionPointsPtr_t me) {
    CEcoSplineEnumConnectionPoints* pCMe = (CEcoSplineEnumConnectionPoints*)me;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    pCMe->m_cIndex = 0;
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Clone Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineEnumConnectionPoints_Clone(/* in */ IEcoEnumConnectionPointsPtr_t me, /* out */ IEcoEnumConnectionPointsPtr_t* ppEnum) {
    CEcoSplineEnumConnectionPoints* pCMe = (CEcoSplineEnumConnectionPoints*)me;
    int16_t result = 0;

    if (ppEnum == 0) {
        return ERR_ECO_POINTER;
    }
    *ppEnum = 0;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    result = CEcoSplineEnumConnectionPoints_Create(pCMe->m_pIMem, pCMe->m_pPoint, ppEnum);
    if (result == ERR_ECO_SUCCESS) {
        ((CEcoSplineEnumConnectionPoints*)*ppEnum)->m_cIndex = pCMe->m_cIndex;
    }
    return result;
}

/* Create Virtual Table IEcoEnumConnectionPointsVTbl */
static IEcoEnumConnectionPointsVTbl g_xSplineEnumConnectionPointsVTbl = {
    CEcoSplineEnumConnectionPoints_QueryInterface,
    CEcoSplineEnumConnectionPoints_AddRef,
    CEcoSplineEnumConnectionPoints_Release,
    CEcoSplineEnumConnectionPoints_Next,
    CEcoSplineEnumConnectionPoints_Skip,
    CEcoSplineEnumConnectionPoints_Reset,
    CEcoSplineEnumConnectionPoints_Clone
};

/* The stored point keeps the source and its allocator alive. */
int16_t CEcoSplineEnumConnectionPoints_Create(/* in */ IEcoMemoryAllocator1* pIMem,
    /* in */ IEcoConnectionPointPtr_t pPoint, /* out */ IEcoEnumConnectionPointsPtr_t* ppEnum) {
    CEcoSplineEnumConnectionPoints* pCObj = 0;

    pCObj = (CEcoSplineEnumConnectionPoints*)pIMem->pVTbl->Alloc(pIMem, sizeof(CEcoSplineEnumConnectionPoints));
    if (pCObj == 0) {
        return ERR_ECO_OUTOFMEMORY;
    }
    pPoint->pVTbl->AddRef(pPoint);
    pCObj->m_Interface.pVTbl = &g_xSplineEnumConnectionPointsVTbl;
    pCObj->m_cRef = 1;
    pCObj->m_cIndex = 0;
    pCObj->m_pPoint = pPoint;
    pCObj->m_pIMem = pIMem;
    *ppEnum = &pCObj->m_Interface;
    return ERR_ECO_SUCCESS;
}
