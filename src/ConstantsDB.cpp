// ConstantsDB.cpp --- Constants Database
//////////////////////////////////////////////////////////////////////////////
// RisohEditor --- Another free Win32 resource editor
// Copyright (C) 2017-2026 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>
// License: GPL-3 or later

#include "ConstantsDB.hpp"
#include <cctype>
#include <cstdio>

ConstantsDB::TableType ConstantsDB::GetTable(CategoryType category) const
{
	if (category.size())
		::CharUpperW(&category[0]);

	MapType::const_iterator it = m_map.find(category);
	if (it == m_map.end())
		return TableType();

	TableType table = it->second;
	return table;
}

ConstantsDB::TableType
ConstantsDB::GetTableByPrefix(CategoryType category, NameType prefix) const
{
	if (category.size())
		::CharUpperW(&category[0]);

	TableType table1;
	MapType::const_iterator found = m_map.find(category);
	if (found == m_map.end())
		return table1;

	const TableType& table2 = found->second;
	TableType::const_iterator it, end = table2.end();
	for (it = table2.begin(); it != end; ++it)
	{
		if (it->name.find(prefix) == 0)
			table1.push_back(*it);
	}
	return table1;
}

ConstantsDB::TableType ConstantsDB::GetConstantTable() const
{
	TableType table;
	for (auto& pair : m_map)
	{
		if (pair.first.find(L'.') != StringType::npos)
			continue;

		table.insert(table.end(), pair.second.begin(), pair.second.end());
	}
	return table;
}

BOOL ConstantsDB::GetValueOfName(const NameType& name, ValueType& value) const
{
	TableType table = GetConstantTable();
	for (const auto& table_entry : table)
	{
		if (table_entry.name == name)
		{
			value = table_entry.value;
			return TRUE;
		}
	}
	return FALSE;
}

ConstantsDB::ValueType ConstantsDB::GetResIDValue(const NameType& name) const
{
	ValueType value = GetValue(L"RESOURCE.ID", name);
	if (!value)
	{
		value = GetValue(L"CTRLID", name);
	}
	return value;
}

ConstantsDB::ValueType ConstantsDB::GetCtrlIDValue(const NameType& name) const
{
	if (name == L"IDC_STATIC")
		return -1;
	return GetValue(L"CTRLID", name);
}

bool ConstantsDB::HasCtrlID(const NameType& name) const
{
	if (name == L"IDC_STATIC")
		return true;

	TableType table = GetTable(L"CTRLID");
	for (auto& table_entry : table)
	{
		if (table_entry.name == name)
			return true;
	}
	return false;
}

bool ConstantsDB::HasResID(const NameType& name) const
{
	TableType table = GetTable(L"RESOURCE.ID");
	for (auto& table_entry : table)
	{
		if (table_entry.name == name)
			return true;
	}
	table = GetTable(L"CTRLID");
	for (auto& table_entry : table)
	{
		if (table_entry.name == name)
			return true;
	}
	return false;
}

ConstantsDB::StringType
ConstantsDB::GetNameOfResID(IDTYPE_ nIDTYPE_, ValueType value, bool unsign) const
{
	if (g_settings.bHideID)
	{
		switch (nIDTYPE_)
		{
		case IDTYPE_CONTROL:
		case IDTYPE_COMMAND:
		case IDTYPE_NEWCOMMAND:
			if (unsign)
				return mstr_dec_word((WORD)value);
			else
				return mstr_dec_short((SHORT)value);
		case IDTYPE_MESSAGE:
			return mstr_hex(value);
		default:
			return mstr_dec_word((WORD)value);
		}
	}

	if (nIDTYPE_ == IDTYPE_COMMAND || nIDTYPE_ == IDTYPE_NEWCOMMAND)
	{
		return GetCtrlOrCmdName(value, unsign);
	}

	if (nIDTYPE_ < 0 || nIDTYPE_ >= INT(g_idtype_strings.size()))
	{
		return mstr_dec_word((WORD)value);
	}

	auto prefix = MapIDTypeToPrefix(nIDTYPE_);
	if (prefix.empty())
	{
		if (nIDTYPE_ == IDTYPE_CONTROL)
			return mstr_dec_short((SHORT)value);
		else if (nIDTYPE_ == IDTYPE_MESSAGE)
			return mstr_hex(value);
		else
			return mstr_dec_word((WORD)value);
	}

	auto table = GetTableByPrefix(L"RESOURCE.ID", prefix);
	auto end = table.end();
	for (auto it = table.begin(); it != end; ++it)
	{
		if (it->value == value)
			return it->name;
	}

	if (nIDTYPE_ == IDTYPE_CONTROL)
	{
		if (value == (ValueType)-1 || value == 0xFFFF)
		{
			if (g_settings.bUseIDC_STATIC && !g_settings.bHideID)
				return L"IDC_STATIC";
			return L"-1";
		}

		return GetCtrlOrCmdName(value);
	}

	if (nIDTYPE_ != IDTYPE_RESOURCE && IsEntityIDType(nIDTYPE_))
	{
		return GetNameOfResID(IDTYPE_RESOURCE, value);
	}

	if (nIDTYPE_ == IDTYPE_HELP)
	{
		return mstr_dec_dword(value);
	}

	if (nIDTYPE_ == IDTYPE_MESSAGE)
	{
		return mstr_hex(value);
	}

	return mstr_dec_word(WORD(value));
}

