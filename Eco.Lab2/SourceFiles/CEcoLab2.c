/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoLab2_2F9385F6
 * </summary>
 *
 * <description>
 *   This source code describes the implementation of the interfaces for CEcoLab2_2F9385F6.
 *   Внешний компонент Eco.Lab2 повторно использует готовые компоненты, не изменяя их:
 *     - агрегирование Eco.AdvancedMath: внутренний компонент создается с указателем на этот
 *       компонент как на внешний, и его интерфейс IEcoAdvancedMath передается клиенту напрямую;
 *     - включение компонента калькулятора E: интерфейсы IEcoCalculatorX / IEcoCalculatorY
 *       реализованы здесь и передают вызовы компоненту E (внутри E агрегирован компонент B).
 *       Если E недоступен, интерфейсы калькулятора предоставляет агрегированный Eco.AdvancedMath.
 *   Собственные методы IEcoLab2 (Calculate, SplineMean) используют внутренние компоненты.
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
#include "ErrEcoLab2.h"
#include "IdEcoAdvancedMath.h"
#include "IdEcoCalculatorE.h"

/* Экземпляр-прототип компонента (определен в конце файла) */
extern CEcoLab2_2F9385F6 g_xCEcoLab2_2F9385F6;

/*
 * Указатель на экземпляр по указателю на интерфейс, таблица функций которого хранится
 * в поле MEMBER. Смещение поля вычисляется разностью адресов внутри экземпляра-прототипа
 * (обе точки - один объект), поэтому не нужны ни size_t, ни предположения о порядке полей.
 */
#define CEcoLab2_2F9385F6_FROM(me, MEMBER) \
    ((CEcoLab2_2F9385F6*)((char_t*)(me) - ((char_t*)&g_xCEcoLab2_2F9385F6.MEMBER - (char_t*)&g_xCEcoLab2_2F9385F6)))

