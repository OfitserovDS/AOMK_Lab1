/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IEcoLab2
 * </summary>
 *
 * <description>
 *   Интерфейс IEcoLab2 компонента Eco.Lab2 (лабораторная работа 2).
 *
 *   Eco.Lab2 - внешний компонент, который повторно использует готовые компоненты:
 *     - агрегирует компонент Eco.AdvancedMath (лабораторная работа 1): интерфейс
 *       IEcoAdvancedMath внутреннего компонента передается клиенту напрямую;
 *     - включает компонент калькулятора E: интерфейсы IEcoCalculatorX (сложение, вычитание)
 *       и IEcoCalculatorY (умножение, деление) реализованы вызовами компонента E, внутри
 *       которого агрегирован компонент B. Если E недоступен, интерфейсы калькулятора
 *       предоставляет агрегированный Eco.AdvancedMath.
 *   Через любой интерфейс компонента методом QueryInterface можно получить любой другой.
 *
 *   Собственные методы IEcoLab2 используют внутренние компоненты:
 *     Calculate  - арифметическая операция через интерфейсы калькулятора компонента;
 *     SplineMean - среднее значение кубического сплайна на отрезке через агрегированный
 *                  Eco.AdvancedMath (IEcoAdvancedMath::new_Spline и IEcoSpline::Integrate).
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __I_ECOLAB2_H__
#define __I_ECOLAB2_H__

#include "IEcoBase1.h"

/* Операции метода IEcoLab2::Calculate */
typedef enum ECO_LAB2_OPERATION {
    ECO_LAB2_OP_ADDITION       = 0,   /* a + b (IEcoCalculatorX::Addition) */
    ECO_LAB2_OP_SUBTRACTION    = 1,   /* a - b (IEcoCalculatorX::Subtraction) */
    ECO_LAB2_OP_MULTIPLICATION = 2,   /* a * b (IEcoCalculatorY::Multiplication) */
    ECO_LAB2_OP_DIVISION       = 3    /* a / b (IEcoCalculatorY::Division), целочисленное деление */
} ECO_LAB2_OPERATION;

/* IEcoLab2 IID = {2D766564-6D5B-49FF-9E29-65E281F70DAF} */
#ifndef __IID_IEcoLab2
static const UGUID IID_IEcoLab2 = {0x01, 0x10, {0x2D, 0x76, 0x65, 0x64, 0x6D, 0x5B, 0x49, 0xFF, 0x9E, 0x29, 0x65, 0xE2, 0x81, 0xF7, 0x0D, 0xAF}};
#endif /* __IID_IEcoLab2 */

typedef struct IEcoLab2* IEcoLab2Ptr_t;

typedef struct IEcoLab2VTbl {

    /* IEcoUnknown */
    int16_t (ECOCALLMETHOD *QueryInterface)(/* in */ IEcoLab2Ptr_t me, /* in */ const UGUID* riid, /* out */ voidptr_t* ppv);
    uint32_t (ECOCALLMETHOD *AddRef)(/* in */ IEcoLab2Ptr_t me);
    uint32_t (ECOCALLMETHOD *Release)(/* in */ IEcoLab2Ptr_t me);

    /* IEcoLab2 */

    /* *pResult = a op b, op - ECO_LAB2_OPERATION; операцию выполняет калькулятор компонента */
    int16_t (ECOCALLMETHOD *Calculate)(/* in */ IEcoLab2Ptr_t me, /* in */ uint16_t op, /* in */ int16_t a, /* in */ int16_t b, /* out */ int32_t* pResult);

    /* *pMean = интеграл S(x) dx от a до b, деленный на (b - a); S - кубический сплайн (not-a-knot) по n узлам (x, y) */
    int16_t (ECOCALLMETHOD *SplineMean)(/* in */ IEcoLab2Ptr_t me, /* in */ const double_t* x, /* in */ const double_t* y, /* in */ uint32_t n, /* in */ double_t a, /* in */ double_t b, /* out */ double_t* pMean);

} IEcoLab2VTbl, *IEcoLab2VTblPtr_t;

interface IEcoLab2 {
    struct IEcoLab2VTbl *pVTbl;
} IEcoLab2;

#endif /* __I_ECOLAB2_H__ */
