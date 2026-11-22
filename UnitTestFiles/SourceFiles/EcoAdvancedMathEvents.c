/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   UnitTest событий сплайна через интерфейсную шину АОМК.
 * </summary>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 */

#include <stdio.h>
#include "IEcoInterfaceBus1.h"
#include "IEcoConnectionPointContainer.h"
#include "IEcoConnectionPoint.h"
#include "IEcoEnumConnectionPoints.h"
#include "IEcoEnumConnections.h"
#include "IdEcoAdvancedMath.h"
#include "IEcoAdvancedMathEvents.h"

#define TRACE_STEPS 15

typedef enum SplineStepKind {
    SYSTEM_ROW, FORWARD_STEP, SLOPE_READY, PIECE_READY
} SplineStepKind;

typedef struct SplineStep {
    SplineStepKind kind;
    uint32_t index;
    ldouble_t values[4];
} SplineStep;

/* x = {0, 1, 2, 4}, f(x) = x^3 - 2x + 1; на концах задано f'(x). */
static const SplineStep g_Expected[TRACE_STEPS] = {
    {SYSTEM_ROW, 0, {0, 1, 0, -2}},
    {SYSTEM_ROW, 1, {1, 4, 1, 12}},
    {SYSTEM_ROW, 2, {2, 6, 1, 108}},
    {SYSTEM_ROW, 3, {0, 1, 0, 46}},
    {FORWARD_STEP, 0, {0, -2, 0, 0}},
    {FORWARD_STEP, 1, {-0.25L, 3.5L, 0, 0}},
    {FORWARD_STEP, 2, {-2.0L / 11, 202.0L / 11, 0, 0}},
    {FORWARD_STEP, 3, {0, 46, 0, 0}},
    {SLOPE_READY, 3, {46, 0, 0, 0}},
    {SLOPE_READY, 2, {10, 0, 0, 0}},
    {SLOPE_READY, 1, {1, 0, 0, 0}},
    {SLOPE_READY, 0, {-2, 0, 0, 0}},
    {PIECE_READY, 0, {1, 0, -2, 1}},
    {PIECE_READY, 1, {1, 3, 1, 0}},
    {PIECE_READY, 2, {1, 6, 10, 5}}
};

typedef struct TestSplineSink {
    IEcoAdvancedMathEvents iface;
    uint32_t refs;
    IEcoUnknown* sources[2];
    uint32_t seen[2];
    int valid;
    int printTrace;
} TestSplineSink;

static uint32_t ECOCALLMETHOD Sink_AddRef(IEcoAdvancedMathEventsPtr_t me) {
    TestSplineSink* self = (TestSplineSink*)me;
    return self != 0 ? ++self->refs : (uint32_t)-1;
}

static uint32_t ECOCALLMETHOD Sink_Release(IEcoAdvancedMathEventsPtr_t me) {
    TestSplineSink* self = (TestSplineSink*)me;
    return self != 0 ? --self->refs : (uint32_t)-1;
}

static int16_t ECOCALLMETHOD Sink_QueryInterface(IEcoAdvancedMathEventsPtr_t me, const UGUID* riid, voidptr_t* ppv) {
    if (ppv == 0) {
        return ERR_ECO_POINTER;
    }
    *ppv = 0;
    if (me == 0 || riid == 0) {
        return ERR_ECO_POINTER;
    }
    if (!IsEqualUGUID(riid, &IID_IEcoUnknown) && !IsEqualUGUID(riid, &IID_IEcoAdvancedMathEvents)) {
        return ERR_ECO_NOINTERFACE;
    }
    Sink_AddRef(me);
    *ppv = me;
    return ERR_ECO_SUCCESS;
}