/*
 *
 * <summary>
 *   QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface function for the IEcoLab2 interface. Возвращает:
 *     IEcoLab2, IEcoUnknown        - собственные интерфейсы;
 *     IEcoCalculatorX / Y          - собственная реализация включением компонента E;
 *     IEcoAdvancedMath, а также    - запрос передается агрегированному Eco.AdvancedMath
 *     IEcoCalculatorX / Y без E      (его интерфейсы делегируют IEcoUnknown этому компоненту).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_QueryInterface(/* in */ IEcoLab2Ptr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoLab2_2F9385F6* pCMe = (CEcoLab2_2F9385F6*)me;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;

    /* Validate and retrieve requested interface */
    if ( IsEqualUGUID(riid, &IID_IEcoLab2) || IsEqualUGUID(riid, &IID_IEcoUnknown) ) {
        *ppv = &pCMe->m_pVTblIEcoLab2;
        ++pCMe->m_cRef;
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoCalculatorX) && pCMe->m_pIX != 0 ) {
        *ppv = &pCMe->m_pVTblIEcoCalculatorX;
        ++pCMe->m_cRef;
    }
    else if ( IsEqualUGUID(riid, &IID_IEcoCalculatorY) && pCMe->m_pIY != 0 ) {
        *ppv = &pCMe->m_pVTblIEcoCalculatorY;
        ++pCMe->m_cRef;
    }
    else if ( pCMe->m_pIUnkInnerMath != 0 &&
              ( IsEqualUGUID(riid, &IID_IEcoAdvancedMath) || IsEqualUGUID(riid, &IID_IEcoCalculatorX) || IsEqualUGUID(riid, &IID_IEcoCalculatorY) ) ) {
        /* Агрегирование: интерфейс запрашивается у внутреннего компонента Eco.AdvancedMath */
        return pCMe->m_pIUnkInnerMath->pVTbl->QueryInterface(pCMe->m_pIUnkInnerMath, riid, ppv);
    }
    else {
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
 *   AddRef function for the IEcoLab2 interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6_AddRef(/* in */ IEcoLab2Ptr_t me) {
    CEcoLab2_2F9385F6* pCMe = (CEcoLab2_2F9385F6*)me;

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
 *   Release function for the IEcoLab2 interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6_Release(/* in */ IEcoLab2Ptr_t me) {
    CEcoLab2_2F9385F6* pCMe = (CEcoLab2_2F9385F6*)me;

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
 *   Calculate Function
 * </summary>
 *
 * <description>
 *   *pResult = a op b. Интерфейс калькулятора запрашивается у самого компонента
 *   (QueryInterface), поэтому операцию выполняет тот компонент, который сейчас предоставляет
 *   интерфейс: E (включение) или Eco.AdvancedMath (агрегирование).
 *   Возвращает ERR_ECO_SUCCESS, ERR_ECO_POINTER, ERR_ECO_NOINTERFACE, ERR_ECOLAB2_BAD_OPERATION
 *   или ERR_ECOLAB2_DIVISION_BY_ZERO.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_Calculate(/* in */ IEcoLab2Ptr_t me, /* in */ uint16_t op, /* in */ int16_t a, /* in */ int16_t b, /* out */ int32_t* pResult) {
    IEcoCalculatorX* pIX = 0;
    IEcoCalculatorY* pIY = 0;
    int16_t result = ERR_ECO_SUCCESS;

    /* Pointer Validation */
    if (me == 0 || pResult == 0) {
        return ERR_ECO_POINTER;
    }
    *pResult = 0;

    switch (op) {
    case ECO_LAB2_OP_ADDITION:
    case ECO_LAB2_OP_SUBTRACTION:
        result = me->pVTbl->QueryInterface(me, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
        if (result != ERR_ECO_SUCCESS || pIX == 0) {
            return ERR_ECO_NOINTERFACE;
        }
        if (op == ECO_LAB2_OP_ADDITION) {
            *pResult = pIX->pVTbl->Addition(pIX, a, b);
        }
        else {
            *pResult = pIX->pVTbl->Subtraction(pIX, a, b);
        }
        pIX->pVTbl->Release(pIX);
        break;
    case ECO_LAB2_OP_MULTIPLICATION:
    case ECO_LAB2_OP_DIVISION:
        if (op == ECO_LAB2_OP_DIVISION && b == 0) {
            return ERR_ECOLAB2_DIVISION_BY_ZERO;
        }
        result = me->pVTbl->QueryInterface(me, &IID_IEcoCalculatorY, (voidptr_t*)&pIY);
        if (result != ERR_ECO_SUCCESS || pIY == 0) {
            return ERR_ECO_NOINTERFACE;
        }
        if (op == ECO_LAB2_OP_MULTIPLICATION) {
            *pResult = pIY->pVTbl->Multiplication(pIY, a, b);
        }
        else {
            *pResult = pIY->pVTbl->Division(pIY, a, b);
        }
        pIY->pVTbl->Release(pIY);
        break;
    default:
        return ERR_ECOLAB2_BAD_OPERATION;
    }
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   SplineMean Function
 * </summary>
 *
 * <description>
 *   *pMean = (интеграл S(x) dx от a до b) / (b - a), где S - кубический сплайн (not-a-knot)
 *   по n узлам (x, y). Сплайн строит и интегрирует агрегированный Eco.AdvancedMath:
 *   IEcoAdvancedMath::new_Spline и IEcoSpline::Integrate. Ошибки входных данных сплайна
 *   возвращаются кодами ERR_ECOADVANCEDMATH_*; при a == b - ERR_ECOLAB2_EMPTY_INTERVAL.
 *   Сложность: построение сплайна O(n), интегрирование O(log n).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_SplineMean(/* in */ IEcoLab2Ptr_t me, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ double_t a, /* in */ double_t b, /* out */ double_t* pMean) {
    IEcoAdvancedMath* pIMath = 0;
    IEcoSpline* pISpline = 0;
    double_t integral = 0;
    int16_t result = ERR_ECO_SUCCESS;

    /* Pointer Validation */
    if (me == 0 || x == 0 || y == 0 || pMean == 0) {
        return ERR_ECO_POINTER;
    }
    *pMean = 0;
    if (a == b) {
        return ERR_ECOLAB2_EMPTY_INTERVAL;
    }

    /* Интерфейс агрегированного Eco.AdvancedMath */
    result = me->pVTbl->QueryInterface(me, &IID_IEcoAdvancedMath, (voidptr_t*)&pIMath);
    if (result != ERR_ECO_SUCCESS || pIMath == 0) {
        return ERR_ECO_NOINTERFACE;
    }

    result = pIMath->pVTbl->new_Spline(pIMath, x, y, n, 0, &pISpline);
    if (result == ERR_ECO_SUCCESS) {
        result = pISpline->pVTbl->Integrate(pISpline, a, b, &integral);
        if (result == ERR_ECO_SUCCESS) {
            *pMean = integral / (b - a);
        }
        pISpline->pVTbl->Release(pISpline);
    }
    pIMath->pVTbl->Release(pIMath);
    return result;
}

/*
 *
 * <summary>
 *   IEcoCalculatorX_QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface интерфейса IEcoCalculatorX (реализация включением): передается
 *   интерфейсу IEcoLab2, поэтому через этот интерфейс доступны все интерфейсы компонента
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_IEcoCalculatorX_QueryInterface(/* in */ IEcoCalculatorXPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pVTblIEcoLab2->QueryInterface((IEcoLab2*)pCMe, riid, ppv);
}

/*
 *
 * <summary>
 *   IEcoCalculatorX_AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef интерфейса IEcoCalculatorX: передается интерфейсу IEcoLab2
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6_IEcoCalculatorX_AddRef(/* in */ IEcoCalculatorXPtr_t me) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pVTblIEcoLab2->AddRef((IEcoLab2*)pCMe);
}

/*
 *
 * <summary>
 *   IEcoCalculatorX_Release Function
 * </summary>
 *
 * <description>
 *   Release интерфейса IEcoCalculatorX: передается интерфейсу IEcoLab2
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6_IEcoCalculatorX_Release(/* in */ IEcoCalculatorXPtr_t me) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pVTblIEcoLab2->Release((IEcoLab2*)pCMe);
}

