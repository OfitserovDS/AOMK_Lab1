/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   EcoLab2
 * </summary>
 *
 * <description>
 *   Юнит-тест лабораторной работы 2 (консольное приложение, точка входа EcoMain).
 *   Демонстрирует включение и агрегирование компонентов:
 *     Eco.AdvancedMath (лабораторная работа 1) - IEcoCalculatorX агрегированием B
 *       (без B - включением A или его второй версии C), IEcoCalculatorY включением D
 *       (без D - E, затем C); поддерживает агрегирование внешним компонентом;
 *     Eco.Lab2 - агрегирует Eco.AdvancedMath, включает E (внутри E агрегирован B).
 *   Компоненты-калькуляторы курса регистрируются в интерфейсной шине один раз через тестовые
 *   фабрики-переключатели: так в одном прогоне меняется набор доступных калькуляторов и видно,
 *   какой внутренний компонент обслуживает интерфейс; под CID компонента A подставляется
 *   его вторая версия C.
 *   Проверяются: запрос QueryInterface из любого интерфейса в любой и единственность
 *   IEcoUnknown (матрицы для Eco.AdvancedMath и Eco.Lab2), общий счетчик ссылок при
 *   агрегировании, результаты операций, обработка ошибочных данных и отсутствие утечек
 *   памяти (число занятых блоков менеджера памяти до и после многократного создания).
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */


/* Eco OS */
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IEcoMemoryManager1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoFileSystemManagement1.h"
#include "IdEcoAdvancedMath.h"
#include "ErrEcoAdvancedMath.h"
#include "IdEcoLab2.h"
#include "ErrEcoLab2.h"
#include "IdEcoCalculatorA.h"
#include "IdEcoCalculatorB.h"
#include "IdEcoCalculatorD.h"
#include "IdEcoCalculatorE.h"

/* Стандартная библиотека C - только в тестовом клиенте */
#include <stdio.h>


/*
 * Фабрики компонентов-калькуляторов курса (статические библиотеки из DependenciesFiles).
 * Тест регистрирует их в шине сам - и в статической, и в динамической сборке.
 * Компонент C - вторая версия A с тем же CID; его символы при сборке получили суффикс _C.
 */
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_4828F6552E4540E78121EBD220DC360E;     /* A */
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_AE202E543CE54550899603BD70C62565;     /* B */
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_4828F6552E4540E78121EBD220DC360E_C;   /* C */
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_3A8E44677E82475CB4A3719ED8397E61;     /* D */
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_872FEF1DE3314B87AD44D1E7C232C2F0;     /* E */

/* Таблица функций IEcoCalculatorX компонента B: совпадение с ней означает, что клиент получил
   интерфейс самого B, то есть интерфейс передан агрегированием */
extern IEcoCalculatorXVTbl g_x9322111622484742AE0682819447843DVTblB;


/* ========================================================================== */
/*   Вспомогательные функции теста                                            */
/* ========================================================================== */

/* Счетчики проверок */
static uint32_t g_nChecks = 0;
static uint32_t g_nFailed = 0;

/* Интерфейсная шина (регистрация компонентов-калькуляторов) */
static IEcoInterfaceBus1* g_pIBus = 0;

/*
 * <summary>
 *   Check
 * </summary>
 *
 * <description>
 *   Регистрирует результат проверки; при неудаче печатает сообщение.
 * </description>
 */
static void Check(int bOk, const char_t* pszMessage) {
    g_nChecks++;
    if (!bOk) {
        g_nFailed++;
        printf("    [ОШИБКА] %s\n", pszMessage);
    }
}

/*
 * <summary>
 *   PrintPadded
 * </summary>
 *
 * <description>
 *   Печатает строку и дополняет ее пробелами до width символов (символ кириллицы в UTF-8
 *   занимает 2 байта, поэтому ширина считается по символам, а не байтам).
 * </description>
 */
static void PrintPadded(const char_t* psz, int width) {
    const char_t* p = psz;
    int chars = 0;

    while (*p != 0) {
        if (((unsigned char)*p & 0xC0) != 0x80) {
            chars++;
        }
        p++;
    }
    printf("%s", psz);
    while (chars < width) {
        putchar(' ');
        chars++;
    }
}

/*
 * <summary>
 *   PrintSection
 * </summary>
 *
 * <description>
 *   Заголовок раздела протокола.
 * </description>
 */
static void PrintSection(const char_t* pszTitle) {
    printf("\n================================================================================\n");
    printf("%s\n", pszTitle);
    printf("================================================================================\n");
}

/*
 * <summary>
 *   CheckCode
 * </summary>
 *
 * <description>
 *   Проверяет код возврата метода и печатает строку протокола.
 * </description>
 */
static void CheckCode(int16_t actual, int16_t expected, const char_t* pszMessage) {
    g_nChecks++;
    if (actual != expected) {
        g_nFailed++;
        printf("    [ОШИБКА] ");
        PrintPadded(pszMessage, 66);
        printf(" код %d, ожидался %d\n", (int)actual, (int)expected);
    }
    else {
        printf("    [OK] ");
        PrintPadded(pszMessage, 70);
        printf(" код %d\n", (int)actual);
    }
}

/*
 * <summary>
 *   CheckRejected
 * </summary>
 *
 * <description>
 *   Проверяет, что запрос отклонен: код ошибки и нулевой указатель. Конкретный код зависит от
 *   шины: фабрика возвращает ERR_ECO_NOAGGREGATION, динамическая шина передает его клиенту,
 *   статическая сообщает ERR_ECO_COMPONENT_NOTFOUND.
 * </description>
 */
static void CheckRejected(int16_t actual, voidptr_t pv, const char_t* pszMessage) {
    g_nChecks++;
    if (actual == ERR_ECO_SUCCESS || pv != 0) {
        g_nFailed++;
        printf("    [ОШИБКА] ");
        PrintPadded(pszMessage, 66);
        printf(" код %d, ожидался отказ\n", (int)actual);
    }
    else {
        printf("    [OK] ");
        PrintPadded(pszMessage, 70);
        printf(" код %d\n", (int)actual);
    }
}

/*
 * <summary>
 *   AbsD
 * </summary>
 *
 * <description>
 *   Модуль числа (без fabs из math.h).
 * </description>
 */
static double_t AbsD(double_t v) {
    return v < 0 ? -v : v;
}