static int16_t RecordStep(TestSplineSink* self, IEcoAdvancedMath* pSource, SplineStepKind kind, uint32_t index,
    ldouble_t a, ldouble_t b, ldouble_t c, ldouble_t d) {
    static const char_t* names[] = {"СЛАУ", "Прямой ход", "Наклон", "Коэффициенты"};
    static const int widths[] = {4, 2, 1, 4};
    IEcoUnknown* pIdentity = 0;
    const SplineStep* expected = 0;
    ldouble_t actual[4];
    ldouble_t difference = 0;
    uint32_t source = 0;
    uint32_t step = 0;
    int i = 0;
    int16_t result = 0;

    result = pSource->pVTbl->QueryInterface(pSource, &IID_IEcoUnknown, (voidptr_t*)&pIdentity);
    if (result != ERR_ECO_SUCCESS || pIdentity == 0) {
        self->valid = 0;
        return ERR_ECO_FAIL;
    }
    if (pIdentity != self->sources[0] && pIdentity != self->sources[1]) {
        self->valid = 0;
        pIdentity->pVTbl->Release(pIdentity);
        return ERR_ECO_FAIL;
    }
    source = pIdentity == self->sources[0] ? 0 : 1;
    pIdentity->pVTbl->Release(pIdentity);
    step = self->seen[source]++;
    expected = &g_Expected[step % TRACE_STEPS];
    actual[0] = a;
    actual[1] = b;
    actual[2] = c;
    actual[3] = d;
    if (source == 1 && kind == PIECE_READY) {
        actual[3] -= 10; /* f(x) + 10 у второго источника */
    }
    self->valid = self->valid && expected->kind == kind && expected->index == index;
    for (i = 0; i < widths[kind]; ++i) {
        difference = actual[i] - expected->values[i];
        self->valid = self->valid && difference >= -1e-5L && difference <= 1e-5L;
    }
    if (self->printTrace && step < TRACE_STEPS) {
        actual[3] = d;
        printf("    %c %s %lu:", 'A' + (int)source, names[kind], (unsigned long)index);
        for (i = 0; i < widths[kind]; ++i) {
            printf(" % .6Lg", (long double)actual[i]);
        }
        printf("\n");
    }
    return ERR_ECO_SUCCESS;
}

static int16_t ECOCALLMETHOD Sink_OnRow(IEcoAdvancedMathEventsPtr_t me, IEcoAdvancedMath* pSource, uint32_t row,
    ldouble_t lower, ldouble_t diagonal, ldouble_t upper, ldouble_t rhs) {
    return RecordStep((TestSplineSink*)me, pSource, SYSTEM_ROW, row, lower, diagonal, upper, rhs);
}

static int16_t ECOCALLMETHOD Sink_OnForward(IEcoAdvancedMathEventsPtr_t me, IEcoAdvancedMath* pSource, uint32_t row,
    ldouble_t alpha, ldouble_t beta) {
    return RecordStep((TestSplineSink*)me, pSource, FORWARD_STEP, row, alpha, beta, 0, 0);
}

static int16_t ECOCALLMETHOD Sink_OnSlope(IEcoAdvancedMathEventsPtr_t me, IEcoAdvancedMath* pSource, uint32_t node, ldouble_t slope) {
    return RecordStep((TestSplineSink*)me, pSource, SLOPE_READY, node, slope, 0, 0, 0);
}

static int16_t ECOCALLMETHOD Sink_OnPiece(IEcoAdvancedMathEventsPtr_t me, IEcoAdvancedMath* pSource, uint32_t piece,
    ldouble_t a, ldouble_t b, ldouble_t c, ldouble_t d) {
    return RecordStep((TestSplineSink*)me, pSource, PIECE_READY, piece, a, b, c, d);
}

static IEcoAdvancedMathEventsVTbl g_xTestSplineSinkVTbl = {
    Sink_QueryInterface, Sink_AddRef, Sink_Release, Sink_OnRow, Sink_OnForward, Sink_OnSlope, Sink_OnPiece
};