/*
 *
 * <summary>
 *   Addition Function
 * </summary>
 *
 * <description>
 *   Сложение включением: вызов передается интерфейсу IEcoCalculatorX, полученному у компонента E (интерфейс агрегированного в E компонента B).
 *   Возвращает -1 при me == 0 (как компоненты курса).
 * </description>
 *
 */
static int32_t ECOCALLMETHOD CEcoLab2_2F9385F6_Addition(/* in */ IEcoCalculatorXPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorX);

    return pCMe->m_pIX->pVTbl->Addition(pCMe->m_pIX, a, b);
}

/*
 *
 * <summary>
 *   Subtraction Function
 * </summary>
 *
 * <description>
 *   Вычитание включением: вызов передается интерфейсу IEcoCalculatorX, полученному у компонента E (интерфейс агрегированного в E компонента B).
 *   Возвращает -1 при me == 0 (как компоненты курса).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_Subtraction(/* in */ IEcoCalculatorXPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorX);

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
 *   интерфейсу IEcoLab2, поэтому через этот интерфейс доступны все интерфейсы компонента
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_IEcoCalculatorY_QueryInterface(/* in */ IEcoCalculatorYPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pVTblIEcoLab2->QueryInterface((IEcoLab2*)pCMe, riid, ppv);
}

/*
 *
 * <summary>
 *   IEcoCalculatorY_AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef интерфейса IEcoCalculatorY: передается интерфейсу IEcoLab2
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6_IEcoCalculatorY_AddRef(/* in */ IEcoCalculatorYPtr_t me) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pVTblIEcoLab2->AddRef((IEcoLab2*)pCMe);
}

/*
 *
 * <summary>
 *   IEcoCalculatorY_Release Function
 * </summary>
 *
 * <description>
 *   Release интерфейса IEcoCalculatorY: передается интерфейсу IEcoLab2
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoLab2_2F9385F6_IEcoCalculatorY_Release(/* in */ IEcoCalculatorYPtr_t me) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pVTblIEcoLab2->Release((IEcoLab2*)pCMe);
}

/*
 *
 * <summary>
 *   Multiplication Function
 * </summary>
 *
 * <description>
 *   Умножение включением: вызов передается компоненту E.
 *   Возвращает -1 при me == 0 (как компоненты курса).
 * </description>
 *
 */
static int32_t ECOCALLMETHOD CEcoLab2_2F9385F6_Multiplication(/* in */ IEcoCalculatorYPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorY);

    return pCMe->m_pIY->pVTbl->Multiplication(pCMe->m_pIY, a, b);
}

/*
 *
 * <summary>
 *   Division Function
 * </summary>
 *
 * <description>
 *   Деление включением: вызов передается компоненту E.
 *   Компоненты курса не проверяют делитель, поэтому при b == 0 деление
 *   не выполняется и возвращается 0. Возвращает -1 при me == 0.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoLab2_2F9385F6_Division(/* in */ IEcoCalculatorYPtr_t me, /* in */ int16_t a, /* in */ int16_t b) {
    CEcoLab2_2F9385F6* pCMe = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return -1; /* ERR_ECO_POINTER */
    }
    pCMe = CEcoLab2_2F9385F6_FROM(me, m_pVTblIEcoCalculatorY);
    if (b == 0) {
        return 0;
    }

    return pCMe->m_pIY->pVTbl->Division(pCMe->m_pIY, a, b);
}

