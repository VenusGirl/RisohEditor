// idmaps.cpp --- ID mappings
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2021 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "MWindowBase.hpp"
#include "RisohSettings.hpp"
#include "MString.hpp"
#include <unordered_map>
#include "resource.h"

std::unordered_map<INT, MStringW> *g_pmapIDTypeToLocalized = NULL;
std::unordered_map<MStringW, INT> *g_pmapLocalizedToIDType = NULL;

void InitIDTypeMaps()
{
	g_pmapIDTypeToLocalized = new std::unordered_map<INT, MStringW>();
	g_pmapLocalizedToIDType = new std::unordered_map<MStringW, INT>();

	MStringW s;
#define DEFINE_IDTYPE(index, idtype, str, wstr, ids, prefix) do { \
	s = LoadStringDx(ids); \
	(*g_pmapIDTypeToLocalized)[index] = s; \
	(*g_pmapLocalizedToIDType)[s] = index; \
} while (0);
#include "idtypes.h"
#undef DEFINE_IDTYPE
}

void UnloadIDTypeMaps()
{
	delete g_pmapIDTypeToLocalized;
	g_pmapIDTypeToLocalized = NULL;
	delete g_pmapLocalizedToIDType;
	g_pmapLocalizedToIDType = NULL;
}

MStringW MapIDType(IDTYPE_ nIDType)
{
	auto it = (*g_pmapIDTypeToLocalized).find(nIDType);
	if (it != (*g_pmapIDTypeToLocalized).end())
		return it->second;
	return (*g_pmapIDTypeToLocalized)[0];
}

IDTYPE_ UnMapIDType(const MStringW& str)
{
	auto it = (*g_pmapLocalizedToIDType).find(str);
	if (it != (*g_pmapLocalizedToIDType).end())
		return (IDTYPE_)it->second;
	return IDTYPE_UNKNOWN;
}