static IEcoConnectionPoint* FindPoint(IEcoAdvancedMath* pMath, void (*Check)(int, const char_t*)) {
    IEcoConnectionPointContainer* pContainer = 0;
    IEcoConnectionPoint* pPoint = 0;
    IEcoConnectionPoint* pFound = 0;
    IEcoEnumConnectionPoints* pEnum = 0;
    IEcoUnknown* pMathUnknown = 0;
    IEcoUnknown* pContainerUnknown = 0;
    UGUID iid;
    int16_t result = 0;

    result = pMath->pVTbl->QueryInterface(pMath, &IID_IEcoConnectionPointContainer, (voidptr_t*)&pContainer);
    Check(result == ERR_ECO_SUCCESS && pContainer != 0, "Контейнер доступен через QueryInterface");
    if (pContainer == 0) {
        return 0;
    }
    pMath->pVTbl->QueryInterface(pMath, &IID_IEcoUnknown, (voidptr_t*)&pMathUnknown);
    pContainer->pVTbl->QueryInterface(pContainer, &IID_IEcoUnknown, (voidptr_t*)&pContainerUnknown);
    Check(pMathUnknown != 0 && pMathUnknown == pContainerUnknown, "Контейнер и вычислительный интерфейс имеют общий IEcoUnknown");

    result = pContainer->pVTbl->FindConnectionPoint(pContainer, &IID_IEcoAdvancedMath, &pFound);
    Check(result == ERR_ECO_NOINTERFACE && pFound == 0, "Поиск точки с неподдерживаемым IID");
    if (pFound != 0) {
        pFound->pVTbl->Release(pFound);
        pFound = 0;
    }
    result = pContainer->pVTbl->FindConnectionPoint(pContainer, &IID_IEcoAdvancedMathEvents, &pPoint);
    Check(result == ERR_ECO_SUCCESS && pPoint != 0, "Поиск точки событий сплайна");
    if (pPoint != 0) {
        result = pPoint->pVTbl->GetConnectionInterface(pPoint, &iid);
        Check(result == ERR_ECO_SUCCESS && IsEqualUGUID(&iid, &IID_IEcoAdvancedMathEvents), "Точка возвращает IID событий");
    }
    result = pContainer->pVTbl->EnumConnectionPoints(pContainer, &pEnum);
    Check(result == ERR_ECO_SUCCESS && pEnum != 0, "Получение перечислителя точек");
    if (pEnum != 0) {
        result = pEnum->pVTbl->Next(pEnum, 1, &pFound, 0);
        Check(result == ERR_ECO_SUCCESS && pPoint != 0 && pFound == pPoint, "Перечислитель находит ту же точку");
        if (pFound != 0) {
            pFound->pVTbl->Release(pFound);
        }
        pFound = pPoint;
        result = pEnum->pVTbl->Next(pEnum, 1, &pFound, 0);
        Check(result == ERR_ECO_FAIL && pFound == 0, "Исчерпанный перечислитель очищает указатель на точку");
        pEnum->pVTbl->Release(pEnum);
    }
    if (pContainerUnknown != 0) {
        pContainerUnknown->pVTbl->Release(pContainerUnknown);
    }
    if (pMathUnknown != 0) {
        pMathUnknown->pVTbl->Release(pMathUnknown);
    }
    pContainer->pVTbl->Release(pContainer);
    return pPoint;
}

static void CheckConnections(IEcoConnectionPoint* pPoint, const uint32_t* cookies, void (*Check)(int, const char_t*)) {
    IEcoEnumConnections* pEnum = 0;
    IEcoEnumConnections* pClone = 0;
    EcoConnectionData data[2];
    uint32_t fetched = 0;
    uint32_t i = 0;
    int16_t result = 0;

    result = pPoint->pVTbl->EnumConnections(pPoint, &pEnum);
    Check(result == ERR_ECO_SUCCESS && pEnum != 0, "Получен перечислитель подписок");
    if (pEnum == 0) {
        return;
    }
    result = pEnum->pVTbl->Next(pEnum, 2, data, &fetched);
    Check(result == ERR_ECO_SUCCESS && fetched == 2 && data[0].pUnk != data[1].pUnk &&
          ((data[0].cCookie == cookies[0] && data[1].cCookie == cookies[1]) ||
           (data[1].cCookie == cookies[0] && data[0].cCookie == cookies[1])),
          "Next выдаёт две отдельные записи и правильные cookie");
    for (i = 0; i < fetched; ++i) {
        data[i].pUnk->pVTbl->Release(data[i].pUnk);
    }
    result = pEnum->pVTbl->Clone(pEnum, &pClone);
    Check(result == ERR_ECO_SUCCESS && pClone != 0, "Клонирование перечислителя подписок");
    if (pClone != 0) {
        result = pClone->pVTbl->Next(pClone, 1, data, &fetched);
        Check(result == ERR_ECO_FAIL && fetched == 0 && data[0].pUnk == 0 && data[0].cCookie == 0,
              "Клон сохраняет позицию после последнего элемента и очищает выход");
        if (fetched != 0) {
            data[0].pUnk->pVTbl->Release(data[0].pUnk);
        }
        pClone->pVTbl->Release(pClone);
    }
    pEnum->pVTbl->Release(pEnum);
}

