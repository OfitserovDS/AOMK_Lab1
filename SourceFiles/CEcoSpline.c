/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   CEcoSpline_4CAA1B2F
 * </summary>
 *
 * <description>
 *   Реализация объекта данных IEcoSpline - кубического сплайна в кусочно-полиномиальной
 *   форме (аналог pp-структуры MATLAB): значения (ppval), производные и определенный интеграл.
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#include "CEcoSpline.h"

/* Обобщенное ядро алгоритма для double: разделы SOLVER (построение) и EVAL (вычисление) */
#define ECO_SPLINE_CORE_SOLVER
#define ECO_SPLINE_CORE_EVAL

#define ECO_SPLINE_REAL     double_t
#define ECO_SPLINE_FN(name) name##_d
#include "CEcoSplineCore.h"
#undef ECO_SPLINE_FN
#undef ECO_SPLINE_REAL

/*
 *
 * <summary>
 *   EcoSplinePieceIntegral
 * </summary>
 *
 * <description>
 *   Интеграл многочлена куска p = {a, b, c, d} по локальной переменной от 0 до t:
 *       a*t^4/4 + b*t^3/3 + c*t^2/2 + d*t  (по схеме Горнера).
 * </description>
 *
 */
static double_t EcoSplinePieceIntegral(/* in */ const double_t* p, /* in */ double_t t) {
    return (((p[0] / 4 * t + p[1] / 3) * t + p[2] / 2) * t + p[3]) * t;
}

/*
 *
 * <summary>
 *   EcoSplineAntiderivative
 * </summary>
 *
 * <description>
 *   Значение первообразной G(X) = интеграл S от x[0] до X. Для X вне [x[0], x[n-1]]
 *   используется продолжение крайних кусков (как при вычислении значений).
 * </description>
 *
 */
static double_t EcoSplineAntiderivative(/* in */ const CEcoSpline_4CAA1B2F* pCMe, /* in */ double_t X) {
    uint32_t k = EcoSplineFindPiece_d(pCMe->m_pBreaks, pCMe->m_nBreaks, X);

    return pCMe->m_pIntegrals[k] + EcoSplinePieceIntegral(pCMe->m_pCoefs + 4 * k, X - pCMe->m_pBreaks[k]);
}

/*
 *
 * <summary>
 *   QueryInterface Function
 * </summary>
 *
 * <description>
 *   QueryInterface function for the IEcoSpline interface
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_QueryInterface(/* in */ IEcoSplinePtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }

    /* Validate and retrieve requested interface */
    if ( IsEqualUGUID(riid, &IID_IEcoSpline) || IsEqualUGUID(riid, &IID_IEcoUnknown) ) {
        *ppv = &pCMe->m_pVTblIEcoSpline;
        pCMe->m_pVTblIEcoSpline->AddRef((IEcoSpline*)pCMe);
        return ERR_ECO_SUCCESS;
    }
    *ppv = 0;
    return ERR_ECO_NOINTERFACE;
}

/*
 *
 * <summary>
 *   AddRef Function
 * </summary>
 *
 * <description>
 *   AddRef function for the IEcoSpline interface
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_AddRef(/* in */ IEcoSplinePtr_t me) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;

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
 *   Release function for the IEcoSpline interface. При обнулении счетчика освобождает
 *   блок памяти объекта и ссылку на распределитель памяти.
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_Release(/* in */ IEcoSplinePtr_t me) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;
    IEcoMemoryAllocator1* pIMem = 0;

    /* Pointer Validation */
    if (me == 0 ) {
        return (uint32_t)-1; /* ERR_ECO_POINTER */
    }

    /* Decrementing the object's reference count */
    --pCMe->m_cRef;
    /* If the count is zero, free the instance data */
    if ( pCMe->m_cRef == 0 ) {
        pIMem = pCMe->m_pIMem;
        /* Объект и его массивы - один блок */
        pIMem->pVTbl->Free(pIMem, pCMe);
        pIMem->pVTbl->Release(pIMem);
        return 0;
    }
    return pCMe->m_cRef;
}

