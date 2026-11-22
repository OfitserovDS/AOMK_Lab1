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
 *   Компонент поддерживает агрегирование внешним компонентом и предоставляет интерфейсы
 *   калькулятора IEcoCalculatorX / IEcoCalculatorY, повторно используя компоненты курса
 *   (B - агрегированием; A, C, D, E - включением).
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
#include "CEcoSplineEvents.h"
#include "IdEcoCalculatorA.h"
#include "IdEcoCalculatorB.h"
#include "IdEcoCalculatorD.h"
#include "IdEcoCalculatorE.h"

/* Экземпляр-прототип компонента (определен в конце файла) */
extern CEcoAdvancedMath_4CAA1B2F g_xCEcoAdvancedMath_4CAA1B2F;

/*
 * Указатель на экземпляр по указателю на интерфейс, таблица функций которого хранится
 * в поле MEMBER. Смещение поля вычисляется разностью адресов внутри экземпляра-прототипа
 * (обе точки - один объект), поэтому не нужны ни size_t, ни предположения о порядке полей.
 */
#define CEcoAdvancedMath_4CAA1B2F_FROM(me, MEMBER) \
    ((CEcoAdvancedMath_4CAA1B2F*)((char_t*)(me) - ((char_t*)&g_xCEcoAdvancedMath_4CAA1B2F.MEMBER - (char_t*)&g_xCEcoAdvancedMath_4CAA1B2F)))

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
 *   NondelegatingQueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface неделегирующего IEcoUnknown. Возвращает интерфейсы компонента:
 *   IEcoAdvancedMath; IEcoCalculatorX - интерфейс агрегированного компонента B или, если B
 *   недоступен, собственная реализация включением A (C); IEcoCalculatorY - собственная
 *   реализация включением D, E или C; IEcoUnknown - неделегирующий IEcoUnknown.
 *   Если внутренний компонент для интерфейса калькулятора недоступен, возвращается ERR_ECO_NOINTERFACE.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_NondelegatingQueryInterface(/* in */ IEcoUnknownPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblINondelegatingUnk);

    /* Validate and retrieve requested interface */
    if ( IsEqualUGUID(riid, &IID_IEcoAdvancedMath) ) {
        *ppv = &pCMe->m_pVTblIEcoAdvancedMath;
        pCMe->m_pVTblIEcoAdvancedMath->AddRef((IEcoAdvancedMath*)&pCMe->m_pVTblIEcoAdvancedMath);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoConnectionPointContainer) ) {
        *ppv = &pCMe->m_ConnectionContainer;
        pCMe->m_ConnectionContainer.pVTbl->AddRef(&pCMe->m_ConnectionContainer);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoCalculatorX) && pCMe->m_pIUnkInnerX != 0 ) {
        /* Агрегирование: интерфейс запрашивается у внутреннего компонента B */
        return pCMe->m_pIUnkInnerX->pVTbl->QueryInterface(pCMe->m_pIUnkInnerX, riid, ppv);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoCalculatorX) && pCMe->m_pIX != 0 ) {
        *ppv = &pCMe->m_pVTblIEcoCalculatorX;
        pCMe->m_pVTblIEcoCalculatorX->AddRef((IEcoCalculatorX*)&pCMe->m_pVTblIEcoCalculatorX);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoCalculatorY) && pCMe->m_pIY != 0 ) {
        *ppv = &pCMe->m_pVTblIEcoCalculatorY;
        pCMe->m_pVTblIEcoCalculatorY->AddRef((IEcoCalculatorY*)&pCMe->m_pVTblIEcoCalculatorY);
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoUnknown) ) {
        *ppv = &pCMe->m_pVTblINondelegatingUnk;
        ++pCMe->m_cRef;
    }
    else {
        return ERR_ECO_NOINTERFACE;
    }
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   NondelegatingAddRef Function
 * </summary>
 *
 * <description>
 *   AddRef неделегирующего IEcoUnknown: увеличивает собственный счетчик ссылок компонента
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_NondelegatingAddRef(/* in */ IEcoUnknownPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblINondelegatingUnk);

    return ++pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   NondelegatingRelease Function
 * </summary>
 *
 * <description>
 *   Release неделегирующего IEcoUnknown: уменьшает собственный счетчик ссылок компонента
 *   и при обнулении освобождает экземпляр
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_NondelegatingRelease(/* in */ IEcoUnknownPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblINondelegatingUnk);

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
 *   QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface function for the IEcoAdvancedMath interface: делегируется управляющему IEcoUnknown
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_QueryInterface(/* in */ IEcoAdvancedMathPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }

    return pCMe->m_pIUnkOuter->pVTbl->QueryInterface(pCMe->m_pIUnkOuter, riid, ppv);
}