/*
 * <summary>
 *   Фабрика-переключатель
 * </summary>
 *
 * <description>
 *   Регистрируется в шине под CID компонента-калькулятора один раз и передает создание
 *   компонента выбранной фабрике m_pTarget или, если компонент "не установлен" (m_pTarget == 0),
 *   возвращает ERR_ECO_COMPONENT_NOTFOUND. Компоненты лабораторных работ по-прежнему
 *   запрашивают калькуляторы у шины по CID и не знают о переключателе.
 * </description>
 */
typedef struct CSwitchFactory {
    IEcoComponentFactoryVTbl* m_pVTbl;
    IEcoComponentFactory* m_pTarget;
} CSwitchFactory;

/*
 * <summary>
 *   Switch_QueryInterface
 * </summary>
 *
 * <description>
 *   QueryInterface переключателя: IEcoComponentFactory и IEcoUnknown - сам переключатель.
 * </description>
 */
static int16_t ECOCALLMETHOD Switch_QueryInterface(/* in */ IEcoComponentFactoryPtr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    if (me == 0 || riid == 0 || ppv == 0) {
        return ERR_ECO_POINTER;
    }
    if (IsEqualUGUID(riid, &IID_IEcoComponentFactory) || IsEqualUGUID(riid, &IID_IEcoUnknown)) {
        *ppv = me;
        return ERR_ECO_SUCCESS;
    }
    *ppv = 0;
    return ERR_ECO_NOINTERFACE;
}

/*
 * <summary>
 *   Switch_AddRef
 * </summary>
 *
 * <description>
 *   Переключатели - статические объекты: счетчик ссылок не нужен.
 * </description>
 */
static uint32_t ECOCALLMETHOD Switch_AddRef(/* in */ IEcoComponentFactoryPtr_t me) {
    (void)me;
    return 1;
}

/*
 * <summary>
 *   Switch_Release
 * </summary>
 *
 * <description>
 *   Переключатели - статические объекты: счетчик ссылок не нужен.
 * </description>
 */
static uint32_t ECOCALLMETHOD Switch_Release(/* in */ IEcoComponentFactoryPtr_t me) {
    (void)me;
    return 1;
}

/*
 * <summary>
 *   Switch_Alloc
 * </summary>
 *
 * <description>
 *   Создание компонента выбранной фабрикой; если компонент "не установлен" -
 *   ERR_ECO_COMPONENT_NOTFOUND.
 * </description>
 */
static int16_t ECOCALLMETHOD Switch_Alloc(/* in */ IEcoComponentFactoryPtr_t me, /* in */ IEcoUnknownPtr_t pISystem, /* in */ IEcoUnknownPtr_t pIUnknownOuter, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv) {
    IEcoComponentFactory* pTarget = ((CSwitchFactory*)me)->m_pTarget;

    if (pTarget == 0) {
        if (ppv != 0) {
            *ppv = 0;
        }
        return ERR_ECO_COMPONENT_NOTFOUND;
    }
    return pTarget->pVTbl->Alloc(pTarget, pISystem, pIUnknownOuter, riid, ppv);
}

/*
 * <summary>
 *   Switch_Init
 * </summary>
 *
 * <description>
 *   Init передается выбранной фабрике.
 * </description>
 */
static int16_t ECOCALLMETHOD Switch_Init(/* in */ IEcoComponentFactoryPtr_t me, /* in */ IEcoUnknownPtr_t pISystem, /* in */ voidptr_t pv) {
    IEcoComponentFactory* pTarget = ((CSwitchFactory*)me)->m_pTarget;

    return pTarget != 0 ? pTarget->pVTbl->Init(pTarget, pISystem, pv) : ERR_ECO_SUCCESS;
}

/*
 * <summary>
 *   Switch_get_Name
 * </summary>
 *
 * <description>
 *   Имя компонента выбранной фабрики.
 * </description>
 */
static char_t* ECOCALLMETHOD Switch_get_Name(/* in */ IEcoComponentFactoryPtr_t me) {
    IEcoComponentFactory* pTarget = ((CSwitchFactory*)me)->m_pTarget;

    return pTarget != 0 ? pTarget->pVTbl->get_Name(pTarget) : 0;
}

/*
 * <summary>
 *   Switch_get_Version
 * </summary>
 *
 * <description>
 *   Версия компонента выбранной фабрики.
 * </description>
 */
static char_t* ECOCALLMETHOD Switch_get_Version(/* in */ IEcoComponentFactoryPtr_t me) {
    IEcoComponentFactory* pTarget = ((CSwitchFactory*)me)->m_pTarget;

    return pTarget != 0 ? pTarget->pVTbl->get_Version(pTarget) : 0;
}

/*
 * <summary>
 *   Switch_get_Manufacturer
 * </summary>
 *
 * <description>
 *   Производитель компонента выбранной фабрики.
 * </description>
 */
static char_t* ECOCALLMETHOD Switch_get_Manufacturer(/* in */ IEcoComponentFactoryPtr_t me) {
    IEcoComponentFactory* pTarget = ((CSwitchFactory*)me)->m_pTarget;

    return pTarget != 0 ? pTarget->pVTbl->get_Manufacturer(pTarget) : 0;
}

static IEcoComponentFactoryVTbl g_xSwitchFactoryVTbl = {
    Switch_QueryInterface,
    Switch_AddRef,
    Switch_Release,
    Switch_Alloc,
    Switch_Init,
    Switch_get_Name,
    Switch_get_Version,
    Switch_get_Manufacturer
};

/* Переключатели под CID компонентов A (A или C), B, D, E */
static CSwitchFactory g_xSwitchA = {&g_xSwitchFactoryVTbl, 0};
static CSwitchFactory g_xSwitchB = {&g_xSwitchFactoryVTbl, 0};
static CSwitchFactory g_xSwitchD = {&g_xSwitchFactoryVTbl, 0};
static CSwitchFactory g_xSwitchE = {&g_xSwitchFactoryVTbl, 0};

/*
 * <summary>
 *   RegisterCalculators
 * </summary>
 *
 * <description>
 *   Регистрирует переключатели в шине под CID компонентов A, B, D, E (один раз).
 * </description>
 */
