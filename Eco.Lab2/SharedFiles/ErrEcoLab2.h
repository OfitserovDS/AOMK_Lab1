/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   ErrEcoLab2
 * </summary>
 *
 * <description>
 *   Коды ошибок компонента Eco.Lab2 (дополняют общие коды ERR_ECO_* из ErrEcoCodes.h
 *   и коды ERR_ECOADVANCEDMATH_* агрегированного компонента Eco.AdvancedMath).
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __ERR_ECOLAB2_H__
#define __ERR_ECOLAB2_H__

/*
 * <error code>
 *   ERR_ECOLAB2_BAD_OPERATION
 * </error code>
 *
 * <description>
 *   Неизвестный код операции метода Calculate (см. ECO_LAB2_OPERATION).
 * </description>
 */
#define ERR_ECOLAB2_BAD_OPERATION       ((int16_t)2001)

/*
 * <error code>
 *   ERR_ECOLAB2_DIVISION_BY_ZERO
 * </error code>
 *
 * <description>
 *   Деление на ноль в методе Calculate.
 * </description>
 */
#define ERR_ECOLAB2_DIVISION_BY_ZERO    ((int16_t)2002)

/*
 * <error code>
 *   ERR_ECOLAB2_EMPTY_INTERVAL
 * </error code>
 *
 * <description>
 *   Отрезок метода SplineMean имеет нулевую длину (a == b).
 * </description>
 */
#define ERR_ECOLAB2_EMPTY_INTERVAL      ((int16_t)2003)

#endif /* __ERR_ECOLAB2_H__ */
