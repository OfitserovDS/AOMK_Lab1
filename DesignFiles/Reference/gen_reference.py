#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Генератор эталонных данных для юнит-теста компонента EcoAdvancedMath.

Эталон - scipy.interpolate.CubicSpline. При bc_type='not-a-knot' SciPy строит тот же
сплайн, что и функция spline MATLAB (те же уравнения для наклонов, вырожденные случаи
n = 2 - прямая, n = 3 - парабола). Значения записываются с 17 значащими цифрами, что
однозначно задает число double.

Результат: UnitTestFiles/HeaderFiles/EcoAdvancedMathReference.h

Запуск (из корня проекта):
    python3 DesignFiles/Reference/gen_reference.py
"""

import os
import numpy as np
import scipy
from scipy.interpolate import CubicSpline

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "UnitTestFiles", "HeaderFiles", "EcoAdvancedMathReference.h")


def c_array(name, values, ctype="double_t"):
    vals = ", ".join("%.17g" % float(v) for v in values)
    return "static const %s %s[%d] = {%s};\n" % (ctype, name, len(values), vals)


def c_scalar(name, value):
    return "#define %s (%.17g)\n" % (name, float(value))


parts = []

# --------------------------------------------------------------------------
# Набор A: пример из документации MATLAB:  x = 0:10; y = sin(x); xx = 0:.25:10
# --------------------------------------------------------------------------
xa = np.arange(0.0, 11.0)
ya = np.sin(xa)
xqa = np.arange(0.0, 10.0 + 0.125, 0.25)
spa = CubicSpline(xa, ya, bc_type="not-a-knot")
parts.append("/* Набор A: x = 0:10, y = sin(x), xq = 0:0.25:10 (пример из документации MATLAB spline) */\n")
parts.append("#define REF_A_N  %d\n#define REF_A_M  %d\n" % (len(xa), len(xqa)))
parts.append(c_array("g_RefA_X", xa))
parts.append(c_array("g_RefA_Y", ya))
parts.append(c_array("g_RefA_XQ", xqa))
parts.append(c_array("g_RefA_YQ", spa(xqa)))
parts.append(c_array("g_RefA_D1", spa(xqa, 1)))
parts.append(c_array("g_RefA_D2", spa(xqa, 2)))
parts.append(c_array("g_RefA_D3", spa(xqa, 3)))
# коэффициенты кусков: SciPy хранит c[порядок, кусок], порядок 0 - при t^3
coefs = []
for k in range(len(xa) - 1):
    coefs.extend([spa.c[0, k], spa.c[1, k], spa.c[2, k], spa.c[3, k]])
parts.append(c_array("g_RefA_Coefs", coefs))
# определенные интегралы (включая выход за пределы узлов и a > b)
ints = [(0.0, 10.0), (2.5, 7.25), (-1.0, 11.0), (3.0, 3.0), (8.0, 1.0), (0.0, np.pi)]
parts.append("#define REF_A_NINT %d\n" % len(ints))
parts.append(c_array("g_RefA_IntA", [a for a, _ in ints]))
parts.append(c_array("g_RefA_IntB", [b for _, b in ints]))
parts.append(c_array("g_RefA_IntV", [spa.integrate(a, b, extrapolate=True) for a, b in ints]))
parts.append("\n")

# --------------------------------------------------------------------------
# Набор B: неравномерная сетка, все типы граничных условий и их сочетания
# --------------------------------------------------------------------------
xb = np.array([0.0, 0.3, 1.1, 1.5, 2.6, 3.0, 4.2, 5.0])
yb = np.exp(-xb / 2.0) * np.cos(2.0 * xb)
xqb = np.array([-0.5, 0.0, 0.15, 0.3, 0.7, 1.1, 1.3, 2.0, 2.6, 2.8, 3.6, 4.2, 4.6, 5.0, 5.5])
# (код типа левого конца, значение, код типа правого конца, значение) - коды ECO_SPLINE_END_*
cases_b = [
    ("NAK_NAK",   (0, 0.0),  (0, 0.0),  "not-a-knot"),
    ("NAT_NAT",   (1, 0.0),  (1, 0.0),  "natural"),
    ("CLA_CLA",   (2, 1.0),  (2, -0.5), ((1, 1.0), (1, -0.5))),
    ("SEC_SEC",   (3, 2.0),  (3, -1.0), ((2, 2.0), (2, -1.0))),
    ("CLA_NAT",   (2, 0.0),  (1, 0.0),  ((1, 0.0), "natural")),
    ("NAK_SEC",   (0, 0.0),  (3, 0.5),  ("not-a-knot", (2, 0.5))),
]
parts.append("/* Набор B: неравномерные узлы, y = exp(-x/2)*cos(2x); граничные условия (тип, значение) */\n")
parts.append("#define REF_B_N  %d\n#define REF_B_M  %d\n#define REF_B_NCASES %d\n" % (len(xb), len(xqb), len(cases_b)))
parts.append(c_array("g_RefB_X", xb))
parts.append(c_array("g_RefB_Y", yb))
parts.append(c_array("g_RefB_XQ", xqb))
parts.append("static const uint16_t g_RefB_LeftType[%d] = {%s};\n" % (len(cases_b), ", ".join(str(c[1][0]) for c in cases_b)))
parts.append("static const uint16_t g_RefB_RightType[%d] = {%s};\n" % (len(cases_b), ", ".join(str(c[2][0]) for c in cases_b)))
parts.append(c_array("g_RefB_LeftValue", [c[1][1] for c in cases_b]))
parts.append(c_array("g_RefB_RightValue", [c[2][1] for c in cases_b]))
parts.append("static const char_t* g_RefB_Name[%d] = {%s};\n" % (len(cases_b), ", ".join('"%s"' % c[0] for c in cases_b)))
yqb = []
for name, left, right, bc in cases_b:
    sp = CubicSpline(xb, yb, bc_type=bc)
    yqb.extend(sp(xqb))
parts.append(c_array("g_RefB_YQ", yqb))
parts.append("\n")

# --------------------------------------------------------------------------
# Набор C: вырожденные случаи малого числа узлов
# --------------------------------------------------------------------------
cases_c = [
    ("n=2 not-a-knot (line)",       [1.0, 3.0],            [2.0, -1.0],            "not-a-knot",               (0, 0.0), (0, 0.0)),
    ("n=3 not-a-knot (parabola)",   [0.0, 1.0, 3.0],       [1.0, 3.0, 2.0],        "not-a-knot",               (0, 0.0), (0, 0.0)),
    ("n=4 not-a-knot (one cubic)",  [0.0, 1.0, 2.5, 4.0],  [1.0, -1.0, 2.0, 0.5],  "not-a-knot",               (0, 0.0), (0, 0.0)),
    ("n=2 clamped (Hermite)",       [0.0, 2.0],            [0.0, 1.0],             ((1, 2.0), (1, -1.0)),      (2, 2.0), (2, -1.0)),
    ("n=3 natural",                 [0.0, 1.0, 3.0],       [1.0, 3.0, 2.0],        "natural",                  (1, 0.0), (1, 0.0)),
    ("n=2 not-a-knot + clamped",    [0.0, 2.0],            [0.0, 1.0],             ("not-a-knot", (1, -1.0)),  (0, 0.0), (2, -1.0)),
    ("n=3 not-a-knot + second",     [0.0, 1.0, 3.0],       [1.0, 3.0, 2.0],        ("not-a-knot", (2, 1.5)),   (0, 0.0), (3, 1.5)),
]
xqc = np.array([-1.0, 0.0, 0.5, 1.0, 1.75, 2.0, 2.5, 3.0, 4.0, 5.0])
parts.append("/* Набор C: вырожденные случаи n = 2, 3, 4 (узлы дополнены до 4 элементов) */\n")
parts.append("#define REF_C_NCASES %d\n#define REF_C_M %d\n" % (len(cases_c), len(xqc)))
parts.append("static const uint32_t g_RefC_N[%d] = {%s};\n" % (len(cases_c), ", ".join(str(len(c[1])) for c in cases_c)))
xc_flat, yc_flat, yqc = [], [], []
for name, xs, ys, bc, left, right in cases_c:
    sp = CubicSpline(np.array(xs), np.array(ys), bc_type=bc)
    xc_flat.extend(xs + [0.0] * (4 - len(xs)))
    yc_flat.extend(ys + [0.0] * (4 - len(ys)))
    yqc.extend(sp(xqc))
parts.append(c_array("g_RefC_X", xc_flat))
parts.append(c_array("g_RefC_Y", yc_flat))
parts.append("static const uint16_t g_RefC_LeftType[%d] = {%s};\n" % (len(cases_c), ", ".join(str(c[4][0]) for c in cases_c)))
parts.append("static const uint16_t g_RefC_RightType[%d] = {%s};\n" % (len(cases_c), ", ".join(str(c[5][0]) for c in cases_c)))
parts.append(c_array("g_RefC_LeftValue", [c[4][1] for c in cases_c]))
parts.append(c_array("g_RefC_RightValue", [c[5][1] for c in cases_c]))
parts.append("static const char_t* g_RefC_Name[%d] = {%s};\n" % (len(cases_c), ", ".join('"%s"' % c[0] for c in cases_c)))
parts.append(c_array("g_RefC_XQ", xqc))
parts.append(c_array("g_RefC_YQ", yqc))

header = """/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   EcoAdvancedMathReference
 * </summary>
 *
 * <description>
 *   Эталонные значения для юнит-теста компонента EcoAdvancedMath.
 *   Файл сгенерирован скриптом DesignFiles/Reference/gen_reference.py
 *   (SciPy %s, NumPy %s, scipy.interpolate.CubicSpline). Не редактировать вручную.
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __ECOADVANCEDMATH_REFERENCE_H__
#define __ECOADVANCEDMATH_REFERENCE_H__

""" % (scipy.__version__, np.__version__)

# Файлы проекта: UTF-8 с сигнатурой (BOM) и окончания строк CRLF (требование стилистики Eco)
with open(OUT, "w", encoding="utf-8-sig", newline="\r\n") as f:
    f.write(header)
    f.write("".join(parts))
    f.write("\n#endif /* __ECOADVANCEDMATH_REFERENCE_H__ */\n")
print("written", OUT)