static int16_t RegisterCalculators(void) {
    int16_t result = 0;

    result = g_pIBus->pVTbl->RegisterComponent(g_pIBus, &CID_EcoCalculatorA, (IEcoUnknown*)&g_xSwitchA);
    if (result == 0) {
        result = g_pIBus->pVTbl->RegisterComponent(g_pIBus, &CID_EcoCalculatorB, (IEcoUnknown*)&g_xSwitchB);
    }
    if (result == 0) {
        result = g_pIBus->pVTbl->RegisterComponent(g_pIBus, &CID_EcoCalculatorD, (IEcoUnknown*)&g_xSwitchD);
    }
    if (result == 0) {
        result = g_pIBus->pVTbl->RegisterComponent(g_pIBus, &CID_EcoCalculatorE, (IEcoUnknown*)&g_xSwitchE);
    }
    return result;
}

/*
 * <summary>
 *   SetCalculators
 * </summary>
 *
 * <description>
 *   Задает набор доступных компонентов-калькуляторов. Под CID компонента A доступен либо A,
 *   либо его вторая версия C (bC != 0).
 * </description>
 */
static void SetCalculators(int bA, int bB, int bC, int bD, int bE) {
    if (bC) {
        g_xSwitchA.m_pTarget = GetIEcoComponentFactoryPtr_4828F6552E4540E78121EBD220DC360E_C;
    }
    else if (bA) {
        g_xSwitchA.m_pTarget = GetIEcoComponentFactoryPtr_4828F6552E4540E78121EBD220DC360E;
    }
    else {
        g_xSwitchA.m_pTarget = 0;
    }
    g_xSwitchB.m_pTarget = bB ? GetIEcoComponentFactoryPtr_AE202E543CE54550899603BD70C62565 : 0;
    g_xSwitchD.m_pTarget = bD ? GetIEcoComponentFactoryPtr_3A8E44677E82475CB4A3719ED8397E61 : 0;
    g_xSwitchE.m_pTarget = bE ? GetIEcoComponentFactoryPtr_872FEF1DE3314B87AD44D1E7C232C2F0 : 0;
}

/*
 * <summary>
 *   CreateMath
 * </summary>
 *
 * <description>
 *   Создает компонент лабораторной работы 1 (Eco.AdvancedMath) через интерфейсную шину.
 * </description>
 */
static IEcoAdvancedMath* CreateMath(void) {
    IEcoAdvancedMath* pIMath = 0;

    g_pIBus->pVTbl->QueryComponent(g_pIBus, &CID_EcoAdvancedMath, 0, &IID_IEcoAdvancedMath, (voidptr_t*)&pIMath);
    return pIMath;
}

/*
 * <summary>
 *   CreateLab2
 * </summary>
 *
 * <description>
 *   Создает компонент Eco.Lab2 через интерфейсную шину.
 * </description>
 */
static IEcoLab2* CreateLab2(void) {
    IEcoLab2* pILab2 = 0;

    g_pIBus->pVTbl->QueryComponent(g_pIBus, &CID_EcoLab2, 0, &IID_IEcoLab2, (voidptr_t*)&pILab2);
    return pILab2;
}


/* Интерфейсы компонентов для проверки свойств QueryInterface */
static const UGUID* const g_iidsMath[4] = {&IID_IEcoAdvancedMath, &IID_IEcoCalculatorX, &IID_IEcoCalculatorY, &IID_IEcoUnknown};
static const char_t* const g_namesMath[4] = {"IEcoAdvancedMath", "IEcoCalculatorX", "IEcoCalculatorY", "IEcoUnknown"};
static const UGUID* const g_iidsLab2[5] = {&IID_IEcoLab2, &IID_IEcoAdvancedMath, &IID_IEcoCalculatorX, &IID_IEcoCalculatorY, &IID_IEcoUnknown};
static const char_t* const g_namesLab2[5] = {"IEcoLab2", "IEcoAdvancedMath", "IEcoCalculatorX", "IEcoCalculatorY", "IEcoUnknown"};

/*
 * <summary>
 *   QueryMatrix
 * </summary>
 *
 * <description>
 *   Свойства QueryInterface: через pStart запрашивает у компонента count интерфейсов, затем
 *   из каждого полученного интерфейса запрашивает каждый и проверяет, что запрос выполнен и
 *   вернул тот же указатель, что и первый запрос этого интерфейса (в том числе единственность
 *   IEcoUnknown). Печатает матрицу результатов; вся матрица - одна проверка. Возвращает 1,
 *   если все запросы выполнены.
 * </description>
 */
static int QueryMatrix(IEcoUnknown* pStart, const UGUID* const* iids, const char_t* const* names, int count, const char_t* pszTitle) {
    IEcoUnknown* p[5] = {0, 0, 0, 0, 0};
    IEcoUnknown* q = 0;
    int bAll = 1;
    int i = 0;
    int j = 0;

    for (j = 0; j < count; j++) {
        if (pStart->pVTbl->QueryInterface(pStart, iids[j], (voidptr_t*)&p[j]) != ERR_ECO_SUCCESS) {
            p[j] = 0;
        }
    }

    printf("\n    %s\n    ", pszTitle);
    PrintPadded("из \\ в", 18);
    for (j = 0; j < count; j++) {
        printf("  ");
        PrintPadded(names[j], 16);
    }
    printf("\n");
    for (i = 0; i < count; i++) {
        printf("    ");
        PrintPadded(names[i], 18);
        for (j = 0; j < count; j++) {
            q = 0;
            if (p[i] != 0 && p[i]->pVTbl->QueryInterface(p[i], iids[j], (voidptr_t*)&q) == ERR_ECO_SUCCESS && q == p[j] && q != 0) {
                printf("  ");
                PrintPadded("+", 16);
            }
            else {
                printf("  ");
                PrintPadded("ОШИБКА", 16);
                bAll = 0;
            }
            if (q != 0) {
                q->pVTbl->Release(q);
            }
        }
        printf("\n");
    }
    Check(bAll, "QueryInterface: интерфейс не получен или запрос из другого интерфейса вернул другой указатель");

    for (j = 0; j < count; j++) {
        if (p[j] != 0) {
            p[j]->pVTbl->Release(p[j]);
        }
    }
    return bAll;
}


/* ========================================================================== */
/*   Тест 1. Пример из задания                                                */
/* ========================================================================== */