/*
 *
 * <summary>
 *   Init Function
 * </summary>
 *
 * <description>
 *   Instance initialization function: получает системный интерфейс (с AddRef), через
 *   интерфейсную шину - распределитель памяти и внутренние компоненты:
 *     Eco.AdvancedMath - агрегирование: запрашивается IEcoUnknown, внешним компонентом
 *                        передается этот компонент;
 *     E                - включение: запрашивается IEcoCalculatorY, у него - IEcoCalculatorX
 *                        (интерфейс агрегированного внутри E компонента B).
 *   Внутренние компоненты необязательны: недоступный компонент просто не используется.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD initCEcoLab2_2F9385F6(/*in*/ CEcoLab2_2F9385F6Ptr_t me, /* in */ IEcoUnknownPtr_t pIUnkSystem) {
    CEcoLab2_2F9385F6* pCMe = (CEcoLab2_2F9385F6*)me;
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

    if (result == ERR_ECO_SUCCESS) {
        /* Агрегирование компонента Eco.AdvancedMath */
        if (pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoAdvancedMath, (IEcoUnknown*)pCMe, &IID_IEcoUnknown, (voidptr_t*)&pCMe->m_pIUnkInnerMath) != 0) {
            pCMe->m_pIUnkInnerMath = 0;
        }

        /* Включение компонента E */
        if (pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoCalculatorE, 0, &IID_IEcoCalculatorY, (voidptr_t*)&pCMe->m_pIY) != 0) {
            pCMe->m_pIY = 0;
        }
        if (pCMe->m_pIY != 0 && pCMe->m_pIY->pVTbl->QueryInterface(pCMe->m_pIY, &IID_IEcoCalculatorX, (voidptr_t*)&pCMe->m_pIX) != 0) {
            pCMe->m_pIX = 0;
        }
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
 *   Instance creation function (агрегирование этого компонента не поддерживается,
 *   дополнительных действий не требуется)
 * </description>
 *
 */
static int16_t ECOCALLMETHOD createCEcoLab2_2F9385F6(/* in */ CEcoLab2_2F9385F6Ptr_t pCMe, /* in */ IEcoUnknownPtr_t pIUnkSystem, /* in */ IEcoUnknownPtr_t pIUnkOuter) {

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
 *   Instance freeing function: освобождает внутренние компоненты, системный интерфейс,
 *   память экземпляра и распределитель памяти
 * </description>
 *
 */
static void ECOCALLMETHOD deleteCEcoLab2_2F9385F6(/* in */ CEcoLab2_2F9385F6Ptr_t pCMe) {
    IEcoMemoryAllocator1* pIMem = 0;

    if (pCMe != 0 ) {
        pIMem = pCMe->m_pIMem;
        /* Freeing the inner components */
        if ( pCMe->m_pIX != 0 ) {
            pCMe->m_pIX->pVTbl->Release(pCMe->m_pIX);
        }
        if ( pCMe->m_pIY != 0 ) {
            pCMe->m_pIY->pVTbl->Release(pCMe->m_pIY);
        }
        if ( pCMe->m_pIUnkInnerMath != 0 ) {
            pCMe->m_pIUnkInnerMath->pVTbl->Release(pCMe->m_pIUnkInnerMath);
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

/* IEcoLab2 Virtual Table */
static IEcoLab2VTbl g_x2D7665646D5B49FF9E2965E281F70DAFVTbl_2F9385F6 = {
    CEcoLab2_2F9385F6_QueryInterface,
    CEcoLab2_2F9385F6_AddRef,
    CEcoLab2_2F9385F6_Release,
    CEcoLab2_2F9385F6_Calculate,
    CEcoLab2_2F9385F6_SplineMean
};

/* IEcoCalculatorX Virtual Table (включение) */
static IEcoCalculatorXVTbl g_x9322111622484742AE0682819447843DVTbl_2F9385F6 = {
    CEcoLab2_2F9385F6_IEcoCalculatorX_QueryInterface,
    CEcoLab2_2F9385F6_IEcoCalculatorX_AddRef,
    CEcoLab2_2F9385F6_IEcoCalculatorX_Release,
    CEcoLab2_2F9385F6_Addition,
    CEcoLab2_2F9385F6_Subtraction
};

/* IEcoCalculatorY Virtual Table (включение) */
static IEcoCalculatorYVTbl g_xBD6414C29096423EA90C04D77AFD1CADVTbl_2F9385F6 = {
    CEcoLab2_2F9385F6_IEcoCalculatorY_QueryInterface,
    CEcoLab2_2F9385F6_IEcoCalculatorY_AddRef,
    CEcoLab2_2F9385F6_IEcoCalculatorY_Release,
    CEcoLab2_2F9385F6_Multiplication,
    CEcoLab2_2F9385F6_Division
};

/* Object Instance */
CEcoLab2_2F9385F6 g_xCEcoLab2_2F9385F6 = {
    &g_x2D7665646D5B49FF9E2965E281F70DAFVTbl_2F9385F6,
    &g_x9322111622484742AE0682819447843DVTbl_2F9385F6,
    &g_xBD6414C29096423EA90C04D77AFD1CADVTbl_2F9385F6,
    initCEcoLab2_2F9385F6,
    createCEcoLab2_2F9385F6,
    deleteCEcoLab2_2F9385F6,
    1, /* m_cRef */
    0, /* m_pIMem */
    0, /* m_pISys */
    0, /* m_pIUnkInnerMath */
    0, /* m_pIY */
    0  /* m_pIX */
};