ConstantsDB::StringType
ConstantsDB::GetNameOfIDTypeValue(IDTYPE_ nIDTYPE_, ValueType value) const
{
	auto prefix = MapIDTypeToPrefix(nIDTYPE_);
	auto table = GetTableByPrefix(L"RESOURCE.ID", prefix);
	{
		auto end = table.end();
		for (auto it = table.begin(); it != end; ++it)
		{
			if (it->value == value)
				return it->name;
		}
	}
	return L"";
}

ConstantsDB::StringType
ConstantsDB::GetCtrlOrCmdName(ValueType value, bool unsign) const
{
	if (value == 0xFFFF || value == (ValueType)-1)
	{
		if (g_settings.bUseIDC_STATIC && !g_settings.bHideID)
			return L"IDC_STATIC";
		else
			return L"-1";
	}
	if (g_settings.bHideID)
		return mstr_dec_dword(value);
	StringType str;
	str = GetNameOfIDTypeValue(IDTYPE_COMMAND, value);
	if (str.size())
		return str;
	str = GetNameOfIDTypeValue(IDTYPE_NEWCOMMAND, value);
	if (str.size())
		return str;
	str = GetNameOfIDTypeValue(IDTYPE_CONTROL, value);
	if (str.size())
		return str;
	str = DumpValue(L"CTRLID", value);
	if (str.empty() || str[0] == L'-' || mchr_is_digit(str[0]))
	{
		if (unsign)
			return mstr_dec_word((WORD)value);
		else
			return mstr_dec_short((SHORT)value);
	}
	return str;
}

ConstantsDB::StringType
ConstantsDB::GetNameOfResID(IDTYPE_ nIDTYPE_1, IDTYPE_ nIDTYPE_2, ValueType value,
	                        bool unsign) const
{
	StringType ret = GetNameOfResID(nIDTYPE_1, value, unsign);
	if (ret.size() && (mchr_is_digit(ret[0]) || ret[0] == L'-'))
		ret = GetNameOfResID(nIDTYPE_2, value, unsign);
	return ret;
}

ConstantsDB::NameType
ConstantsDB::GetName(const CategoryType& category, ValueType value) const
{
	const TableType& table = GetTable(category);
	TableType::const_iterator it, end = table.end();
	for (it = table.begin(); it != end; ++it)
	{
		if (it->value == value)
			return it->name;
	}
	return NameType();
}

ConstantsDB::NameType ConstantsDB::GetLangName(ValueType value) const
{
	const TableType& table = GetTable(L"Languages");
	TableType::const_iterator it, end = table.end();
	for (it = table.begin(); it != end; ++it)
	{
		if (it->name.size() != 5 || it->name[2] != L'_')
			continue;
		if (it->value == value)
			return it->name;
	}
	return mstr_dec_short((SHORT)value);
}

ConstantsDB::ValueType
ConstantsDB::GetValue(const CategoryType& category, const NameType& name) const
{
	const TableType& table = GetTable(category);
	TableType::const_iterator it, end = table.end();
	for (it = table.begin(); it != end; ++it)
	{
		if (it->name == name)
			return it->value;
	}
	return (ValueType)mstr_parse_int(name.c_str());
}