static void TestTaskExample(void) {
    IEcoAdvancedMath* pIMath = 0;
    IEcoCalculatorX* pIX = 0;
    IEcoCalculatorY* pIY = 0;
    int32_t c = 0;
    int bOk = 1;
    int16_t result = 0;

    PrintSection("Тест 1. Пример из задания: запрос интерфейсов калькулятора у компонента лабы 1");

    SetCalculators(1, 1, 0, 1, 1);
    pIMath = CreateMath();
    if (pIMath == 0) {
        Check(0, "Тест 1: компонент Eco.AdvancedMath не создан");
        return;
    }

    result = pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
    CheckCode(result, ERR_ECO_SUCCESS, "pIMath->QueryInterface(IID_IEcoCalculatorX, &pIX)");
    if (pIX != 0) {
        c = pIX->pVTbl->Addition(pIX, 3, 5);
        printf("    c = pIX->pVTbl->Addition(pIX, 3, 5)        = %ld\n", (long)c);
        bOk = bOk && c == 8;
        c = pIX->pVTbl->Subtraction(pIX, 3, 5);
        printf("    c = pIX->pVTbl->Subtraction(pIX, 3, 5)     = %ld\n", (long)c);
        bOk = bOk && c == -2;
    }

    result = pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorY, (voidptr_t*)&pIY);
    CheckCode(result, ERR_ECO_SUCCESS, "pIMath->QueryInterface(IID_IEcoCalculatorY, &pIY)");
    if (pIY != 0) {
        c = pIY->pVTbl->Multiplication(pIY, 3, 5);
        printf("    c = pIY->pVTbl->Multiplication(pIY, 3, 5)  = %ld\n", (long)c);
        bOk = bOk && c == 15;
        c = pIY->pVTbl->Division(pIY, 15, 3);
        printf("    c = pIY->pVTbl->Division(pIY, 15, 3)       = %ld\n", (long)c);
        bOk = bOk && c == 5;
        pIY->pVTbl->Release(pIY);
    }
    Check(bOk && pIX != 0 && pIY != 0, "Тест 1: ожидалось 3 + 5 = 8, 3 - 5 = -2, 3 * 5 = 15, 15 / 3 = 5");
    if (pIX != 0) {
        pIX->pVTbl->Release(pIX);
    }
    pIMath->pVTbl->Release(pIMath);
}


/* ========================================================================== */
/*   Тест 2. Компонент лабы 1: поставщики интерфейсов и свойства QueryInterface */
/* ========================================================================== */

typedef struct SCENARIO {
    const char_t* pszSet;     /* зарегистрированные калькуляторы */
    int bA, bB, bC, bD, bE;
    const char_t* pszX;       /* ожидаемый поставщик IEcoCalculatorX */
    const char_t* pszY;       /* ожидаемый поставщик IEcoCalculatorY */
    int bXAggregated;         /* X - интерфейс самого B (агрегирование) */
    int bHasX;
    int bHasY;
} SCENARIO;

static void TestProviders(void) {
    static const SCENARIO scenarios[6] = {
        {"A, B, D, E",       1, 1, 0, 1, 1, "агрегирование B", "включение D", 1, 1, 1},
        {"A, D, E",          1, 0, 0, 1, 1, "включение A",     "включение D", 0, 1, 1},
        {"A, E",             1, 0, 0, 0, 1, "включение A",     "включение E", 0, 1, 1},
        {"C на месте A",     0, 0, 1, 0, 0, "включение C",     "включение C", 0, 1, 1},
        {"A",                1, 0, 0, 0, 0, "включение A",     "нет",         0, 1, 0},
        {"нет калькуляторов", 0, 0, 0, 0, 0, "нет",            "нет",         0, 0, 0}
    };
    static const double_t x[3] = {0.0, 1.0, 2.0};
    static const double_t y[3] = {1.0, 3.0, 2.0};
    IEcoAdvancedMath* pIMath = 0;
    IEcoCalculatorX* pIX = 0;
    IEcoCalculatorY* pIY = 0;
    double_t xq = 0.5;
    double_t yq = 0;
    int32_t r[4];
    int16_t resultX = 0;
    int16_t resultY = 0;
    int bOk = 0;
    uint32_t i = 0;

    PrintSection("Тест 2. Компонент лабы 1 (Eco.AdvancedMath): поставщики интерфейсов калькулятора и свойства QueryInterface");
    printf("\n    ");
    PrintPadded("калькуляторы в шине", 20);
    PrintPadded("IEcoCalculatorX", 17);
    PrintPadded("IEcoCalculatorY", 15);
    printf("  12+4  12-4  12*4  12/4\n");

    for (i = 0; i < 6; i++) {
        SetCalculators(scenarios[i].bA, scenarios[i].bB, scenarios[i].bC, scenarios[i].bD, scenarios[i].bE);
        pIMath = CreateMath();
        if (pIMath == 0) {
            Check(0, "Тест 2: компонент Eco.AdvancedMath не создан");
            continue;
        }
        pIX = 0;
        pIY = 0;
        r[0] = r[1] = r[2] = r[3] = 0;
        resultX = pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
        resultY = pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorY, (voidptr_t*)&pIY);
        /* Наличие интерфейсов соответствует набору, при отсутствии - ERR_ECO_NOINTERFACE и нулевой указатель */
        bOk = (scenarios[i].bHasX ? resultX == ERR_ECO_SUCCESS && pIX != 0 : resultX == ERR_ECO_NOINTERFACE && pIX == 0) &&
              (scenarios[i].bHasY ? resultY == ERR_ECO_SUCCESS && pIY != 0 : resultY == ERR_ECO_NOINTERFACE && pIY == 0);
        if (pIX != 0) {
            r[0] = pIX->pVTbl->Addition(pIX, 12, 4);
            r[1] = pIX->pVTbl->Subtraction(pIX, 12, 4);
            /* Механизм: при агрегировании у клиента интерфейс самого B */
            bOk = bOk && (pIX->pVTbl == &g_x9322111622484742AE0682819447843DVTblB) == (scenarios[i].bXAggregated != 0) &&
                  r[0] == 16 && r[1] == 8;
        }
        if (pIY != 0) {
            r[2] = pIY->pVTbl->Multiplication(pIY, 12, 4);
            r[3] = pIY->pVTbl->Division(pIY, 12, 4);
            bOk = bOk && r[2] == 48 && r[3] == 3;
        }
        /* Собственная функциональность лабы 1 не зависит от калькуляторов */
        bOk = bOk && pIMath->pVTbl->spline(pIMath, x, y, 3, 0, &xq, 1, &yq) == ERR_ECO_SUCCESS;
        Check(bOk, "Тест 2: поставщики, механизм или результаты калькулятора не соответствуют набору (строка ниже)");

        printf("    ");
        PrintPadded(scenarios[i].pszSet, 20);
        PrintPadded(scenarios[i].pszX, 17);
        PrintPadded(scenarios[i].pszY, 15);
        if (pIX != 0) {
            printf("  %4ld  %4ld", (long)r[0], (long)r[1]);
        }
        else {
            printf("     -     -");
        }
        if (pIY != 0) {
            printf("  %4ld  %4ld\n", (long)r[2], (long)r[3]);
        }
        else {
            printf("     -     -\n");
        }

        if (pIY != 0) {
            pIY->pVTbl->Release(pIY);
        }
        if (pIX != 0) {
            pIX->pVTbl->Release(pIX);
        }
        pIMath->pVTbl->Release(pIMath);
    }
    printf("\n    Агрегирование: клиент получает интерфейс самого B (совпадает таблица функций B).\n");
    printf("    Включение: клиент получает интерфейс компонента лабы 1, который вызывает A, C, D или E.\n");
    printf("    C зарегистрирован под CID компонента A: без перекомпиляции та же цепочка получает от него\n");
    printf("    и IEcoCalculatorY, которого у первой версии A нет.\n");

    /* Свойства QueryInterface: X - интерфейс агрегированного B и X - обертка включения A */
    SetCalculators(1, 1, 0, 1, 1);
    pIMath = CreateMath();
    if (pIMath != 0) {
        QueryMatrix((IEcoUnknown*)pIMath, g_iidsMath, g_namesMath, 4, "QueryInterface из каждого интерфейса Eco.AdvancedMath в каждый (X - агрегирование B, Y - включение D):");
        pIMath->pVTbl->Release(pIMath);
    }
    else {
        Check(0, "Тест 2: компонент Eco.AdvancedMath не создан");
    }
    SetCalculators(1, 0, 0, 0, 1);
    pIMath = CreateMath();
    if (pIMath != 0) {
        QueryMatrix((IEcoUnknown*)pIMath, g_iidsMath, g_namesMath, 4, "QueryInterface из каждого интерфейса Eco.AdvancedMath в каждый (X - включение A, Y - включение E):");
        pIMath->pVTbl->Release(pIMath);
    }
    else {
        Check(0, "Тест 2: компонент Eco.AdvancedMath не создан");
    }
}


