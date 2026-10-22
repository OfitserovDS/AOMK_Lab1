/*
 * <character encoding>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </character encoding>
 *
 * <summary>
 *   IdEcoLab2
 * </summary>
 *
 * <description>
 *   Идентификатор компонента Eco.Lab2 и функция получения его фабрики
 * </description>
 *
 * <author>
 *   Copyright (c) 2026 Офицеров Д.С. All rights reserved.
 * </author>
 *
 */

#ifndef __ID_ECOLAB2_H__
#define __ID_ECOLAB2_H__

#include "IEcoBase1.h"
#include "IEcoLab2.h"

/* EcoLab2 CID = {1EF402B8-755C-4289-BBE1-795A2F9385F6} */
#ifndef __CID_EcoLab2
static const UGUID CID_EcoLab2 = {0x01, 0x10, {0x1E, 0xF4, 0x02, 0xB8, 0x75, 0x5C, 0x42, 0x89, 0xBB, 0xE1, 0x79, 0x5A, 0x2F, 0x93, 0x85, 0xF6}};
#endif /* __CID_EcoLab2 */

/* Component factory for dynamic and static layout */
#ifdef ECO_DLL
ECO_EXPORT IEcoComponentFactory* ECOCALLMETHOD GetIEcoComponentFactoryPtr();
#elif ECO_LIB
extern IEcoComponentFactory* GetIEcoComponentFactoryPtr_1EF402B8755C4289BBE1795A2F9385F6;
#endif

#endif /* __ID_ECOLAB2_H__ */