ConstantsDB::ValueType
ConstantsDB::GetValueI(const CategoryType& category, const NameType& name) const
{
	const TableType& table = GetTable(category);
	TableType::const_iterator it, end = table.end();
	for (it = table.begin(); it != end; ++it)
	{
		if (lstrcmpiW(it->name.c_str(), name.c_str()) == 0)
			return it->value;
	}
	return (ValueType)mstr_parse_int(name.c_str());
}

ConstantsDB::ValuesType
ConstantsDB::GetValues(const CategoryType& category, const NameType& name) const
{
	ValuesType ret;
	const TableType& table = GetTable(category);
	TableType::const_iterator it, end = table.end();
	for (it = table.begin(); it != end; ++it)
	{
		if (it->name == name)
		{
			ret.push_back(it->value);
		}
	}
	return ret;
}

bool ConstantsDB::LoadFromFile(LPCWSTR FileName)
{
	using namespace std;
	m_map.clear();

	FILE *fp = _wfopen(FileName, L"rb");
	if (fp == NULL)
		return false;

	CategoryType category;
	char buf[512];
	bool first = true;
	while (fgets(buf, _countof(buf), fp))
	{
		if (first)
		{
			if (std::memcmp(buf, "\xEF\xBB\xBF", 3) == 0) // UTF-8 BOM?
			{
				std::string str = &buf[3];
				StringCchCopyA(buf, _countof(buf), str.c_str());
			}
			first = false;
		}

		mstr_trim(buf);
		if (buf[0] == ';') // line comment
			continue;

		MStringW line;
		line = MAnsiToWide(CP_UTF8, buf);

		while (mstr_replace_all(line, L" |", L"|"))
			;
		while (mstr_replace_all(line, L"| ", L"|"))
			;

		mstr_trim(line);
		if (line.empty())
			continue;

		// "[category]"
		if (line[0] == L'[')
		{
			if (line[line.size() - 1] == L']')
			{
				category = line.substr(1, line.size() - 2);
				if (category.size())
					::CharUpperW(&category[0]);
				m_map[category];
			}
			continue;
		}

		// "[name], value, mask"
		static const wchar_t *s_delim = L" ,\r\n";
		WCHAR *pch0, *pch1, *pch2;
		pch0 = &line[0];
		if (*pch0 == L',')
		{
			pch0 = &line[0];
			*pch0 = 0;
			pch1 = wcstok(pch0 + 1, s_delim);
		}
		else
		{
			pch0 = wcstok(&line[0], s_delim);
			if (pch0 == NULL)
				continue;
			pch1 = wcstok(NULL, s_delim);
		}
		if (pch1 == NULL)
			continue;
		pch2 = wcstok(NULL, s_delim);

		NameType name = pch0;
		mstr_trim(name);

		StringType value_str = pch1;
		mstr_trim(value_str);
		if (value_str.empty())
			continue;

		StringType mask_str;
		if (pch2 == NULL)
		{
			mask_str = value_str;
		}
		else
		{
			mask_str = pch2;
		}
		mstr_trim(mask_str);

		ValueType value;
		if (value_str.size() && iswdigit(value_str[0]))
		{
			value = mstr_parse_int(value_str.c_str(), false);
		}
		else
		{
			value = ParseBitField(category, value_str);
		}

		ValueType mask;
		if (mask_str.size() && iswdigit(mask_str[0]))
		{
			mask = mstr_parse_int(mask_str.c_str(), false);
		}
		else
		{
			mask = ParseBitField(category, mask_str);
		}

		mstr_replace_all(name, L"|", L" | ");

		EntryType entry(name, value, mask);
		m_map[category].push_back(entry);
	}

	fclose(fp);
	return true;
}

ConstantsDB::StringType
ConstantsDB::DumpBitField(const CategoryType& cat1, ValueType& value,
	                      ValueType default_value) const
{
	StringType ret, str1, str3;

	ValueType def = default_value;
	default_value &= ~value;
	value &= ~def;

	str1 = _dumpBitField(cat1, value);

	if (!str1.empty())
	{
		ret = std::move(str1);
	}
	else
	{
		ret = L"0";
	}

	if (value)
	{
		if (!ret.empty() && ret != L"0")
			ret += L" | ";

		ret += mstr_hex(value);
	}

	if (default_value)
	{
		str3 = _dumpBitField(cat1, default_value, true);
		if (ret == L"0")
			ret.clear();
		else if (!ret.empty() && !str3.empty())
			ret += L" | ";
		ret += str3;
	}

	return ret;
}