/* ========================================================================== */
/*   Тест 3. Eco.Lab2: агрегирование Eco.AdvancedMath и включение E           */
/* ========================================================================== */

/*
 * <summary>
 *   Cubic
 * </summary>
 *
 * <description>
 *   Кубический многочлен p(x) = x^3 - 2x: сплайн not-a-knot воспроизводит его точно.
 * </description>
 */
static double_t Cubic(double_t x) {
    return (x * x - 2) * x;
}

static void TestLab2(void) {
    static const double_t xs[6] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    double_t ys[6];
    double_t xq[4] = {0.5, 1.75, 3.25, 4.5};
    double_t yqLab2[4];
    double_t yqMath[4];
    double_t mean = 0;
    double_t err = 0;
    IEcoLab2* pILab2 = 0;
    IEcoAdvancedMath* pIMathL2 = 0;
    IEcoAdvancedMath* pIMath = 0;
    IEcoCalculatorX* pIX = 0;
    IEcoCalculatorY* pIY = 0;
    int32_t calc[4];
    int32_t direct[4];
    uint32_t r1 = 0;
    uint32_t r2 = 0;
    uint32_t i = 0;
    int bOk = 0;
    int16_t result = 0;

    PrintSection("Тест 3. Eco.Lab2: агрегирование Eco.AdvancedMath и включение E (внутри E агрегирован B)");

    for (i = 0; i < 6; i++) {
        ys[i] = Cubic(xs[i]);
    }
    SetCalculators(1, 1, 0, 1, 1);
    pILab2 = CreateLab2();
    if (pILab2 == 0) {
        Check(0, "Тест 3: компонент Eco.Lab2 не создан");
        return;
    }

    /* Свойства QueryInterface */
    QueryMatrix((IEcoUnknown*)pILab2, g_iidsLab2, g_namesLab2, 5, "QueryInterface из каждого интерфейса Eco.Lab2 в каждый (+ - успех и тот же указатель):");
    printf("\n    Единственный IEcoUnknown: запрос IEcoUnknown из любого интерфейса возвращает один и тот же указатель.\n");

    /* Интерфейсы калькулятора: включение E */
    pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
    pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoCalculatorY, (voidptr_t*)&pIY);
    if (pIX != 0 && pIY != 0) {
        /* X реализован включением E (не интерфейс B), Calculate совпадает с вызовом интерфейсов */
        bOk = pIX->pVTbl != &g_x9322111622484742AE0682819447843DVTblB;
        direct[0] = pIX->pVTbl->Addition(pIX, 100, 23);
        direct[1] = pIX->pVTbl->Subtraction(pIX, 100, 23);
        direct[2] = pIY->pVTbl->Multiplication(pIY, 100, 23);
        direct[3] = pIY->pVTbl->Division(pIY, 100, 23);
        for (i = 0; i < 4; i++) {
            result = pILab2->pVTbl->Calculate(pILab2, (uint16_t)i, 100, 23, &calc[i]);
            bOk = bOk && result == ERR_ECO_SUCCESS && calc[i] == direct[i];
        }
        printf("\n    Калькулятор Eco.Lab2 (включение E):  100+23 = %ld, 100-23 = %ld, 100*23 = %ld, 100/23 = %ld\n",
               (long)direct[0], (long)direct[1], (long)direct[2], (long)direct[3]);
        printf("    IEcoLab2::Calculate дает те же значения: %ld, %ld, %ld, %ld\n", (long)calc[0], (long)calc[1], (long)calc[2], (long)calc[3]);
        bOk = bOk && direct[0] == 123 && direct[1] == 77 && direct[2] == 2300 && direct[3] == 4;
        Check(bOk, "Тест 3: калькулятор Eco.Lab2 - не включение E, неверный результат или Calculate не совпадает с интерфейсами");
    }
    else {
        Check(0, "Тест 3: Eco.Lab2 не предоставил интерфейсы калькулятора");
    }
    if (pIY != 0) {
        pIY->pVTbl->Release(pIY);
    }
    if (pIX != 0) {
        pIX->pVTbl->Release(pIX);
    }

    /* IEcoAdvancedMath агрегированного компонента: тот же результат, что у отдельного Eco.AdvancedMath */
    pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoAdvancedMath, (voidptr_t*)&pIMathL2);
    pIMath = CreateMath();
    if (pIMathL2 != 0 && pIMath != 0) {
        bOk = pIMathL2->pVTbl->spline(pIMathL2, xs, ys, 6, 0, xq, 4, yqLab2) == ERR_ECO_SUCCESS &&
              pIMath->pVTbl->spline(pIMath, xs, ys, 6, 0, xq, 4, yqMath) == ERR_ECO_SUCCESS;
        err = 0;
        for (i = 0; i < 4; i++) {
            bOk = bOk && yqLab2[i] == yqMath[i];
            if (AbsD(yqLab2[i] - Cubic(xq[i])) > err) {
                err = AbsD(yqLab2[i] - Cubic(xq[i]));
            }
        }
        printf("\n    IEcoAdvancedMath через Eco.Lab2 (агрегирование): spline(x, x^3 - 2x) в 4 точках совпадает с отдельным\n");
        printf("    Eco.AdvancedMath бит в бит, max |S - p| = %.2e\n", err);
        Check(bOk && err <= 1e-12, "Тест 3: spline через Eco.Lab2 вернул ошибку, отличается от Eco.AdvancedMath или не воспроизвел многочлен");

        /* Общий счетчик ссылок: AddRef агрегированного интерфейса увеличивает счетчик Eco.Lab2 */
        r1 = pILab2->pVTbl->AddRef(pILab2);
        r2 = pIMathL2->pVTbl->AddRef(pIMathL2);
        printf("    Счетчик ссылок: AddRef через IEcoLab2 -> %lu, через IEcoAdvancedMath -> %lu (общий счетчик Eco.Lab2)\n", (unsigned long)r1, (unsigned long)r2);
        Check(r2 == r1 + 1, "Тест 3: агрегированный интерфейс должен изменять счетчик ссылок внешнего компонента");
        pIMathL2->pVTbl->Release(pIMathL2);
        pILab2->pVTbl->Release(pILab2);
    }
    else {
        Check(0, "Тест 3: не получен IEcoAdvancedMath");
    }
    if (pIMath != 0) {
        pIMath->pVTbl->Release(pIMath);
    }
    if (pIMathL2 != 0) {
        pIMathL2->pVTbl->Release(pIMathL2);
    }

    /* IEcoLab2::SplineMean: среднее значение x^3 - 2x на [0, 2] равно (4 - 4) / 2 = 0, на [1, 3] - (20 - 8) / 2 = 6 */
    result = pILab2->pVTbl->SplineMean(pILab2, xs, ys, 6, 0.0, 2.0, &mean);
    bOk = result == ERR_ECO_SUCCESS && AbsD(mean - 0.0) <= 1e-12;
    printf("\n    IEcoLab2::SplineMean(x^3 - 2x, [0, 2]) = %.15f (точно 0)\n", mean);
    result = pILab2->pVTbl->SplineMean(pILab2, xs, ys, 6, 1.0, 3.0, &mean);
    bOk = bOk && result == ERR_ECO_SUCCESS && AbsD(mean - 6.0) <= 1e-12;
    Check(bOk, "Тест 3: SplineMean на [0, 2] или [1, 3] не совпал с точным значением");
    printf("    IEcoLab2::SplineMean(x^3 - 2x, [1, 3]) = %.15f (точно 6)\n", mean);

    Check(pILab2->pVTbl->Release(pILab2) == 0, "Тест 3: последний Release должен вернуть 0");
}


