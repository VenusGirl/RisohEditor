#pragma once

#include <unordered_map>
#include "RisohSettings.hpp"

extern std::unordered_map<INT, MStringW> *g_pmapIDTypeToLocalized;
extern std::unordered_map<MStringW, INT> *g_pmapLocalizedToIDType;

void InitIDTypeMaps();
void UnloadIDTypeMaps();
MStringW MapIDType(IDTYPE_ nIDType);
IDTYPE_ UnMapIDType(const MStringW& str);
