/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   ErrEcoAdvancedMath
 * </summary>
 *
 * <description>
 *   Коды ошибок компонента EcoAdvancedMath (дополняют общие коды ERR_ECO_* из ErrEcoCodes.h).
 * </description>
 *
 * <reference>
 *
 * </reference>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __ERR_ECOADVANCEDMATH_H__
#define __ERR_ECOADVANCEDMATH_H__

/*
 * <error code>
 *   ERR_ECOADVANCEDMATH_TOO_FEW_POINTS
 * </error code>
 *
 * <description>
 *   Для построения сплайна нужно не менее двух узлов (n >= 2).
 * </description>
 */
#define ERR_ECOADVANCEDMATH_TOO_FEW_POINTS      ((int16_t)1001)

/*
 * <error code>
 *   ERR_ECOADVANCEDMATH_NOT_INCREASING
 * </error code>
 *
 * <description>
 *   Узлы интерполяции x должны строго возрастать: x[i] < x[i+1].
 * </description>
 */
#define ERR_ECOADVANCEDMATH_NOT_INCREASING      ((int16_t)1002)

/*
 * <error code>
 *   ERR_ECOADVANCEDMATH_NOT_FINITE
 * </error code>
 *
 * <description>
 *   Во входных данных (x, y или значениях граничных условий) есть NaN или бесконечность.
 * </description>
 */
#define ERR_ECOADVANCEDMATH_NOT_FINITE          ((int16_t)1003)

/*
 * <error code>
 *   ERR_ECOADVANCEDMATH_BAD_END_CONDITION
 * </error code>
 *
 * <description>
 *   Неизвестный тип граничного условия (см. ECO_SPLINE_END_TYPE).
 * </description>
 */
#define ERR_ECOADVANCEDMATH_BAD_END_CONDITION   ((int16_t)1004)

/*
 * <error code>
 *   ERR_ECOADVANCEDMATH_SINGULAR
 * </error code>
 *
 * <description>
 *   В методе прогонки получен нулевой знаменатель (система вырождена).
 *   При корректных входных данных не возникает - защитная проверка.
 * </description>
 */
#define ERR_ECOADVANCEDMATH_SINGULAR            ((int16_t)1005)

/*
 * <error code>
 *   ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL
 * </error code>
 *
 * <description>
 *   Размер переданного выходного буфера меньше требуемого.
 * </description>
 */
#define ERR_ECOADVANCEDMATH_BUFFER_TOO_SMALL    ((int16_t)1006)


#endif /* __ERR_ECOADVANCEDMATH_H__ */