/* ========================================================================== */
/*   Тест 4. Eco.Lab2 без E: интерфейсы калькулятора агрегированного компонента */
/* ========================================================================== */

static void TestLab2WithoutE(void) {
    IEcoLab2* pILab2 = 0;
    IEcoLab2* pILab2FromX = 0;
    IEcoCalculatorX* pIX = 0;
    IEcoCalculatorY* pIY = 0;

    PrintSection("Тест 4. Eco.Lab2 без компонента E: интерфейсы калькулятора дает агрегированный Eco.AdvancedMath");

    SetCalculators(1, 1, 0, 1, 0);
    pILab2 = CreateLab2();
    if (pILab2 == 0) {
        Check(0, "Тест 4: компонент Eco.Lab2 не создан");
        return;
    }
    QueryMatrix((IEcoUnknown*)pILab2, g_iidsLab2, g_namesLab2, 5, "QueryInterface из каждого интерфейса Eco.Lab2 в каждый (X - интерфейс B, Y - включение D в Eco.AdvancedMath):");

    pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
    pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoCalculatorY, (voidptr_t*)&pIY);
    if (pIX != 0 && pIY != 0) {
        /* Двойное агрегирование: B агрегирован в Eco.AdvancedMath, а тот - в Eco.Lab2; X - интерфейс
           самого B, и запрос из него попадает к внешнему компоненту Eco.Lab2 */
        Check(pIX->pVTbl == &g_x9322111622484742AE0682819447843DVTblB &&
              pIX->pVTbl->QueryInterface(pIX, &IID_IEcoLab2, (voidptr_t*)&pILab2FromX) == ERR_ECO_SUCCESS && pILab2FromX == pILab2,
              "Тест 4: IEcoCalculatorX - не интерфейс B или из него не получен IEcoLab2 внешнего компонента");
        if (pILab2FromX != 0) {
            pILab2FromX->pVTbl->Release(pILab2FromX);
        }
        Check(pIX->pVTbl->Addition(pIX, -7, 30) == 23 && pIY->pVTbl->Division(pIY, -30, 7) == -4,
              "Тест 4: ожидалось Addition(-7, 30) = 23, Division(-30, 7) = -4");
        printf("\n    X - интерфейс B, агрегированного в Eco.AdvancedMath, который агрегирован в Eco.Lab2:\n");
        printf("    pIX->QueryInterface(IID_IEcoLab2) возвращает IEcoLab2 внешнего компонента; Addition(-7, 30) = %ld\n",
               (long)pIX->pVTbl->Addition(pIX, -7, 30));
    }
    else {
        Check(0, "Тест 4: Eco.Lab2 не предоставил интерфейсы калькулятора");
    }
    if (pIY != 0) {
        pIY->pVTbl->Release(pIY);
    }
    if (pIX != 0) {
        pIX->pVTbl->Release(pIX);
    }
    Check(pILab2->pVTbl->Release(pILab2) == 0, "Тест 4: последний Release должен вернуть 0");
}