/*
 *
 * <summary>
 *   get_Pieces Function
 * </summary>
 *
 * <description>
 *   Возвращает число кубических кусков сплайна (n - 1), 0 при me == 0.
 * </description>
 *
 */
static uint32_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_get_Pieces(/* in */ IEcoSplinePtr_t me) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 ) {
        return 0;
    }

    return pCMe->m_nBreaks - 1;
}

/*
 *
 * <summary>
 *   get_Breaks Function
 * </summary>
 *
 * <description>
 *   Копирует n узлов сплайна в буфер pBreaks размером size элементов.
 *   Возвращает ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL, если size < n.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_get_Breaks(/* in */ IEcoSplinePtr_t me, /* out */ double_t* pBreaks, /* in */ uint32_t size) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 || pBreaks == 0) {
        return ERR_ECO_POINTER;
    }
    if (size < pCMe->m_nBreaks) {
        return ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL;
    }

    /* Копирование средствами распределителя памяти Eco (без memcpy из libc) */
    pCMe->m_pIMem->pVTbl->Copy(pCMe->m_pIMem, pBreaks, pCMe->m_pBreaks, pCMe->m_nBreaks * (uint32_t)sizeof(double_t));
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   get_Coefs Function
 * </summary>
 *
 * <description>
 *   Копирует 4*(n-1) коэффициентов {a, b, c, d} в буфер pCoefs размером size элементов.
 *   Возвращает ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL, если size < 4*(n-1).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_get_Coefs(/* in */ IEcoSplinePtr_t me, /* out */ double_t* pCoefs, /* in */ uint32_t size) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;
    uint32_t count = 0;

    /* Pointer Validation */
    if (me == 0 || pCoefs == 0) {
        return ERR_ECO_POINTER;
    }
    count = 4 * (pCMe->m_nBreaks - 1);
    if (size < count) {
        return ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL;
    }

    pCMe->m_pIMem->pVTbl->Copy(pCMe->m_pIMem, pCoefs, pCMe->m_pCoefs, count * (uint32_t)sizeof(double_t));
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Evaluate Function
 * </summary>
 *
 * <description>
 *   yq[j] = S(xq[j]), j = 0..m-1 (аналог ppval MATLAB). При m == 0 указатели xq и yq не используются.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_Evaluate(/* in */ IEcoSplinePtr_t me, /* in */ const double_t* xq, /* in */ uint32_t m, /* out */ double_t* yq) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 || (m > 0 && (xq == 0 || yq == 0))) {
        return ERR_ECO_POINTER;
    }

    EcoSplineEvaluate_d(pCMe->m_pBreaks, pCMe->m_pCoefs, pCMe->m_nBreaks, xq, m, yq);
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   EvaluateDerivative Function
 * </summary>
 *
 * <description>
 *   yq[j] = S^(order)(xq[j]). Производные куска S = a*t^3 + b*t^2 + c*t + d:
 *       S'   = (3a*t + 2b)*t + c,   S'' = 6a*t + 2b,   S''' = 6a,   S^(k) = 0 при k > 3.
 *   В узле берется кусок справа (как при вычислении значений), поэтому для разрывной
 *   S''' возвращается значение правого куска.
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_EvaluateDerivative(/* in */ IEcoSplinePtr_t me, /* in */ uint32_t order, /* in */ const double_t* xq, /* in */ uint32_t m, /* out */ double_t* yq) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;
    const double_t* p = 0;
    double_t t = 0;
    uint32_t j = 0;
    uint32_t k = 0;

    /* Pointer Validation */
    if (me == 0 || (m > 0 && (xq == 0 || yq == 0))) {
        return ERR_ECO_POINTER;
    }

    for (j = 0; j < m; j++) {
        k = EcoSplineFindPiece_d(pCMe->m_pBreaks, pCMe->m_nBreaks, xq[j]);
        p = pCMe->m_pCoefs + 4 * k;
        t = xq[j] - pCMe->m_pBreaks[k];
        switch (order) {
        case 0:
            yq[j] = ((p[0] * t + p[1]) * t + p[2]) * t + p[3];
            break;
        case 1:
            yq[j] = (3 * p[0] * t + 2 * p[1]) * t + p[2];
            break;
        case 2:
            yq[j] = 6 * p[0] * t + 2 * p[1];
            break;
        case 3:
            yq[j] = 6 * p[0];
            break;
        default:
            yq[j] = 0;
            break;
        }
    }
    return ERR_ECO_SUCCESS;
}

/*
 *
 * <summary>
 *   Integrate Function
 * </summary>
 *
 * <description>
 *   *pResult = интеграл S(x) dx от a до b = G(b) - G(a), где G - первообразная
 *   (значения G в узлах вычислены при создании объекта, поэтому сложность O(log n)).
 *   При a > b результат отрицателен. Пределы вне [x[0], x[n-1]] допустимы (экстраполяция).
 * </description>
 *
 */
static int16_t ECOCALLMETHOD CEcoSpline_4CAA1B2F_Integrate(/* in */ IEcoSplinePtr_t me, /* in */ double_t a, /* in */ double_t b, /* out */ double_t* pResult) {
    CEcoSpline_4CAA1B2F* pCMe = (CEcoSpline_4CAA1B2F*)me;

    /* Pointer Validation */
    if (me == 0 || pResult == 0) {
        return ERR_ECO_POINTER;
    }
    if (!EcoSplineIsFinite_d(a) || !EcoSplineIsFinite_d(b)) {
        return ERR_ECOADVANCEDMATH_NOT_FINITE;
    }

    *pResult = EcoSplineAntiderivative(pCMe, b) - EcoSplineAntiderivative(pCMe, a);
    return ERR_ECO_SUCCESS;
}

/* IEcoSpline Virtual Table */
static IEcoSplineVTbl g_xB0ED03B2183147478635F0C9191A7C2BVTbl_4CAA1B2F = {
    CEcoSpline_4CAA1B2F_QueryInterface,
    CEcoSpline_4CAA1B2F_AddRef,
    CEcoSpline_4CAA1B2F_Release,
    CEcoSpline_4CAA1B2F_get_Pieces,
    CEcoSpline_4CAA1B2F_get_Breaks,
    CEcoSpline_4CAA1B2F_get_Coefs,
    CEcoSpline_4CAA1B2F_Evaluate,
    CEcoSpline_4CAA1B2F_EvaluateDerivative,
    CEcoSpline_4CAA1B2F_Integrate
};

/*
 *
 * <summary>
 *   createCEcoSpline_4CAA1B2F Function
 * </summary>
 *
 * <description>
 *   Создает объект сплайна:
 *     1) проверяет входные данные;
 *     2) выделяет один блок под объект и массивы (6n - 4 чисел double) и рабочий
 *        массив 4n (наклоны + диагонали прогонки). Массивы начинаются после структуры,
 *        размер которой округляется вверх до кратного sizeof(double_t): на 32-битных
 *        платформах sizeof структуры (28 байт) не кратен 8, и без округления числа double
 *        были бы не выровнены (на RISC-V это может вызвать исключение);
 *     3) решает СЛАУ для наклонов методом прогонки и строит коэффициенты кусков;
 *     4) вычисляет значения первообразной в узлах для быстрого интегрирования.
 *   Объект получает собственную ссылку на распределитель памяти (AddRef).
 * </description>
 *
 */
int16_t createCEcoSpline_4CAA1B2F(/* in */ IEcoMemoryAllocator1* pIMem, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ const ECO_SPLINE_ENDS_T* pEnds, /* out */ IEcoSpline** ppISpline) {
    CEcoSpline_4CAA1B2F* pCObj = 0;
    double_t* pWork = 0;
    uint32_t headerSize = 0;
    uint32_t k = 0;
    int16_t result = ERR_ECO_SUCCESS;

    /* Pointer Validation */
    if (ppISpline == 0) {
        return ERR_ECO_POINTER;
    }
    *ppISpline = 0;
    if (pIMem == 0 || x == 0 || y == 0) {
        return ERR_ECO_POINTER;
    }

    /* Проверка входных данных */
    result = EcoSplineCheckData_d(x, y, n);
    if (result != ERR_ECO_SUCCESS) {
        return result;
    }
    result = EcoSplineCheckEnds(pEnds);
    if (result != ERR_ECO_SUCCESS) {
        return result;
    }

    /* Размер структуры, округленный вверх до кратного sizeof(double_t): массивы после нее выровнены */
    headerSize = (uint32_t)((sizeof(CEcoSpline_4CAA1B2F) + sizeof(double_t) - 1) / sizeof(double_t) * sizeof(double_t));

    /* Размер блока объекта должен помещаться в uint32_t (аргумент Alloc) */
    if (n > (uint32_t)((0xFFFFFFFFUL - headerSize) / (6UL * sizeof(double_t)))) {
        return ERR_ECO_OUTOFMEMORY;
    }

    /* Блок объекта: структура (headerSize байт) + узлы n + коэффициенты 4(n-1) + первообразная n */
    pCObj = (CEcoSpline_4CAA1B2F*)pIMem->pVTbl->Alloc(pIMem, (uint32_t)(headerSize + (6UL * n - 4) * sizeof(double_t)));
    if (pCObj == 0) {
        return ERR_ECO_OUTOFMEMORY;
    }
    /* Рабочий массив: наклоны s[n] + диагонали прогонки 3n */
    pWork = (double_t*)pIMem->pVTbl->Alloc(pIMem, (uint32_t)(4UL * n * sizeof(double_t)));
    if (pWork == 0) {
        pIMem->pVTbl->Free(pIMem, pCObj);
        return ERR_ECO_OUTOFMEMORY;
    }

    /* Формирование данных экземпляра */
    pCObj->m_pVTblIEcoSpline = &g_xB0ED03B2183147478635F0C9191A7C2BVTbl_4CAA1B2F;
    pCObj->m_cRef = 1;
    pCObj->m_pIMem = pIMem;
    pCObj->m_nBreaks = n;
    pCObj->m_pBreaks = (double_t*)((uint8_t*)pCObj + headerSize);
    pCObj->m_pCoefs = pCObj->m_pBreaks + n;
    pCObj->m_pIntegrals = pCObj->m_pCoefs + 4 * (n - 1);

    /* Наклоны (метод прогонки) и коэффициенты кусков */
    result = EcoSplineSolveSlopes_d(x, y, n, pEnds, pWork, pWork + n);
    if (result == ERR_ECO_SUCCESS) {
        /* Copy принимает неконстантный указатель на источник (API IEcoMemoryAllocator1), данные не изменяются */
        pIMem->pVTbl->Copy(pIMem, pCObj->m_pBreaks, (voidptr_t)x, n * (uint32_t)sizeof(double_t));
        EcoSplineBuildCoefs_d(x, y, pWork, n, pCObj->m_pCoefs);

        /* Значения первообразной в узлах: F[0] = 0, F[k+1] = F[k] + интеграл k-го куска */
        pCObj->m_pIntegrals[0] = 0;
        for (k = 0; k + 1 < n; k++) {
            pCObj->m_pIntegrals[k + 1] = pCObj->m_pIntegrals[k] + EcoSplinePieceIntegral(pCObj->m_pCoefs + 4 * k, x[k + 1] - x[k]);
        }
    }
    pIMem->pVTbl->Free(pIMem, pWork);

    if (result != ERR_ECO_SUCCESS) {
        pIMem->pVTbl->Free(pIMem, pCObj);
        return result;
    }

    /* Объект хранит собственную ссылку на распределитель памяти */
    pIMem->pVTbl->AddRef(pIMem);
    *ppISpline = (IEcoSpline*)pCObj;
    return ERR_ECO_SUCCESS;
}
