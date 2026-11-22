/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   CEcoSplineConnectionPoint
 * </сводка>
 *
 * <описание>
 *   Данный исходный код описывает реализацию интерфейсов CEcoSplineConnectionPoint
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2016 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 * Adapted for Eco.AdvancedMath from Eco Wizard 0.2.0 (NewProject/CROOTConnectionPoint.c).
 */

#include "CEcoSplineConnectionPoint.h"
#include "IEcoAdvancedMathEvents.h"

static void DeleteSubscription(/* in */ IEcoMemoryAllocator1* pIMem, /* in */ CEcoSplineSubscription* pItem) {
    IEcoUnknown* pSink = pItem->m_Connection.pUnk;

    pIMem->pVTbl->Free(pIMem, pItem);
    pSink->pVTbl->Release(pSink);
}

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
static int16_t ECOCALLMETHOD CEcoSplineConnectionPoint_QueryInterface(/* in */ IEcoConnectionPointPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    if (IsEqualUGUID(riid, &IID_IEcoUnknown) || IsEqualUGUID(riid, &IID_IEcoConnectionPoint)) {
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
static uint32_t ECOCALLMETHOD CEcoSplineConnectionPoint_AddRef(/* in */ IEcoConnectionPointPtr_t me) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    if (me == 0) {
        return (uint32_t)-1;
    }
    return pCMe->m_pICPC->pVTbl->AddRef(pCMe->m_pICPC);
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
static uint32_t ECOCALLMETHOD CEcoSplineConnectionPoint_Release(/* in */ IEcoConnectionPointPtr_t me) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    if (me == 0) {
        return (uint32_t)-1;
    }
    return pCMe->m_pICPC->pVTbl->Release(pCMe->m_pICPC);
}

/*
 *
 * <summary>
 *   GetConnectionInterface Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineConnectionPoint_GetConnectionInterface(/* in */ IEcoConnectionPointPtr_t me, /* out */ UGUID* pIID) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    static const UGUID emptyIID = {0, 0, {0}};

    if (pIID == 0) {
        return ERR_ECO_POINTER;
    }
    if (me == 0) {
        *pIID = emptyIID;
        return ERR_ECO_POINTER;
    }
    *pIID = *pCMe->m_piid;
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   GetConnectionPointContainer Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineConnectionPoint_GetConnectionPointContainer(/* in */ IEcoConnectionPointPtr_t me, /* out */ IEcoConnectionPointContainerPtr_t* ppICPC) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    if (ppICPC == 0) {
        return ERR_ECO_POINTER;
    }
    *ppICPC = 0;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    *ppICPC = pCMe->m_pICPC;
    pCMe->m_pICPC->pVTbl->AddRef(pCMe->m_pICPC);
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Advise Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineConnectionPoint_Advise(/* in */ IEcoConnectionPointPtr_t me, /* in */ IEcoUnknownPtr_t pUnkSink, /* out */ uint32_t* pcCookie) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    CEcoSplineSubscription* pItem = 0;
    IEcoAdvancedMathEvents* pEvents = 0;
    int16_t result = 0;

    if (pcCookie == 0) {
        return ERR_ECO_POINTER;
    }
    *pcCookie = 0;
    if (me == 0 || pUnkSink == 0) {
        return ERR_ECO_POINTER;
    }
    result = pUnkSink->pVTbl->QueryInterface(pUnkSink, pCMe->m_piid, (voidptr_t*)&pEvents);
    if (result != ERR_ECO_SUCCESS || pEvents == 0) {
        return ERR_ECO_NOINTERFACE;
    }
    pEvents->pVTbl->Release(pEvents);

    if (pCMe->m_cNextCookie == (uint32_t)-1) {
        return ERR_ECO_FAIL;
    }
    pItem = (CEcoSplineSubscription*)pCMe->m_pIMem->pVTbl->Alloc(pCMe->m_pIMem, sizeof(CEcoSplineSubscription));
    if (pItem == 0) {
        return ERR_ECO_OUTOFMEMORY;
    }
    pUnkSink->pVTbl->AddRef(pUnkSink);
    pItem->m_Connection.pUnk = pUnkSink;
    pItem->m_Connection.cCookie = ++pCMe->m_cNextCookie;
    pItem->m_pNext = pCMe->m_pSinkList;
    pCMe->m_pSinkList = pItem;
    *pcCookie = pItem->m_Connection.cCookie;
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Unadvise Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineConnectionPoint_Unadvise(/* in */ IEcoConnectionPointPtr_t me, /* in */ uint32_t cCookie) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    CEcoSplineSubscription** ppItem = 0;
    CEcoSplineSubscription* pItem = 0;

    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    for (ppItem = &pCMe->m_pSinkList; *ppItem != 0; ppItem = &pItem->m_pNext) {
        pItem = *ppItem;
        if (pItem->m_Connection.cCookie == cCookie) {
            *ppItem = pItem->m_pNext;
            DeleteSubscription(pCMe->m_pIMem, pItem);
            return ERR_ECO_SUCCESS;
        }
    }
    return ERR_ECO_FAIL;
}

/*
 *
 * <summary>
 *   EnumConnections Function
 * </summary>
 *
 * <description>
 *
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSplineConnectionPoint_EnumConnections(/* in */ IEcoConnectionPointPtr_t me, /* out */ IEcoEnumConnectionsPtr_t* ppEnum) {
    CEcoSplineConnectionPoint* pCMe = (CEcoSplineConnectionPoint*)me;
    if (ppEnum == 0) {
        return ERR_ECO_POINTER;
    }
    *ppEnum = 0;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }
    return CEcoSplineEnumConnections_Create(pCMe->m_pIMem, pCMe->m_pSinkList, ppEnum);
}

/* Create Virtual Table IEcoConnectionPointVTbl */
static IEcoConnectionPointVTbl g_xSplineConnectionPointVTbl = {
    CEcoSplineConnectionPoint_QueryInterface,
    CEcoSplineConnectionPoint_AddRef,
    CEcoSplineConnectionPoint_Release,
    CEcoSplineConnectionPoint_GetConnectionInterface,
    CEcoSplineConnectionPoint_GetConnectionPointContainer,
    CEcoSplineConnectionPoint_Advise,
    CEcoSplineConnectionPoint_Unadvise,
    CEcoSplineConnectionPoint_EnumConnections
};

/* The point is embedded in the source and borrows its container and allocator. */
void CEcoSplineConnectionPoint_Init(/* in */ CEcoSplineConnectionPoint* pCMe,
    /* in */ IEcoConnectionPointContainer* pICPC, /* in */ IEcoMemoryAllocator1* pIMem) {
    pCMe->m_Interface.pVTbl = &g_xSplineConnectionPointVTbl;
    pCMe->m_pICPC = pICPC;
    pCMe->m_piid = &IID_IEcoAdvancedMathEvents;
    pCMe->m_cNextCookie = 0;
    pCMe->m_pSinkList = 0;
    pCMe->m_pIMem = pIMem;
}

void CEcoSplineConnectionPoint_Clear(/* in */ CEcoSplineConnectionPoint* pCMe) {
    CEcoSplineSubscription* pItem = 0;

    while (pCMe->m_pSinkList != 0) {
        pItem = pCMe->m_pSinkList;
        pCMe->m_pSinkList = pItem->m_pNext;
        DeleteSubscription(pCMe->m_pIMem, pItem);
    }
}