/* ========================================================================== */
/*   Тест 5. Обработка ошибочных данных                                       */
/* ========================================================================== */

static void TestErrors(IEcoSystem1* pISys) {
    static const double_t xs[3] = {0.0, 1.0, 2.0};
    static const double_t ys[3] = {0.0, 1.0, 0.0};
    IEcoLab2* pILab2 = 0;
    IEcoAdvancedMath* pIMath = 0;
    IEcoCalculatorY* pIY = 0;
    IEcoUnknown* pIUnk = 0;
    IEcoUnknown* pIOuter = (IEcoUnknown*)pISys;
    voidptr_t pv = 0;
    voidptr_t pv2 = 0;
    int32_t value = 0;
    int32_t value2 = 0;
    double_t mean = 0;
    int16_t result = 0;

    PrintSection("Тест 5. Обработка ошибочных данных");

    SetCalculators(1, 1, 0, 1, 1);
    pILab2 = CreateLab2();
    pIMath = CreateMath();
    if (pILab2 == 0 || pIMath == 0) {
        Check(0, "Тест 5: компоненты не созданы");
        return;
    }

    pv = &value;
    pv2 = &value;
    CheckCode(pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoMemoryAllocator1, &pv), ERR_ECO_NOINTERFACE, "Eco.Lab2: QueryInterface чужого интерфейса");
    CheckCode(pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoLab2, &pv2), ERR_ECO_NOINTERFACE, "Eco.AdvancedMath (без внешнего): QueryInterface(IID_IEcoLab2)");
    Check(pv == 0 && pv2 == 0, "Тест 5: при ERR_ECO_NOINTERFACE выходной указатель должен быть обнулен");
    CheckCode(pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoLab2, 0), ERR_ECO_POINTER, "Eco.Lab2: QueryInterface, ppv = 0");

    pIUnk = 0;
    result = g_pIBus->pVTbl->QueryComponent(g_pIBus, &CID_EcoLab2, pIOuter, &IID_IEcoUnknown, (voidptr_t*)&pIUnk);
    CheckRejected(result, pIUnk, "агрегирование Eco.Lab2 отклонено (не поддерживается)");
    pIUnk = 0;
    result = g_pIBus->pVTbl->QueryComponent(g_pIBus, &CID_EcoAdvancedMath, pIOuter, &IID_IEcoAdvancedMath, (voidptr_t*)&pIUnk);
    CheckRejected(result, pIUnk, "агрегирование Eco.AdvancedMath с IID != IEcoUnknown отклонено");

    CheckCode(pILab2->pVTbl->Calculate(pILab2, 9, 1, 2, &value), ERR_ECOLAB2_BAD_OPERATION, "Calculate: неизвестная операция 9");
    CheckCode(pILab2->pVTbl->Calculate(pILab2, ECO_LAB2_OP_DIVISION, 7, 0, &value), ERR_ECOLAB2_DIVISION_BY_ZERO, "Calculate: деление на 0");
    CheckCode(pILab2->pVTbl->Calculate(pILab2, ECO_LAB2_OP_ADDITION, 1, 2, 0), ERR_ECO_POINTER, "Calculate: pResult = 0");
    CheckCode(pILab2->pVTbl->SplineMean(pILab2, xs, ys, 3, 1.0, 1.0, &mean), ERR_ECOLAB2_EMPTY_INTERVAL, "SplineMean: отрезок нулевой длины");
    CheckCode(pILab2->pVTbl->SplineMean(pILab2, xs, ys, 1, 0.0, 1.0, &mean), ERR_ECOADVANCEDMATH_TOO_FEW_POINTS, "SplineMean: n = 1 (код агрегированного Eco.AdvancedMath)");
    CheckCode(pILab2->pVTbl->SplineMean(pILab2, 0, ys, 3, 0.0, 1.0, &mean), ERR_ECO_POINTER, "SplineMean: x = 0");

    /* Деление на 0 через интерфейс Y, реализованный включением: компоненты курса не проверяют
       делитель, обертка включения возвращает 0 без деления */
    value = -1;
    value2 = -1;
    if (pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorY, (voidptr_t*)&pIY) == ERR_ECO_SUCCESS && pIY != 0) {
        value = pIY->pVTbl->Division(pIY, 7, 0);
        pIY->pVTbl->Release(pIY);
        pIY = 0;
    }
    if (pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoCalculatorY, (voidptr_t*)&pIY) == ERR_ECO_SUCCESS && pIY != 0) {
        value2 = pIY->pVTbl->Division(pIY, 7, 0);
        pIY->pVTbl->Release(pIY);
    }
    Check(value == 0 && value2 == 0, "Тест 5: Division(7, 0) через Eco.AdvancedMath и Eco.Lab2 должен вернуть 0");
    printf("    Division(7, 0) через интерфейсы включения Eco.AdvancedMath и Eco.Lab2 возвращает 0 без деления\n");

    pIMath->pVTbl->Release(pIMath);
    pILab2->pVTbl->Release(pILab2);
}


/* ========================================================================== */
/*   Тест 6. Многократное создание и освобождение                             */
/* ========================================================================== */

