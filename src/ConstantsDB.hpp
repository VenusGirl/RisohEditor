// ConstantsDB.hpp --- Constants Database
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#pragma once

#include <vector>
#include <unordered_map>

#include "MString.hpp"
#include "MIdOrString.hpp"
#include "RisohSettings.hpp"

class ConstantsDB
{
public:
	typedef std::wstring StringType;
	typedef StringType NameType;
	typedef StringType CategoryType;
	typedef DWORD ValueType;
	typedef std::vector<ValueType> ValuesType;

	struct EntryType
	{
		NameType    name;
		ValueType   value;
		ValueType   mask;

		EntryType(NameType name_, ValueType value_)
			: name(name_), value(value_), mask(value_)
		{
		}
		EntryType(NameType name_, ValueType value_, ValueType mask_)
			: name(name_), value(value_), mask(mask_)
		{
		}
	};
	typedef std::vector<EntryType> TableType;
	typedef std::unordered_map<CategoryType, TableType> MapType;
	MapType m_map;

	ConstantsDB() = default;

	TableType GetTable(CategoryType category) const;
	TableType GetTableByPrefix(CategoryType category, NameType prefix) const;

	TableType GetConstantTable() const;
	BOOL GetValueOfName(const NameType& name, ValueType& value) const;

	ValueType GetResIDValue(const NameType& name) const;
	ValueType GetCtrlIDValue(const NameType& name) const;

	bool HasCtrlID(const NameType& name) const;
	bool HasResID(const NameType& name) const;

	StringType GetNameOfResID(IDTYPE_ nIDTYPE_, ValueType value, bool unsign = false) const;
	StringType GetNameOfIDTypeValue(IDTYPE_ nIDTYPE_, ValueType value) const;
	StringType GetCtrlOrCmdName(ValueType value, bool unsign = false) const;
	StringType GetNameOfResID(IDTYPE_ nIDTYPE_1, IDTYPE_ nIDTYPE_2, ValueType value,
		                      bool unsign = false) const;

	NameType GetName(const CategoryType& category, ValueType value) const;
	NameType GetLangName(ValueType value) const;
	ValueType GetValue(const CategoryType& category, const NameType& name) const;
	ValueType GetValueI(const CategoryType& category, const NameType& name) const;
	ValuesType GetValues(const CategoryType& category, const NameType& name) const;

	bool LoadFromFile(LPCWSTR FileName);

	StringType DumpBitField(const CategoryType& cat1, ValueType& value,
							ValueType default_value = 0) const;
	StringType DumpBitFieldOrZero(const CategoryType& cat1, ValueType& value,
								  ValueType default_value = 0) const;
	StringType DumpBitField(const CategoryType& cat1, const CategoryType& cat2,
							ValueType& value, ValueType default_value = 0) const;
	StringType DumpBitFieldOrZero(const CategoryType& cat1, const CategoryType& cat2,
								  ValueType& value, ValueType default_value = 0) const;
	StringType DumpValue(CategoryType category, ValueType value, bool is_hex = false) const;

	ValueType
	ParseBitField(CategoryType category, const StringType& str,
				  ValueType default_value = 0) const;

protected:
	StringType _dumpBitField(CategoryType category, ValueType& value,
							 bool bNot = false) const;
};

#ifdef USE_GLOBALS
	extern ConstantsDB g_db;
#else
	inline ConstantsDB& DB_GetMaster(void)
	{
		static ConstantsDB s_db;
		return s_db;
	}
	#define g_db DB_GetMaster()
#endif
