/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoLab2_2F9385F6Factory
 * </summary>
 *
 * <description>
 *   This source code describes the implementation of the factory for the component
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#include "IEcoSystem1.h"
#include "IEcoInterfaceBus1.h"
#include "IEcoInterfaceBus1MemExt.h"

#include "CEcoLab2.h"
#include "CEcoLab2Factory.h"

extern CEcoLab2_2F9385F6 g_xCEcoLab2_2F9385F6;

/*
 *
 * <summary>
 *   QueryInterface function
 * </summary>
 *
 * <description>
 *   The function returns a pointer to the interface
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6Factory_QueryInterface(IEcoComponentFactory* me, const UGUID* riid, void** ppv) {

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }

    if ( IsEqualUGUID(riid, &IID_IEcoUnknown) || IsEqualUGUID(riid, &IID_IEcoComponentFactory) ) {
        *ppv = me;
    }
    else {
        *ppv = 0;
        return ERR_ECO_NOINTERFACE;
    }
    ((IEcoUnknown*)(*ppv))->pVTbl->AddRef((IEcoUnknown*)*ppv);

    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   AddRef function
 * </summary>
 *
 * <description>
 *   The function increments the reference count for the interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6Factory_AddRef(/* in */ IEcoComponentFactory* me) {
    CEcoLab2_2F9385F6Factory* pCMe = (CEcoLab2_2F9385F6Factory*)me;

    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }

    return ++pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   Release function
 * </summary>
 *
 * <description>
 *   The function decrements the reference count for the interface
 *   (фабрика - статический объект, память не освобождается)
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6Factory_Release(/* in */ IEcoComponentFactory* me) {
    CEcoLab2_2F9385F6Factory* pCMe = (CEcoLab2_2F9385F6Factory*)me;

    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }

    /* Decrementing the component reference counter */
    if ( pCMe->m_cRef > 0 ) {
        --pCMe->m_cRef;
    }
    return pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   Init function
 * </summary>
 *
 * <description>
 *   The function initializes the component with parameters (параметры не требуются)
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6Factory_Init(/* in */ struct IEcoComponentFactory* me, /* in */ struct IEcoUnknown *pIUnkSystem, /* in */ void* pv) {

    /* Parameters are not used */
    (void)pIUnkSystem;
    (void)pv;

    if (me == 0 ) {
        return ERR_ECO_POINTER;
    }

    /* Initializing the component with parameters */

    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Alloc function
 * </summary>
 *
 * <description>
 *   The function creates a component.
 *   Все интерфейсы, полученные фабрикой (система, шина, распределитель памяти),
 *   освобождаются перед возвратом; экземпляр компонента получает собственные
 *   ссылки в своей функции Init. Агрегирование компонентом не поддерживается.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6Factory_Alloc(/* in */ struct IEcoComponentFactory* me, /* in */ struct IEcoUnknown *pISystem, /* in */ struct IEcoUnknown *pIUnknownOuter, /* in */ const UGUID* riid, /* out */ void** ppv) {
    IEcoUnknown* pIUnk = 0;
    int16_t result = ERR_ECO_POINTER;
    IEcoSystem1* pISys = 0;
    IEcoInterfaceBus1* pIBus = 0;
    IEcoInterfaceBus1MemExt* pIMemExt = 0;
    IEcoMemoryAllocator1* pIMem = 0;
    CEcoLab2_2F9385F6* pCObj = 0;
    const UGUID* rcid = &CID_EcoMemoryManager1;

    if (me == 0 || pISystem == 0 || riid == 0 || ppv == 0) {
        return result; /* ERR_ECO_POINTER */
    }
    *ppv = 0;

    /* Aggregation is not supported */
    if ( pIUnknownOuter != 0 ) {
        return ERR_ECO_NOAGGREGATION;
    }

    /* Getting the application system interface */
    result = pISystem->pVTbl->QueryInterface(pISystem, &GID_IEcoSystem, (void **)&pISys);
    /* Check */
    if (result != 0 || pISys == 0) {
        return ERR_ECO_NOSYSTEM;
    }

    /* Getting the interface for working with the interface bus */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);
    /* Check */
    if (result != 0 || pIBus == 0) {
        pIBus = 0;
        result = ERR_ECO_NOBUS;
        goto Release;
    }

    /* Getting the component ID for memory operations */
    result = pIBus->pVTbl->QueryInterface(pIBus, &IID_IEcoInterfaceBus1MemExt, (void**)&pIMemExt);
    if (result == 0 && pIMemExt != 0) {
        rcid = pIMemExt->pVTbl->get_Manager(pIMemExt);
        pIMemExt->pVTbl->Release(pIMemExt);
    }

    /* Getting the memory allocator interface */
    result = pIBus->pVTbl->QueryComponent(pIBus, rcid, 0, &IID_IEcoMemoryAllocator1, (void**) &pIMem);
    /* Check */
    if (result != 0 || pIMem == 0) {
        pIMem = 0;
        result = ERR_ECO_GET_MEMORY_ALLOCATOR;
        goto Release;
    }

    /* Allocating memory for instance data */
    pCObj = (CEcoLab2_2F9385F6*)pIMem->pVTbl->Alloc(pIMem, sizeof(CEcoLab2_2F9385F6));
    if (pCObj == 0) {
        result = ERR_ECO_OUTOFMEMORY;
        goto Release;
    }

    /* Forming instance data */
    pCObj = (CEcoLab2_2F9385F6*)pIMem->pVTbl->Copy(pIMem, pCObj, &g_xCEcoLab2_2F9385F6, sizeof(CEcoLab2_2F9385F6));

    /* Component creation and initialization */
    result = pCObj->Create(pCObj, pISystem, pIUnknownOuter);
    if (result == ERR_ECO_SUCCESS) {
        result = pCObj->Init(pCObj, pISystem);
    }
    if (result != ERR_ECO_SUCCESS) {
        /* Freeing a partially initialized instance */
        if (pCObj->m_pISys != 0) {
            pCObj->m_pISys->pVTbl->Release(pCObj->m_pISys);
        }
        if (pCObj->m_pIMem != 0) {
            pCObj->m_pIMem->pVTbl->Release(pCObj->m_pIMem);
        }
        pIMem->pVTbl->Free(pIMem, pCObj);
        goto Release;
    }

    /* Getting a pointer to the interface */
    pIUnk = (IEcoUnknown*)pCObj;
    result = pIUnk->pVTbl->QueryInterface(pIUnk, riid, ppv);

    /* Decrementing the reference requested by the Component Factory
       (if QueryInterface failed, the instance is deleted here) */
    pIUnk->pVTbl->Release(pIUnk);

Release:
    /* Freeing the interfaces obtained by the factory */
    if (pIMem != 0) {
        pIMem->pVTbl->Release(pIMem);
    }
    if (pIBus != 0) {
        pIBus->pVTbl->Release(pIBus);
    }
    pISys->pVTbl->Release(pISys);

    return result;
}

