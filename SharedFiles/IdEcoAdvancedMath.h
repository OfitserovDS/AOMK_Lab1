/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IdEcoAdvancedMath
 * </summary>
 *
 * <description>
 *   This header describes the interface IdEcoAdvancedMath
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

#ifndef __ID_ECOADVANCEDMATH_H__
#define __ID_ECOADVANCEDMATH_H__

#include "IEcoBase1.h"
#include "IEcoAdvancedMath.h"

/* EcoAdvancedMath CID = {A2A2065C-8C3B-4926-BC6C-30A84CAA1B2F} */
#ifndef __CID_EcoAdvancedMath
static const UGUID CID_EcoAdvancedMath = {0x01, 0x10, {0xA2, 0xA2, 0x06, 0x5C, 0x8C, 0x3B, 0x49, 0x26, 0xBC, 0x6C, 0x30, 0xA8, 0x4C, 0xAA, 0x1B, 0x2F}};
#endif /* __CID_EcoAdvancedMath */

/* Component factory for dynamic and static layout */
#ifdef ECO_DLL
ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr();
#elif ECO_LIB
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_A2A2065C8C3B4926BC6C30A84CAA1B2F;
#endif

#endif /* __ID_ECOADVANCEDMATH_H__ */

