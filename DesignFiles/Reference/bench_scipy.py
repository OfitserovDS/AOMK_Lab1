#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Замер производительности эталонной реализации scipy.interpolate.CubicSpline
на тех же данных, что и тест 9.2 компонента EcoAdvancedMath:
    x[i] = 10*(i + 0.3*sin(0.7*i))/n,  y = sin(x)*exp(-x/10),  xq[i] = 10*i/m,  m = n.
Измеряется процессорное время (time.process_time, аналог clock() в C) построения
сплайна и вычисления в m точках (аналог одного вызова spline(x, y, xq)).

Запуск: python3 DesignFiles/Reference/bench_scipy.py
"""

import time
import numpy as np
import scipy
from scipy.interpolate import CubicSpline

MIN_SECONDS = 0.25


def bench(fn):
    reps = 0
    start = time.process_time()
    while True:
        fn()
        reps += 1
        elapsed = time.process_time() - start
        if elapsed >= MIN_SECONDS:
            return elapsed / reps


def main():
    print("SciPy %s, NumPy %s" % (scipy.__version__, np.__version__))
    print("%8s  %16s  %16s  %16s  %10s" % ("n", "build+eval, ms", "build, ms", "eval, ms", "ns/point"))
    for n in (1000, 10000, 100000, 1000000):
        i = np.arange(n, dtype=np.float64)
        x = 10.0 * (i + 0.3 * np.sin(0.7 * i)) / n
        y = np.sin(x) * np.exp(-x / 10.0)
        xq = 10.0 * np.arange(n, dtype=np.float64) / n
        t_all = bench(lambda: CubicSpline(x, y)(xq))
        t_build = bench(lambda: CubicSpline(x, y))
        sp = CubicSpline(x, y)
        t_eval = bench(lambda: sp(xq))
        print("%8d  %16.3f  %16.3f  %16.3f  %10.1f" % (n, t_all * 1e3, t_build * 1e3, t_eval * 1e3, t_all * 1e9 / n))


if __name__ == "__main__":
    main()