/*
 *
 * <summary>
 *   AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef function for the IEcoAdvancedMath interface: делегируется управляющему IEcoUnknown
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_AddRef(/* in */ IEcoAdvancedMathPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }

    return pCMe->m_pIUnkOuter->pVTbl->AddRef(pCMe->m_pIUnkOuter);
}

/*
 *
 * <summary>
 *   Release Function
 * </summary>
 *
 * <description>
 *   Release function for the IEcoAdvancedMath interface: делегируется управляющему IEcoUnknown
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_Release(/* in */ IEcoAdvancedMathPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = (CEcoAdvancedMath_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }

    return pCMe->m_pIUnkOuter->pVTbl->Release(pCMe->m_pIUnkOuter);
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

    return EcoSplineInterpolate_d(pCMe->m_pIMem, x, y, n, pEnds, xq, m, yq, pCMe);
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

    return EcoSplineInterpolate_f(pCMe->m_pIMem, x, y, n, pEnds, xq, m, yq, pCMe);
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

    return EcoSplineInterpolate_l(pCMe->m_pIMem, x, y, n, pEnds, xq, m, yq, pCMe);
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
    if (ppISpline == 0) {
        return ERR_ECO_POINTER;
    }
    *ppISpline = 0;
    if (me == 0) {
        return ERR_ECO_POINTER;
    }

    return createCEcoSpline_4CAA1B2F(pCMe->m_pIMem, x, y, n, pEnds, ppISpline, pCMe);
}


/*
 *
 * <summary>
 *   IEcoCalculatorX_QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface интерфейса IEcoCalculatorX (реализация включением): передается
 *   управляющему IEcoUnknown, поэтому через этот интерфейс доступны все интерфейсы компонента
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorX_QueryInterface(/* in */ IEcoCalculatorXPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pIUnkOuter->pVTbl->QueryInterface(pCMe->m_pIUnkOuter, riid, ppv);
}

/*
 *
 * <summary>
 *   IEcoCalculatorX_AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef интерфейса IEcoCalculatorX: передается управляющему IEcoUnknown
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorX_AddRef(/* in */ IEcoCalculatorXPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pIUnkOuter->pVTbl->AddRef(pCMe->m_pIUnkOuter);
}

/*
 *
 * <summary>
 *   IEcoCalculatorX_Release Function
 * </summary>
 *
 * <description>
 *   Release интерфейса IEcoCalculatorX: передается управляющему IEcoUnknown
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorX_Release(/* in */ IEcoCalculatorXPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pIUnkOuter->pVTbl->Release(pCMe->m_pIUnkOuter);
}

/*
 *
 * <summary>
 *   Addition Function
 * </summary>
 *
 * <description>
 *   Сложение включением: вызов передается внутреннему компоненту A (или C).
 *   Возвращает -1 при me == 0 (как компоненты курса) и 0, если внутренний компонент недоступен.
 * </description>
 *
 */
static int32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_Addition(/* in */ IEcoCalculatorXPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorX);
    if (pCMe->m_pIX == 0) {
        return 0;
    }

    return pCMe->m_pIX->pVTbl->Addition(pCMe->m_pIX, a, b);
}

/*
 *
 * <summary>
 *   Subtraction Function
 * </summary>
 *
 * <description>
 *   Вычитание включением: вызов передается внутреннему компоненту A (или C).
 *   Возвращает -1 при me == 0 (как компоненты курса) и 0, если внутренний компонент недоступен.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_Subtraction(/* in */ IEcoCalculatorXPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorX);
    if (pCMe->m_pIX == 0) {
        return 0;
    }

    return pCMe->m_pIX->pVTbl->Subtraction(pCMe->m_pIX, a, b);
}

/*
 *
 * <summary>
 *   IEcoCalculatorY_QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface интерфейса IEcoCalculatorY (реализация включением): передается
 *   управляющему IEcoUnknown, поэтому через этот интерфейс доступны все интерфейсы компонента
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorY_QueryInterface(/* in */ IEcoCalculatorYPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pIUnkOuter->pVTbl->QueryInterface(pCMe->m_pIUnkOuter, riid, ppv);
}

