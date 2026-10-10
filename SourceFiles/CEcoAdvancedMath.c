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
 *   This source code describes the implementation of the interfaces for CEcoAdvancedMath_4CAA1B2F.
 *   Компонент реализует интерфейс IEcoAdvancedMath: кубическая сплайн-интерполяция
 *   (аналог spline MATLAB) с решением трехдиагональной СЛАУ методом прогонки.
 *   Перегрузки spline / splinef / splinel получены из одного обобщенного ядра
 *   CEcoSplineCore.h для типов double_t, float_t и ldouble_t.
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
#include "CEcoAdvancedMath.h"
#include "CEcoSpline.h"

/* Обобщенное ядро алгоритма: все разделы для трех вещественных типов */
#define ECO_SPLINE_CORE_SOLVER
#define ECO_SPLINE_CORE_EVAL
#define ECO_SPLINE_CORE_DRIVER

/* float_t: функции с суффиксом _f */
#define ECO_SPLINE_REAL     float_t
#define ECO_SPLINE_FN(name) name##_f
#include "CEcoSplineCore.h"
#undef ECO_SPLINE_FN
#undef ECO_SPLINE_REAL

/* double_t: функции с суффиксом _d */
#define ECO_SPLINE_REAL     double_t
#define ECO_SPLINE_FN(name) name##_d
#include "CEcoSplineCore.h"
#undef ECO_SPLINE_FN
#undef ECO_SPLINE_REAL

/* ldouble_t (long double): функции с суффиксом _l */
#define ECO_SPLINE_REAL     ldouble_t
#define ECO_SPLINE_FN(name) name##_l
#include "CEcoSplineCore.h"
#undef ECO_SPLINE_FN
#undef ECO_SPLINE_REAL

/*
 *
 * <summary>
 *   QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface function for the IEcoAdvancedMath interface
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_QueryInterface(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }

    /* Validate and retrieve requested interface */
    if ( IsEqualUGUID(riid, &IID_IEcoAdvancedMath) ) {
        *ppv = &pCMe->m_pVTblIEcoAdvancedMath;
        pCMe->m_pVTblIEcoAdvancedMath->AddRef((IEcoAdvancedMath*)pCMe);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoUnknown) ) {
        *ppv = &pCMe->m_pVTblIEcoAdvancedMath;
        pCMe->m_pVTblIEcoAdvancedMath->AddRef((IEcoAdvancedMath*)pCMe);
    }
    else {
        *ppv = 0;
        return ERR_ECO_NOINTERFACE;
    }
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef function for the IEcoAdvancedMath interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_AddRef(/* in */ IEcoAdvancedMathPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
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
 *   Release function for the IEcoAdvancedMath interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_Release(/* in */ IEcoAdvancedMathPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }

    /* Decrementing the component's reference count */
    --pCMe->m_cRef;
    /* If the count is zero, free the instance data */
    if ( pCMe->m_cRef == 0 ) {
        pCMe->Delete(pCMe);
        return 0;
    }
    return pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   spline Function
 * </summary>
 *
 * <description>
 *   yq = spline(x, y, xq) для double: кубический сплайн по n узлам (x, y) с граничными
 *   условиями pEnds (0 - not-a-knot, как в MATLAB), вычисленный в m точках xq.
 *   Возвращает ERR_ECO_SUCCESS, ERR_ECO_POINTER, ERR_ECO_OUTOFMEMORY или ERR_ECOADVANCEDMATH_*.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_spline(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const double_t* xq, /* in */ uint32_t m, /* out */ double_t* yq) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0) {
        return ERR_ECO_POINTER;
    }

    return EcoSplineInterpolate_d(pCMe->m_pIMem, x, y, n, pEnds, xq, m, yq);
}

/*
 *
 * <summary>
 *   splinef Function
 * </summary>
 *
 * <description>
 *   yq = spline(x, y, xq) для float (вычисления в одинарной точности).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_splinef(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const float_t* x, /* in */ const float_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const float_t* xq, /* in */ uint32_t m, /* out */ float_t* yq) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0) {
        return ERR_ECO_POINTER;
    }

    return EcoSplineInterpolate_f(pCMe->m_pIMem, x, y, n, pEnds, xq, m, yq);
}

/*
 *
 * <summary>
 *   splinel Function
 * </summary>
 *
 * <description>
 *   yq = spline(x, y, xq) для long double (вычисления в повышенной точности).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_splinel(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const ldouble_t* x, /* in */ const ldouble_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* in */ const ldouble_t* xq, /* in */ uint32_t m, /* out */ ldouble_t* yq) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0) {
        return ERR_ECO_POINTER;
    }

    return EcoSplineInterpolate_l(pCMe->m_pIMem, x, y, n, pEnds, xq, m, yq);
}

/*
 *
 * <summary>
 *   new_Spline Function
 * </summary>
 *
 * <description>
 *   pp = spline(x, y): строит сплайн и возвращает объект данных IEcoSpline
 *   (кусочно-полиномиальная форма). Объект освобождается клиентом вызовом Release.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_new_Spline(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* out */ IEcoSpline** ppISpline) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 || ppISpline == 0) {
        return ERR_ECO_POINTER;
    }

    return createCEcoSpline_4CAA1B2F(pCMe->m_pIMem, x, y, n, pEnds, ppISpline);
}