void TestSplineEvents(IEcoInterfaceBus1* pBus, IEcoAdvancedMath* pFirst, void (*Check)(int, const char_t*)) {
    static const double_t x[4] = {0, 1, 2, 4};
    static const double_t yA[4] = {1, 0, 5, 57};
    static const double_t yB[4] = {11, 10, 15, 67};
    static const double_t xq = 3;
    static const float_t xf[4] = {0, 1, 2, 4};
    static const float_t yf[4] = {1, 0, 5, 57};
    static const float_t xqf = 3;
    static const ldouble_t xl[4] = {0, 1, 2, 4};
    static const ldouble_t yl[4] = {1, 0, 5, 57};
    static const ldouble_t xql = 3;
    static const ECO_SPLINE_ENDS_T ends = {ECO_SPLINE_END_FIRST_DERIVATIVE, ECO_SPLINE_END_FIRST_DERIVATIVE, -2, 46};
    IEcoAdvancedMath* pSecond = 0;
    IEcoUnknown* identities[2] = {0, 0};
    IEcoConnectionPoint* points[2] = {0, 0};
    IEcoConnectionPointContainer* pRetainedContainer = 0;
    IEcoSpline* pSpline = 0;
    IEcoUnknown* pSinkUnknown = 0;
    TestSplineSink sinks[2] = {{{0}, 0, {0, 0}, {0, 0}, 0, 0}, {{0}, 0, {0, 0}, {0, 0}, 0, 0}};
    uint32_t cookies[2][2] = {{0, 0}, {0, 0}};
    uint32_t cookie = 0;
    int16_t result = 0;
    int source = 0;
    int sink = 0;
    double_t answer = 0;
    float_t answerF = 0;
    ldouble_t answerL = 0;

    printf("\nЛабораторная 3. События сплайна, два источника и два приёмника\n");
    result = pBus->pVTbl->QueryComponent(pBus, &CID_EcoAdvancedMath, 0, &IID_IEcoAdvancedMath, (voidptr_t*)&pSecond);
    if (result == ERR_ECO_SUCCESS && pSecond != 0) {
        pFirst->pVTbl->QueryInterface(pFirst, &IID_IEcoUnknown, (voidptr_t*)&identities[0]);
        pSecond->pVTbl->QueryInterface(pSecond, &IID_IEcoUnknown, (voidptr_t*)&identities[1]);
    }
    Check(identities[0] != 0 && identities[1] != 0 && identities[0] != identities[1],
          "Шина создала два экземпляра Eco.AdvancedMath с разными IEcoUnknown");
    if (identities[0] == 0 || identities[1] == 0 || identities[0] == identities[1]) {
        goto Release;
    }
    for (sink = 0; sink < 2; ++sink) {
        sinks[sink].iface.pVTbl = &g_xTestSplineSinkVTbl;
        sinks[sink].refs = 1;
        sinks[sink].sources[0] = identities[0];
        sinks[sink].sources[1] = identities[1];
        sinks[sink].valid = 1;
    }
    sinks[0].printTrace = 1;
    points[0] = FindPoint(pFirst, Check);
    points[1] = FindPoint(pSecond, Check);
    if (points[0] == 0 || points[1] == 0) {
        goto Release;
    }
    result = points[0]->pVTbl->Advise(points[0], identities[0], &cookie);
    Check(result == ERR_ECO_NOINTERFACE && cookie == 0, "Advise отвергает объект без интерфейса приёмника");
    for (source = 0; source < 2; ++source) {
        for (sink = 0; sink < 2; ++sink) {
            result = sinks[sink].iface.pVTbl->QueryInterface(&sinks[sink].iface, &IID_IEcoUnknown, (voidptr_t*)&pSinkUnknown);
            if (result == ERR_ECO_SUCCESS && pSinkUnknown != 0) {
                result = points[source]->pVTbl->Advise(points[source], pSinkUnknown, &cookies[source][sink]);
                pSinkUnknown->pVTbl->Release(pSinkUnknown);
                pSinkUnknown = 0;
            }
            Check(result == ERR_ECO_SUCCESS && cookies[source][sink] != 0, "Подписка через Advise");
            if (cookies[source][sink] == 0) {
                goto Release;
            }
        }
        Check(cookies[source][0] != cookies[source][1], "Cookie уникальны в пределах точки");
    }
    CheckConnections(points[0], cookies[0], Check);

    result = pFirst->pVTbl->spline(pFirst, x, yA, 4, &ends, &xq, 1, &answer);
    Check(result == ERR_ECO_SUCCESS && answer == 22, "spline с подписчиками вычисляет f(3) = 22");
    result = pSecond->pVTbl->new_Spline(pSecond, x, yB, 4, &ends, &pSpline);
    if (result == ERR_ECO_SUCCESS && pSpline != 0) {
        result = pSpline->pVTbl->Evaluate(pSpline, &xq, 1, &answer);
    }
    Check(result == ERR_ECO_SUCCESS && pSpline != 0 && answer == 32, "new_Spline с подписчиками строит f(3) + 10 = 32");
    for (sink = 0; sink < 2; ++sink) {
        Check(sinks[sink].valid && sinks[sink].seen[0] == TRACE_STEPS && sinks[sink].seen[1] == TRACE_STEPS,
              "Приёмник получил по 15 событий от каждого источника в ожидаемом порядке и с верными значениями");
    }

    cookie = cookies[0][1];
    result = points[0]->pVTbl->Unadvise(points[0], cookie);
    Check(result == ERR_ECO_SUCCESS, "Снята одна из четырёх подписок");
    if (result == ERR_ECO_SUCCESS) {
        cookies[0][1] = 0;
    }
    result = points[0]->pVTbl->Unadvise(points[0], cookie);
    Check(result != ERR_ECO_SUCCESS, "Снятие уже удалённой подписки возвращает ошибку");
    result = pFirst->pVTbl->splinef(pFirst, xf, yf, 4, &ends, &xqf, 1, &answerF);
    Check(result == ERR_ECO_SUCCESS && answerF == 22, "Трассировка splinef");
    result = pFirst->pVTbl->splinel(pFirst, xl, yl, 4, &ends, &xql, 1, &answerL);
    Check(result == ERR_ECO_SUCCESS && answerL == 22, "Трассировка splinel");
    result = pSecond->pVTbl->spline(pSecond, x, yB, 4, &ends, &xq, 1, &answer);
    Check(result == ERR_ECO_SUCCESS && answer == 32, "Повторное вычисление у второго источника");
    Check(sinks[0].valid && sinks[0].seen[0] == 3 * TRACE_STEPS && sinks[0].seen[1] == 2 * TRACE_STEPS &&
          sinks[1].valid && sinks[1].seen[0] == TRACE_STEPS && sinks[1].seen[1] == 2 * TRACE_STEPS,
          "После Unadvise три оставшиеся подписки продолжают работать");
    printf("    После Unadvise, события A/B: приёмник 1 = %lu/%lu, приёмник 2 = %lu/%lu\n",
           (unsigned long)sinks[0].seen[0], (unsigned long)sinks[0].seen[1],
           (unsigned long)sinks[1].seen[0], (unsigned long)sinks[1].seen[1]);

    identities[1]->pVTbl->Release(identities[1]);
    identities[1] = 0;
    pSecond->pVTbl->Release(pSecond);
    pSecond = 0;
    result = points[1]->pVTbl->GetConnectionPointContainer(points[1], &pRetainedContainer);
    Check(result == ERR_ECO_SUCCESS && pRetainedContainer != 0, "Точка удерживает владельца после Release исходного интерфейса");
    if (pRetainedContainer != 0) {
        pRetainedContainer->pVTbl->Release(pRetainedContainer);
    }
    points[1]->pVTbl->Release(points[1]); /* Владелец удаляет оставшиеся подписки B. */
    points[1] = 0;

Release:
    if (pSpline != 0) {
        pSpline->pVTbl->Release(pSpline);
    }
    for (source = 0; source < 2; ++source) {
        if (points[source] != 0) {
            for (sink = 0; sink < 2; ++sink) {
                if (cookies[source][sink] != 0) {
                    points[source]->pVTbl->Unadvise(points[source], cookies[source][sink]);
                }
            }
            points[source]->pVTbl->Release(points[source]);
        }
        if (identities[source] != 0) {
            identities[source]->pVTbl->Release(identities[source]);
        }
    }
    if (pSecond != 0) {
        pSecond->pVTbl->Release(pSecond);
    }
    for (sink = 0; sink < 2; ++sink) {
        if (sinks[sink].refs != 0) {
            Check(sinks[sink].iface.pVTbl->Release(&sinks[sink].iface) == 0, "Все ссылки на приёмник освобождены");
        }
    }
}