/*
 *
 * <summary>
 *   IEcoCalculatorY_AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef интерфейса IEcoCalculatorY: передается управляющему IEcoUnknown
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorY_AddRef(/* in */ IEcoCalculatorYPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pIUnkOuter->pVTbl->AddRef(pCMe->m_pIUnkOuter);
}

/*
 *
 * <summary>
 *   IEcoCalculatorY_Release Function
 * </summary>
 *
 * <description>
 *   Release интерфейса IEcoCalculatorY: передается управляющему IEcoUnknown
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorY_Release(/* in */ IEcoCalculatorYPtr_t me) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pIUnkOuter->pVTbl->Release(pCMe->m_pIUnkOuter);
}

/*
 *
 * <summary>
 *   Multiplication Function
 * </summary>
 *
 * <description>
 *   Умножение включением: вызов передается внутреннему компоненту D, E или C.
 *   Возвращает -1 при me == 0 (как компоненты курса) и 0, если внутренний компонент недоступен.
 * </description>
 *
 */
static int32_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_Multiplication(/* in */ IEcoCalculatorYPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorY);
    if (pCMe->m_pIY == 0) {
        return 0;
    }

    return pCMe->m_pIY->pVTbl->Multiplication(pCMe->m_pIY, a, b);
}

/*
 *
 * <summary>
 *   Division Function
 * </summary>
 *
 * <description>
 *   Деление включением: вызов передается внутреннему компоненту D, E или C.
 *   Компоненты курса не проверяют делитель, поэтому при b == 0 деление
 *   не выполняется и возвращается 0. Возвращает -1 при me == 0 и 0, если внутренний компонент недоступен.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoAdvancedMath_4CAA1B2F_Division(/* in */ IEcoCalculatorYPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoAdvancedMath_4CAA1B2F* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoAdvancedMath_4CAA1B2F_FROM(me, m_pVTblIEcoCalculatorY);
    if (pCMe->m_pIY == 0 || b == 0) {
        return 0;
    }

    return pCMe->m_pIY->pVTbl->Division(pCMe->m_pIY, a, b);
}

/*
 *
 * <summary>
 *   InitCalculators Function
 * </summary>
 *
 * <description>
 *   Получает внутренние компоненты калькулятора через интерфейсную шину:
 *     IEcoCalculatorX - агрегирование компонента B (запрашивается IEcoUnknown, внешним
 *                       компонентом для B становится управляющий IEcoUnknown этого компонента);
 *                       если B недоступен - включение компонента A или его второй версии C
 *                       (тот же CID);
 *     IEcoCalculatorY - включение компонента D, если он недоступен - E, затем C (у A этого
 *                       интерфейса нет).
 *   Компоненты калькулятора необязательны: если ни один не зарегистрирован, интерфейс
 *   не предоставляется, а остальная функциональность компонента работает как прежде.
 * </description>
 *
 */
static void CEcoAdvancedMath_4CAA1B2F_InitCalculators(/* in */ CEcoAdvancedMath_4CAA1B2F* pCMe, /* in */ IEcoInterfaceBus1* pIBus) {
    int16_t result = 0;

    /* IEcoCalculatorX: агрегирование компонента B */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoCalculatorB, pCMe->m_pIUnkOuter, &IID_IEcoUnknown, (voidptr_t*)&pCMe->m_pIUnkInnerX);
    if (result != 0 || pCMe->m_pIUnkInnerX == 0) {
        pCMe->m_pIUnkInnerX = 0;
        /* B недоступен: включение компонента A (или C с тем же CID) */
        result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoCalculatorA, 0, &IID_IEcoCalculatorX, (voidptr_t*)&pCMe->m_pIX);
        if (result != 0 || pCMe->m_pIX == 0) {
            pCMe->m_pIX = 0;
        }
    }

    /* IEcoCalculatorY: включение компонента D, затем E, затем C */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoCalculatorD, 0, &IID_IEcoCalculatorY, (voidptr_t*)&pCMe->m_pIY);
    if (result != 0 || pCMe->m_pIY == 0) {
        pCMe->m_pIY = 0;
        result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoCalculatorE, 0, &IID_IEcoCalculatorY, (voidptr_t*)&pCMe->m_pIY);
    }
    if (result != 0 || pCMe->m_pIY == 0) {
        pCMe->m_pIY = 0;
        result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoCalculatorA, 0, &IID_IEcoCalculatorY, (voidptr_t*)&pCMe->m_pIY);
        if (result != 0 || pCMe->m_pIY == 0) {
            pCMe->m_pIY = 0;
        }
    }
}