/*
 *
 * <summary>
 *   get_Name function
 * </summary>
 *
 * <description>
 *   The function returns the name of the component
 * </description>
 *
 */
static char_t* ECOCALLMETHOD CEcoLab2_2F9385F6Factory_get_Name(/* in */ struct IEcoComponentFactory* me) {
    CEcoLab2_2F9385F6Factory* pCMe = (CEcoLab2_2F9385F6Factory*)me;

    if (me == 0 ) {
        return 0; /* ERR_ECO_POINTER */
    }

    return pCMe->m_Name;
}

/*
 *
 * <summary>
 *   get_Version function
 * </summary>
 *
 * <description>
 *   The function returns the version of the component
 * </description>
 *
 */
static char_t* ECOCALLMETHOD CEcoLab2_2F9385F6Factory_get_Version(/* in */ struct IEcoComponentFactory* me) {
    CEcoLab2_2F9385F6Factory* pCMe = (CEcoLab2_2F9385F6Factory*)me;

    if (me == 0 ) {
        return 0; /* ERR_ECO_POINTER */
    }

    return pCMe->m_Version;
}

/*
 *
 * <summary>
 *   get_Manufacturer function
 * </summary>
 *
 * <description>
 *   The function returns the name of the component manufacturer
 * </description>
 *
 */
static char_t* ECOCALLMETHOD CEcoLab2_2F9385F6Factory_get_Manufacturer(/* in */ struct IEcoComponentFactory* me) {
    CEcoLab2_2F9385F6Factory* pCMe = (CEcoLab2_2F9385F6Factory*)me;

    if (me == 0 ) {
        return 0; /* ERR_ECO_POINTER */
    }

    return pCMe->m_Manufacturer;
}

/* Create Virtual Table */
IEcoComponentFactoryVTbl g_x1EF402B8755C4289BBE1795A2F9385F6FactoryVTbl = {
    CEcoLab2_2F9385F6Factory_QueryInterface,
    CEcoLab2_2F9385F6Factory_AddRef,
    CEcoLab2_2F9385F6Factory_Release,
    CEcoLab2_2F9385F6Factory_Alloc,
    CEcoLab2_2F9385F6Factory_Init,
    CEcoLab2_2F9385F6Factory_get_Name,
    CEcoLab2_2F9385F6Factory_get_Version,
    CEcoLab2_2F9385F6Factory_get_Manufacturer
};

/*
 *
 * <summary>
 *   Factory instance
 * </summary>
 *
 * <description>
 *   Статический экземпляр фабрики компонента
 * </description>
 *
 */
CEcoLab2_2F9385F6Factory g_x1EF402B8755C4289BBE1795A2F9385F6Factory = {
    &g_x1EF402B8755C4289BBE1795A2F9385F6FactoryVTbl,
    0,
    "EcoLab2\0",
    "1.0.0.0\0",
    "Офицеров Д.С.\0"
};

#ifdef ECO_DLL
ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr() {
    return (IEcoComponentFactory*)&g_x1EF402B8755C4289BBE1795A2F9385F6Factory;
}
#elif ECO_LIB
IEcoComponentFactory* GetIEcoComponentFactoryPtr_1EF402B8755C4289BBE1795A2F9385F6 = (IEcoComponentFactory*)&g_x1EF402B8755C4289BBE1795A2F9385F6Factory;
#endif