static void TestLifetime(void) {
    static const int sets[3][5] = {{1, 0, 0, 1, 1}, {1, 0, 0, 0, 1}, {0, 0, 1, 0, 0}};
    IEcoLab2* pILab2 = 0;
    IEcoAdvancedMath* pIMath = 0;
    IEcoCalculatorX* pIX = 0;
    IEcoCalculatorY* pIY = 0;
    IEcoMemoryManager1* pIMgr = 0;
    ECOMEMORYMANAGER1STATUS before = {0, 0, 0, 0, 0};
    ECOMEMORYMANAGER1STATUS after = {0, 0, 0, 0, 0};
    int32_t value = 0;
    uint32_t i = 0;
    uint32_t k = 0;
    uint32_t nOk = 0;
    uint32_t nOkMath = 0;
    const uint32_t n = 2000;
    const uint32_t nMath = 500;

    PrintSection("Тест 6. Многократное создание и освобождение компонентов, утечки памяти");

    /* Состояние менеджера памяти Eco: число занятых блоков до и после циклов */
    g_pIBus->pVTbl->QueryComponent(g_pIBus, &CID_EcoMemoryManager1, 0, &IID_IEcoMemoryManager1, (voidptr_t*)&pIMgr);
    if (pIMgr != 0) {
        pIMgr->pVTbl->get_Status(pIMgr, &before);
    }
    else {
        Check(0, "Тест 6: не получен интерфейс менеджера памяти");
    }

    SetCalculators(1, 1, 0, 1, 1);
    for (i = 0; i < n; i++) {
        pILab2 = CreateLab2();
        if (pILab2 == 0) {
            break;
        }
        pIMath = 0;
        pIX = 0;
        pILab2->pVTbl->QueryInterface(pILab2, &IID_IEcoAdvancedMath, (voidptr_t*)&pIMath);
        if (pIMath != 0) {
            pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
        }
        if (pIX != 0) {
            value = pIX->pVTbl->Addition(pIX, (int16_t)i, 1);
            pIX->pVTbl->Release(pIX);
        }
        if (pIMath != 0) {
            pIMath->pVTbl->Release(pIMath);
        }
        if (pILab2->pVTbl->Release(pILab2) == 0 && value == (int32_t)i + 1) {
            nOk++;
        }
    }
    printf("    Eco.Lab2 (Eco.AdvancedMath с B и D, E с B): %lu циклов создания, запросов, вызова и освобождения - успешно %lu\n",
           (unsigned long)n, (unsigned long)nOk);
    Check(nOk == n, "Тест 6: не все циклы создания и освобождения Eco.Lab2 выполнены");

    /* Eco.AdvancedMath с включением A, D, E и C */
    for (k = 0; k < 3; k++) {
        SetCalculators(sets[k][0], sets[k][1], sets[k][2], sets[k][3], sets[k][4]);
        for (i = 0; i < nMath; i++) {
            pIMath = CreateMath();
            if (pIMath == 0) {
                break;
            }
            pIX = 0;
            pIY = 0;
            pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorX, (voidptr_t*)&pIX);
            pIMath->pVTbl->QueryInterface(pIMath, &IID_IEcoCalculatorY, (voidptr_t*)&pIY);
            if (pIX != 0 && pIY != 0 && pIX->pVTbl->Addition(pIX, 2, 3) == 5 && pIY->pVTbl->Multiplication(pIY, 2, 3) == 6) {
                nOkMath++;
            }
            if (pIY != 0) {
                pIY->pVTbl->Release(pIY);
            }
            if (pIX != 0) {
                pIX->pVTbl->Release(pIX);
            }
            pIMath->pVTbl->Release(pIMath);
        }
    }
    printf("    Eco.AdvancedMath (включение A и D; A и E; C): %lu циклов - успешно %lu\n", (unsigned long)(3 * nMath), (unsigned long)nOkMath);
    Check(nOkMath == 3 * nMath, "Тест 6: не все циклы создания и освобождения Eco.AdvancedMath выполнены");
    SetCalculators(1, 1, 0, 1, 1);

    if (pIMgr != 0) {
        pIMgr->pVTbl->get_Status(pIMgr, &after);
        printf("    Занятых блоков менеджера памяти Eco: до циклов %lu, после %lu%s\n",
               (unsigned long)before.usedBlocks, (unsigned long)after.usedBlocks,
               after.usedBlocks == before.usedBlocks ? " - утечек нет" : " - УТЕЧКА");
        Check(after.usedBlocks == before.usedBlocks, "Тест 6: число занятых блоков памяти изменилось (утечка)");
        pIMgr->pVTbl->Release(pIMgr);
    }
}


/*
 *
 * <summary>
 *   EcoMain Function
 * </summary>
 *
 * <description>
 *   EcoMain function - entry point
 * </description>
 *
 */
int16_t EcoMain(IEcoUnknown* pIUnk) {
    int16_t result = -1;
    /* Pointer to the system interface */
    IEcoSystem1* pISys = 0;
    /* Pointer to the interface for working with the system interface bus */
    IEcoInterfaceBus1* pIBus = 0;

    /* System interface check and creation */
    result = pIUnk->pVTbl->QueryInterface(pIUnk, &GID_IEcoSystem, (void **)&pISys);
    if (result != 0 || pISys == 0) {
        goto Release;
    }

    /* Getting the interface for working with the interface bus */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);
    if (result != 0 || pIBus == 0) {
        goto Release;
    }
    g_pIBus = pIBus;

    /* Registration of the calculator components (switch factories) */
    result = RegisterCalculators();
    if (result != 0 ) {
        printf("Не удалось зарегистрировать компоненты-калькуляторы (код %d)\n", (int)result);
        goto Release;
    }
#ifdef ECO_LIB
    /* Registration of the static components Eco.AdvancedMath and Eco.Lab2 */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoAdvancedMath, (IEcoUnknown*)GetIEcoComponentFactoryPtr_A2A2065C8C3B4926BC6C30A84CAA1B2F);
    if (result != 0 ) {
        goto Release;
    }
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoLab2, (IEcoUnknown*)GetIEcoComponentFactoryPtr_1EF402B8755C4289BBE1795A2F9385F6);
    if (result != 0 ) {
        goto Release;
    }
#endif

    printf("Юнит-тест лабораторной работы 2: включение и агрегирование компонентов\n");
    printf("Eco.AdvancedMath (лабораторная работа 1) и Eco.Lab2 с компонентами-калькуляторами A, B, C, D, E\n");

    TestTaskExample();
    TestProviders();
    TestLab2();
    TestLab2WithoutE();
    TestErrors(pISys);
    TestLifetime();

    PrintSection("Итог");
    printf("Проверок: %lu, успешно: %lu, с ошибками: %lu\n", (unsigned long)g_nChecks, (unsigned long)(g_nChecks - g_nFailed), (unsigned long)g_nFailed);
    printf("%s\n", g_nFailed == 0 ? "ВСЕ ТЕСТЫ ПРОЙДЕНЫ" : "ЕСТЬ ОШИБКИ");
    result = g_nFailed == 0 ? ERR_ECO_SUCCESS : ERR_ECO_FAIL;

Release:

    /* Free the interface for working with the interface bus */
    if (pIBus != 0) {
        pIBus->pVTbl->Release(pIBus);
    }

    /* Free the system interface */
    if (pISys != 0) {
        pISys->pVTbl->Release(pISys);
    }

    return result;
}