ConstantsDB::StringType
ConstantsDB::DumpBitFieldOrZero(const CategoryType& cat1, ValueType& value,
	                            ValueType default_value) const
{
	StringType ret = DumpBitField(cat1, value, default_value);
	if (ret.empty())
		ret = L"0";
	return ret;
}

ConstantsDB::StringType
ConstantsDB::DumpBitField(const CategoryType& cat1, const CategoryType& cat2,
                          ValueType& value, ValueType default_value) const
{
	StringType ret, str1, str2, str3, str4;

	ValueType def = default_value;
	default_value &= ~value;
	value &= ~def;

	str1 = _dumpBitField(cat1, value);
	if (!cat2.empty())
		str2 = _dumpBitField(cat2, value);

	if (!str1.empty() && str1 != L"0")
	{
		if (!str2.empty() && str2 != L"0")
			ret = str1 + L" | " + str2;
		else
			ret = std::move(str1);
	}
	else
	{
		if (!str2.empty() && str2 != L"0")
			ret = std::move(str2);
		else
			ret = L"0";
	}

	if (value)
	{
		if (!ret.empty())
			ret += L" | ";

		ret += mstr_hex(value);
	}

	if (default_value)
	{
		str3 = _dumpBitField(cat1, default_value, true);
		if (!cat2.empty())
			str4 = _dumpBitField(cat2, default_value, true);
		if (ret == L"0")
			ret.clear();
		else if (!ret.empty() && !str3.empty())
			ret += L" | ";
		ret += str3;
		if (!ret.empty() && !str4.empty())
			ret += L" | ";
		ret += str4;
	}

	return ret;
}

ConstantsDB::StringType
ConstantsDB::DumpBitFieldOrZero(const CategoryType& cat1, const CategoryType& cat2,
	                            ValueType& value, ValueType default_value) const
{
	StringType ret = DumpBitField(cat1, cat2, value, default_value);
	if (ret.empty())
		ret = L"0";
	return ret;
}

ConstantsDB::StringType
ConstantsDB::DumpValue(CategoryType category, ValueType value, bool is_hex) const
{
	if (category.size())
		::CharUpperW(&category[0]);

	MapType::const_iterator found = m_map.find(category);
	if (found != m_map.end())
	{
		const TableType& table = found->second;
		TableType::const_iterator it, end = table.end();
		for (it = table.begin(); it != end; ++it)
		{
			if (value == it->value)
			{
				return it->name;
			}
		}
	}

	if (is_hex)
		return mstr_hex(value);

	return mstr_dec(value);
}

ConstantsDB::ValueType
ConstantsDB::ParseBitField(CategoryType category, const StringType& str,
	                       ValueType default_value) const
{
	if (category.size())
		::CharUpperW(&category[0]);

	std::vector<StringType> values;
	mstr_split(values, str, L" \t\r\n|+");

	ValueType value = default_value;
	auto end = values.end();
	for (auto it = values.begin(); it != end; ++it)
	{
		mstr_trim(*it);
		if ((*it).empty())
			continue;

		if (iswdigit((*it)[0]))
		{
			value |= mstr_parse_int(it->c_str(), false);
		}
		else
		{
			if ((*it).find(L"NOT ") != StringType::npos)
			{
				(*it) = (*it).substr(4);
				mstr_trim(*it);
				value &= ~GetValue(category, *it);
			}
			else
			{
				value |= GetValue(category, *it);
			}
		}
	}

	return value;
}

ConstantsDB::StringType
ConstantsDB::_dumpBitField(CategoryType category, ValueType& value, bool bNot) const
{
	StringType ret;

	if (category.size())
		::CharUpperW(&category[0]);

	MapType::const_iterator found = m_map.find(category);
	if (found == m_map.end())
		return ret;

	const TableType& table = found->second;
	for (;;)
	{
		ValueType max_value = 0;
		TableType::const_iterator max_it = table.end();
		TableType::const_iterator it, end = table.end();
		for (it = table.begin(); it != end; ++it)
		{
			if (it->value == 0)
				continue;

			if ((value & it->mask) == it->value)
			{
				if (it->value > max_value)
				{
					max_value = it->value;
					max_it = it;
				}
			}
		}

		if (max_it == end)
			break;  // not found

		if (!ret.empty())
			ret += L" | ";
		if (bNot)
		{
			ret += L"NOT ";
		}

		ret += max_it->name;
		value &= ~max_it->value;
	}

	return ret;
}
