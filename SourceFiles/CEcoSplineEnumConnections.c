/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoSplineEnumConnections
 * </summary>
 *
 * <description>
 *   This header describes the implementation of the CEcoSplineEnumConnections component
 * </description>
 *
 * <author>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </author>
 *
 * Adapted for Eco.AdvancedMath from Eco Wizard 0.2.0 (NewProject/CROOTEnumConnections.c).
 */

#include "CEcoSplineConnectionPoint.h"

typedef struct CEcoSplineEnumConnections {
    IEcoEnumConnections m_Interface;
    uint32_t m_cRef;
    EcoConnectionData* m_pSinkList;
    uint32_t m_cIndex;
    uint32_t m_cCount;
    IEcoMemoryAllocator1* m_pIMem;
} CEcoSplineEnumConnections;

static CEcoSplineEnumConnections* NewSnapshot(/* in */ IEcoMemoryAllocator1* pIMem, /* in */ uint32_t count);

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
static int16_t ECOCALLMETHOD CEcoSplineEnumConnections_QueryInterface(/* in */ IEcoEnumConnectionsPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    if (IsEqualUGUID(riid, &IID_IEcoUnknown) || IsEqualUGUID(riid, &IID_IEcoEnumConnections)) {
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
static uint32_t ECOCALLMETHOD CEcoSplineEnumConnections_AddRef(/* in */ IEcoEnumConnectionsPtr_t me) {
    CEcoSplineEnumConnections* pCMe = (CEcoSplineEnumConnections*)me;
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
static uint32_t ECOCALLMETHOD CEcoSplineEnumConnections_Release(/* in */ IEcoEnumConnectionsPtr_t me) {
    CEcoSplineEnumConnections* pCMe = (CEcoSplineEnumConnections*)me;
    IEcoMemoryAllocator1* pIMem = 0;
    IEcoUnknown* pSink = 0;
    uint32_t i = 0;

    if (me == 0) {
        return (uint32_t)-1;
    }
    if (--pCMe->m_cRef != 0) {
        return pCMe->m_cRef;
    }
    pIMem = pCMe->m_pIMem;
    for (i = 0; i < pCMe->m_cCount; ++i) {
        pSink = pCMe->m_pSinkList[i].pUnk;
        pSink->pVTbl->Release(pSink);
    }
    pIMem->pVTbl->Free(pIMem, pCMe);
    pIMem->pVTbl->Release(pIMem);
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
static int16_t ECOCALLMETHOD CEcoSplineEnumConnections_Next(/* in */ IEcoEnumConnectionsPtr_t me, /* in */ uint32_t cConnections, /* out */ EcoConnectionData* rgcd,
    /* out */ uint32_t* pcFetched) {
    CEcoSplineEnumConnections* pCMe = (CEcoSplineEnumConnections*)me;
    uint32_t count = 0;
    IEcoUnknown* pSink = 0;

    if (pcFetched != 0) {
        *pcFetched = 0;
    }
    if (rgcd != 0 && cConnections != 0) {
        rgcd[0].pUnk = 0;
        rgcd[0].cCookie = 0;
    }
    if (me == 0 || rgcd == 0 || (pcFetched == 0 && cConnections > 1)) {
        return ERR_ECO_POINTER;
    }
    while (pCMe->m_cIndex < pCMe->m_cCount && count < cConnections) {
        rgcd[count] = pCMe->m_pSinkList[pCMe->m_cIndex];
        pSink = rgcd[count].pUnk;
        pSink->pVTbl->AddRef(pSink);
        ++pCMe->m_cIndex;
        ++count;
    }
    if (pcFetched != 0) {
        *pcFetched = count;
    }
    if (count < cConnections) {
        rgcd[count].pUnk = 0;
        rgcd[count].cCookie = 0;
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
static int16_t ECOCALLMETHOD CEcoSplineEnumConnections_Skip(/* in */ IEcoEnumConnectionsPtr_t me, /* in */ uint32_t cConnections) {
    CEcoSplineEnumConnections* pCMe = (CEcoSplineEnumConnections*)me;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    if (cConnections > pCMe->m_cCount - pCMe->m_cIndex) {
        pCMe->m_cIndex = pCMe->m_cCount;
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
static int16_t ECOCALLMETHOD CEcoSplineEnumConnections_Reset(/* in */ IEcoEnumConnectionsPtr_t me) {
    CEcoSplineEnumConnections* pCMe = (CEcoSplineEnumConnections*)me;
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
static int16_t ECOCALLMETHOD CEcoSplineEnumConnections_Clone(/* in */ IEcoEnumConnectionsPtr_t me, /* out */ IEcoEnumConnectionsPtr_t* ppEnum) {
    CEcoSplineEnumConnections* pCMe = (CEcoSplineEnumConnections*)me;
    CEcoSplineEnumConnections* pCObj = 0;
    IEcoUnknown* pSink = 0;
    uint32_t i = 0;

    if (ppEnum == 0) {
        return ERR_ECO_POINTER;
    }
    *ppEnum = 0;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    pCObj = NewSnapshot(pCMe->m_pIMem, pCMe->m_cCount);
    if (pCObj == 0) {
        return ERR_ECO_OUTOFMEMORY;
    }
    pCObj->m_cIndex = pCMe->m_cIndex;
    for (i = 0; i < pCObj->m_cCount; ++i) {
        pCObj->m_pSinkList[i] = pCMe->m_pSinkList[i];
        pSink = pCObj->m_pSinkList[i].pUnk;
        pSink->pVTbl->AddRef(pSink);
    }
    *ppEnum = &pCObj->m_Interface;
    return ERR_ECO_SUCCESS;
}

/* Create Virtual Table IEcoEnumConnectionsVTbl */
static IEcoEnumConnectionsVTbl g_xSplineEnumConnectionsVTbl = {
    CEcoSplineEnumConnections_QueryInterface,
    CEcoSplineEnumConnections_AddRef,
    CEcoSplineEnumConnections_Release,
    CEcoSplineEnumConnections_Next,
    CEcoSplineEnumConnections_Skip,
    CEcoSplineEnumConnections_Reset,
    CEcoSplineEnumConnections_Clone
};

/* The snapshot owns its allocator and one reference to each sink. */
static CEcoSplineEnumConnections* NewSnapshot(/* in */ IEcoMemoryAllocator1* pIMem, /* in */ uint32_t count) {
    CEcoSplineEnumConnections* pCObj = 0;
    uint32_t bytes = 0;

    if (count > ((uint32_t)-1 - sizeof(CEcoSplineEnumConnections)) / sizeof(EcoConnectionData)) {
        return 0;
    }
    bytes = (uint32_t)(sizeof(CEcoSplineEnumConnections) + count * sizeof(EcoConnectionData));
    pCObj = (CEcoSplineEnumConnections*)pIMem->pVTbl->Alloc(pIMem, bytes);
    if (pCObj != 0) {
        pIMem->pVTbl->AddRef(pIMem);
        pCObj->m_Interface.pVTbl = &g_xSplineEnumConnectionsVTbl;
        pCObj->m_cRef = 1;
        pCObj->m_pSinkList = (EcoConnectionData*)(pCObj + 1);
        pCObj->m_cIndex = 0;
        pCObj->m_cCount = count;
        pCObj->m_pIMem = pIMem;
    }
    return pCObj;
}

int16_t CEcoSplineEnumConnections_Create(/* in */ IEcoMemoryAllocator1* pIMem,
    /* in */ const CEcoSplineSubscription* pList, /* out */ IEcoEnumConnectionsPtr_t* ppEnum) {
    const CEcoSplineSubscription* pItem = 0;
    CEcoSplineEnumConnections* pCObj = 0;
    IEcoUnknown* pSink = 0;
    uint32_t count = 0;
    uint32_t i = 0;

    for (pItem = pList; pItem != 0; pItem = pItem->m_pNext) {
        ++count;
    }
    pCObj = NewSnapshot(pIMem, count);
    if (pCObj == 0) {
        return ERR_ECO_OUTOFMEMORY;
    }
    for (pItem = pList; pItem != 0; pItem = pItem->m_pNext) {
        pCObj->m_pSinkList[i++] = pItem->m_Connection;
        pSink = pItem->m_Connection.pUnk;
        pSink->pVTbl->AddRef(pSink);
    }
    *ppEnum = &pCObj->m_Interface;
    return ERR_ECO_SUCCESS;
}