/*
 *
 * <summary>
 *   Init Function
 * </summary>
 *
 * <description>
 *   Instance initialization function: получает системный интерфейс (с AddRef),
 *   через интерфейсную шину - распределитель памяти и внутренние компоненты калькулятора.
 *   При ошибке поля, которые не удалось получить, остаются нулевыми.
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

    /* Getting the inner calculator components */
    if (result == ERR_ECO_SUCCESS) {
        EcoSplineInitConnections(pCMe);
        CEcoAdvancedMath_4CAA1B2F_InitCalculators(pCMe, pIBus);
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
 *   Instance creation function: задает управляющий IEcoUnknown - внешний компонент
 *   при агрегировании, иначе собственный неделегирующий IEcoUnknown
 * </description>
 *
 */
static int16_t ECOCALLMETHOD createCEcoAdvancedMath_4CAA1B2F(/* in */ CEcoAdvancedMath_4CAA1B2FPtr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter) {

    /* Parameter is not used */
    (void)pIUnkSystem;

    /* Pointer Validation */
    if (pCMe == 0) {
        return ERR_ECO_POINTER;
    }

    /* Controlling IEcoUnknown */
    if (pIUnkOuter != 0) {
        pCMe->m_pIUnkOuter = pIUnkOuter;
    }
    else {
        pCMe->m_pIUnkOuter = (IEcoUnknown*)&pCMe->m_pVTblINondelegatingUnk;
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
        EcoSplineClearConnections(pCMe);
        /* Freeing the inner calculator components */
        if ( pCMe->m_pIY != 0 ) {
            pCMe->m_pIY->pVTbl->Release(pCMe->m_pIY);
        }
        if ( pCMe->m_pIX != 0 ) {
            pCMe->m_pIX->pVTbl->Release(pCMe->m_pIX);
        }
        if ( pCMe->m_pIUnkInnerX != 0 ) {
            pCMe->m_pIUnkInnerX->pVTbl->Release(pCMe->m_pIUnkInnerX);
        }
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

/* Nondelegating IEcoUnknown Virtual Table */
static IEcoUnknownVTbl g_x000000000000000000000000000000AAVTbl_4CAA1B2F = {
    CEcoAdvancedMath_4CAA1B2F_NondelegatingQueryInterface,
    CEcoAdvancedMath_4CAA1B2F_NondelegatingAddRef,
    CEcoAdvancedMath_4CAA1B2F_NondelegatingRelease
};

/* IEcoCalculatorX Virtual Table (включение) */
static IEcoCalculatorXVTbl g_x9322111622484742AE0682819447843DVTbl_4CAA1B2F = {
    CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorX_QueryInterface,
    CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorX_AddRef,
    CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorX_Release,
    CEcoAdvancedMath_4CAA1B2F_Addition,
    CEcoAdvancedMath_4CAA1B2F_Subtraction
};

/* IEcoCalculatorY Virtual Table (включение) */
static IEcoCalculatorYVTbl g_xBD6414C29096423EA90C04D77AFD1CADVTbl_4CAA1B2F = {
    CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorY_QueryInterface,
    CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorY_AddRef,
    CEcoAdvancedMath_4CAA1B2F_IEcoCalculatorY_Release,
    CEcoAdvancedMath_4CAA1B2F_Multiplication,
    CEcoAdvancedMath_4CAA1B2F_Division
};



/* Object Instance */
CEcoAdvancedMath_4CAA1B2F g_xCEcoAdvancedMath_4CAA1B2F = {
    &g_xA8A4C0172F9246D4A3F7870EAC1CDCB6VTbl_4CAA1B2F,
    initCEcoAdvancedMath_4CAA1B2F,
    createCEcoAdvancedMath_4CAA1B2F,
    deleteCEcoAdvancedMath_4CAA1B2F,
    1, /* m_cRef */
    0, /* m_pIMem */
    0, /* m_pISys */
    &g_x000000000000000000000000000000AAVTbl_4CAA1B2F, /* m_pVTblINondelegatingUnk */
    0, /* m_pIUnkOuter */
    &g_x9322111622484742AE0682819447843DVTbl_4CAA1B2F, /* m_pVTblIEcoCalculatorX */
    &g_xBD6414C29096423EA90C04D77AFD1CADVTbl_4CAA1B2F, /* m_pVTblIEcoCalculatorY */
    0, /* m_pIUnkInnerX */
    0, /* m_pIX */
    0, /* m_pIY */
    {0}, /* m_ConnectionContainer */
    {{0}, 0, 0, 0, 0, 0} /* m_ConnectionPoint */
};