/*
 *
 * <summary>
 *   Init Function
 * </summary>
 *
 * <description>
 *   Instance initialization function: получает системный интерфейс (с AddRef),
 *   через интерфейсную шину - распределитель памяти. При ошибке поля, которые не
 *   удалось получить, остаются нулевыми.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD initCEcoAdvancedMath_4CAA1B2F(/*in*/ CEcoAdvancedMath_4CAA1B2FPtr_t me, /* in */ IEcoUnknownPtr_t pIUnkSystem) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;
    IEcoInterfaceBus1* pIBus = 0;
    IEcoInterfaceBus1MemExt* pIMemExt = 0;
    int16_t result = ERR_ECO_POINTER;
    const UGUID* rcid = &CID_EcoMemoryManager1;

    /* Pointer Validation */
    if (me == 0 || pIUnkSystem == 0) {
        return result;
    }

    /* Getting the system interface (reference is released in Delete) */
    result = pIUnkSystem->pVTbl->QueryInterface(pIUnkSystem, &GID_IEcoSystem, (voidptr_t*)&pCMe->m_pISys);
    if (result != 0 || pCMe->m_pISys == 0) {
        pCMe->m_pISys = 0;
        return ERR_ECO_NOSYSTEM;
    }

    /* Getting the interface for working with the interface bus */
    result = pCMe->m_pISys->pVTbl->QueryInterface(pCMe->m_pISys, &IID_IEcoInterfaceBus1, (voidptr_t*)&pIBus);
    if (result != 0 || pIBus == 0) {
        return ERR_ECO_NOBUS;
    }

    /* Getting the component ID for working with memory */
    result = pIBus->pVTbl->QueryInterface(pIBus, &IID_IEcoInterfaceBus1MemExt, (voidptr_t*)&pIMemExt);
    if (result == 0 && pIMemExt != 0) {
        rcid = pIMemExt->pVTbl->get_Manager(pIMemExt);
        pIMemExt->pVTbl->Release(pIMemExt);
    }

    /* Getting the memory allocator interface */
    result = pIBus->pVTbl->QueryComponent(pIBus, rcid, 0, &IID_IEcoMemoryAllocator1, (voidptr_t*)&pCMe->m_pIMem);
    if (result != 0 || pCMe->m_pIMem == 0) {
        pCMe->m_pIMem = 0;
        result = ERR_ECO_GET_MEMORY_ALLOCATOR;
    }

    /* Freeing */
    pIBus->pVTbl->Release(pIBus);

    return result;
}

/*
 *
 * <summary>
 *   Create Function
 * </summary>
 *
 * <description>
 *   Instance creation function (агрегирование не поддерживается, дополнительных
 *   действий не требуется)
 * </description>
 *
 */
static int16_t ECOCALLMETHOD createCEcoAdvancedMath_4CAA1B2F(/* in */ CEcoAdvancedMath_4CAA1B2FPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter) {

    /* Parameters are not used: the component does not support aggregation */
    (void)pIUnkSystem;
    (void)pIUnkOuter;

    /* Pointer Validation */
    if (pCMe == 0) {
        return ERR_ECO_POINTER;
    }

    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Delete Function
 * </summary>
 *
 * <description>
 *   Instance freeing function
 * </description>
 *
 */
static void ECOCALLMETHOD deleteCEcoAdvancedMath_4CAA1B2F(/* in */ CEcoAdvancedMath_4CAA1B2FPtr_t pCMe) {
    IEcoMemoryAllocator1* pIMem = 0;

    if (pCMe != 0 ) {
        pIMem = pCMe->m_pIMem;
        /* Freeing */
        if ( pCMe->m_pISys != 0 ) {
            pCMe->m_pISys->pVTbl->Release(pCMe->m_pISys);
        }
        if ( pIMem != 0 ) {
            pIMem->pVTbl->Free(pIMem, pCMe);
            pIMem->pVTbl->Release(pIMem);
        }
    }
}

/* IEcoAdvancedMath Virtual Table */
IEcoAdvancedMathVTbl g_xA8A4C0172F9246D4A3F7870EAC1CDCB6VTbl_4CAA1B2F = {
    CEcoAdvancedMath_4CAA1B2F_QueryInterface,
    CEcoAdvancedMath_4CAA1B2F_AddRef,
    CEcoAdvancedMath_4CAA1B2F_Release,
    CEcoAdvancedMath_4CAA1B2F_spline,
    CEcoAdvancedMath_4CAA1B2F_splinef,
    CEcoAdvancedMath_4CAA1B2F_splinel,
    CEcoAdvancedMath_4CAA1B2F_new_Spline
};



/* Object Instance */
CEcoAdvancedMath_4CAA1B2F g_xCEcoAdvancedMath_4CAA1B2F = {
    &g_xA8A4C0172F9246D4A3F7870EAC1CDCB6VTbl_4CAA1B2F,
    initCEcoAdvancedMath_4CAA1B2F,
    createCEcoAdvancedMath_4CAA1B2F,
    deleteCEcoAdvancedMath_4CAA1B2F,
    1, /* m_cRef */
    0, /* m_pIMem */
    0  /* m_pISys */
};
