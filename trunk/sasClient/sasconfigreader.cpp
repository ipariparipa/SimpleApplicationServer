
#include "include/sasClient/sasconfigreader.h"

#include <assert.h>

class WConfigReader : public SAS::ConfigReader
{
public:
	WConfigReader(const sas_ConfigReader_Functions & functions) : _functions(functions), SAS::ConfigReader()
	{ }

	virtual bool getEntryAsString(const std::string & path, std::string & ret, SAS::ErrorCollector & ec) final
	{
		if (_functions.getEntryAsString_1)
		{
			sas_String tmp = { 0, NULL, FALSE };
			if (!_functions.getEntryAsString_1(path.c_str(), &tmp, &ec))
				return false;
			ret.append(tmp.data, tmp.size);
			return true;
		}
		return SAS::ConfigReader::getEntryAsString(path, ret, ec);
	}

	virtual bool getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, SAS::ErrorCollector & ec)  final
	{
		if (_functions.getEntryAsString_2)
		{
			sas_String tmp = { 0, NULL, FALSE };
			if (!_functions.getEntryAsString_2(path.c_str(), &tmp, defaultValue.c_str(), &ec))
				return false;
			ret.append(tmp.data, tmp.size);
			return true;
		}
		return SAS::ConfigReader::getEntryAsString(path, ret, ec);
	}

	virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, SAS::ErrorCollector & ec) final
	{
		assert(_functions.getEntryAsStringList_1);
		sas_StringArray tmp = { 0, NULL };
		if (!_functions.getEntryAsStringList_1(path.c_str(), &tmp, &ec))
			return false;
		ret.resize(tmp.size);
		for (size_t i(0); i < tmp.size; ++i)
			ret[i].append(tmp.data[i].data, tmp.data[i].size);
		return true;
	}

	virtual bool getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, SAS::ErrorCollector & ec) final
	{
		assert(_functions.getEntryAsStringList_2);
		sas_StringArray tmp = {0, NULL};
		sas_StringArray def_tmp = sas_StringArray_init(defaultValue.size());
		for (size_t i(0); i < def_tmp.size; ++i)
			def_tmp.data[i] = SAS::Client::string_set_ref(defaultValue[i]);
		if (!_functions.getEntryAsStringList_2(path.c_str(), &tmp, &def_tmp, &ec))
			return false;
		ret.resize(tmp.size);
		for (size_t i(0); i < tmp.size; ++i)
			ret[i].append(tmp.data[i].data, tmp.data[i].size);
		return true;
	}

	virtual bool getStringEntry(const std::string & path, std::string & ret, SAS::ErrorCollector & ec) final
	{
		if (_functions.getStringEntry_1)
		{
			sas_String tmp = { 0, NULL, FALSE };
			if (!_functions.getStringEntry_1(path.c_str(), &tmp, &ec))
				return false;
			ret.append(tmp.data, tmp.size);
			return true;
		}
		return SAS::ConfigReader::getStringEntry(path, ret, ec);
	}

	virtual bool getStringEntry(const std::string & path, std::string & ret, const std::string & defaultValue, SAS::ErrorCollector & ec)  final
	{
		if (_functions.getStringEntry_2)
		{
			sas_String tmp = { 0, NULL, FALSE };
			if (!_functions.getStringEntry_2(path.c_str(), &tmp, defaultValue.c_str(), &ec))
				return false;
			ret.append(tmp.data, tmp.size);
			return true;
		}
		return SAS::ConfigReader::getStringEntry(path, ret, ec);
	}

	virtual bool getStringListEntry(const std::string & path, std::vector<std::string> & ret, SAS::ErrorCollector & ec) final
	{
		if (_functions.getEntryAsStringList_1)
		{
			sas_StringArray tmp = { 0, NULL };
			if (!_functions.getEntryAsStringList_1(path.c_str(), &tmp, &ec))
				return false;
			ret.resize(tmp.size);
			for (size_t i(0); i < tmp.size; ++i)
				ret[i].append(tmp.data[i].data, tmp.data[i].size);
			return true;
		}
		return SAS::ConfigReader::getStringListEntry(path, ret, ec);
	}

	virtual bool getStringListEntry(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, SAS::ErrorCollector & ec) final
	{
		if (_functions.getEntryAsStringList_2)
		{
			sas_StringArray tmp = { 0, NULL };
			sas_StringArray def_tmp = sas_StringArray_init(defaultValue.size());
			for (size_t i(0); i < def_tmp.size; ++i)
				def_tmp.data[i] = SAS::Client::string_set_ref(defaultValue[i]);
			if (_functions.getEntryAsStringList_2(path.c_str(), &tmp, &def_tmp, &ec) == FALSE)
				return false;
			ret.resize(tmp.size);
			for (size_t i(0); i < tmp.size; ++i)
				ret[i].append(tmp.data[i].data, tmp.data[i].size);
			return true;
		}
		return SAS::ConfigReader::getStringListEntry(path, ret, defaultValue, ec);
	}

	virtual bool getNumberEntry(const std::string & path, long long & ret, SAS::ErrorCollector & ec) final
	{
		if (_functions.getNumberEntry_1)
			return _functions.getNumberEntry_1(path.c_str(), &ret, &ec) != FALSE;
		return SAS::ConfigReader::getNumberEntry(path, ret, ec);
	}

	virtual bool getNumberEntry(const std::string & path, long long & ret, long long defaultvalue, SAS::ErrorCollector & ec) final
	{
		if (_functions.getNumberEntry_2)
			return _functions.getNumberEntry_2(path.c_str(), &ret, defaultvalue, &ec) != FALSE;
		return SAS::ConfigReader::getNumberEntry(path, ret, defaultvalue, ec);
	}

	virtual bool getBoolEntry(const std::string & path, bool & ret, SAS::ErrorCollector & ec)
	{
		if (_functions.getBoolEntry_1)
		{
			sas_Bool tmp;
			if (!_functions.getBoolEntry_1(path.c_str(), &tmp, &ec))
				return false;
			ret = tmp != FALSE;
			return true;
		}
		return SAS::ConfigReader::getBoolEntry(path, ret, ec);
	}

	virtual bool getBoolEntry(const std::string & path, bool & ret, bool defaultvalue, SAS::ErrorCollector & ec)
	{
		if (_functions.getBoolEntry_2)
		{
			sas_Bool tmp;
			if (!_functions.getBoolEntry_2(path.c_str(), &tmp, (sas_Bool)defaultvalue, &ec))
				return false;
			ret = tmp != FALSE;
			return true;
		}
		return SAS::ConfigReader::getBoolEntry(path, ret, defaultvalue, ec);
	}

private:
	sas_ConfigReader_Functions _functions;
};

extern "C" SAS_CLIENT__FUNCTION sas_ConfigReader_T SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_init(const sas_ConfigReader_Functions * functions)
{
	assert(functions->getEntryAsStringList_1);
	assert(functions->getEntryAsStringList_2);
	return new WConfigReader(*functions);
}

extern "C" SAS_CLIENT__FUNCTION void SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_deinit(sas_ConfigReader_T obj)
{
	delete obj;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsString_1(sas_ConfigReader_T obj, const char * path, sas_String * ret, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	std::string tmp;
	if (!obj->getEntryAsString(path, tmp, *ec))
		return FALSE;
	SAS::Client::string_copy(*ret, tmp);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsString_2(sas_ConfigReader_T obj, const char * path, sas_String * ret, const char * defaultValue, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(defaultValue);
	assert(ec);
	std::string tmp;
	if (!obj->getEntryAsString(path, tmp, defaultValue, *ec))
		return FALSE;
	SAS::Client::string_copy(*ret, tmp);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsStringList_1(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	std::vector<std::string> tmp;
	if (!obj->getEntryAsStringList(path, tmp, *ec))
		return FALSE;
	sas_StringArray_reset(ret, tmp.size());
	for (size_t i(0); i < ret->size; ++i)
		SAS::Client::string_copy(ret->data[i], tmp[i]);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getEntryAsStringList_2(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, const sas_StringArray * defaultValue, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(defaultValue);
	assert(ec);
	std::vector<std::string> tmp;
	std::vector<std::string> def_tmp(defaultValue->size);
	for (size_t i(0); i < defaultValue->size; ++i)
		def_tmp[i].append(defaultValue->data[i].data, defaultValue->data[i].size);
	if (!obj->getEntryAsStringList(path, tmp, def_tmp, *ec))
		return FALSE;
	sas_StringArray_reset(ret, tmp.size());
	for (size_t i(0); i < ret->size; ++i)
		SAS::Client::string_copy(ret->data[i], tmp[i]);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringEntry_1(sas_ConfigReader_T obj, const char * path, sas_String * ret, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	std::string tmp;
	if (!obj->getEntryAsString(path, tmp, *ec))
		return FALSE;
	SAS::Client::string_copy(*ret, tmp);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringEntry_2(sas_ConfigReader_T obj, const char * path, sas_String * ret, const char * defaultValue, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(defaultValue);
	assert(ec);
	std::string tmp;
	if (!obj->getEntryAsString(path, tmp, defaultValue, *ec))
		return FALSE;
	SAS::Client::string_copy(*ret, tmp);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringListEntry_1(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	std::vector<std::string> tmp;
	if (!obj->getStringListEntry(path, tmp, *ec))
		return FALSE;
	sas_StringArray_reset(ret, tmp.size());
	for (size_t i(0); i < ret->size; ++i)
		SAS::Client::string_copy(ret->data[i], tmp[i]);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getStringListEntry_2(sas_ConfigReader_T obj, const char * path, sas_StringArray * ret, const sas_StringArray * defaultValue, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(defaultValue);
	assert(ec);
	std::vector<std::string> tmp;
	std::vector<std::string> def_tmp(defaultValue->size);
	for (size_t i(0); i < defaultValue->size; ++i)
		def_tmp[i].append(defaultValue->data[i].data, defaultValue->data[i].size);
	if (!obj->getStringListEntry(path, tmp, def_tmp, *ec))
		return FALSE;
	sas_StringArray_reset(ret, tmp.size());
	for (size_t i(0); i < ret->size; ++i)
		SAS::Client::string_copy(ret->data[i], tmp[i]);
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getNumberEntry_1(sas_ConfigReader_T obj, const char * path, long long * ret, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	return (sas_Bool)obj->getNumberEntry(path, *ret, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getNumberEntry_2(sas_ConfigReader_T obj, const char * path, long long * ret, long long defaultValue, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	return (sas_Bool)obj->getNumberEntry(path, *ret, defaultValue, *ec);
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getBoolEntry_1(sas_ConfigReader_T obj, const char * path, sas_Bool * ret, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	bool tmp;
	if (!obj->getBoolEntry(path, tmp, *ec))
		return FALSE;
	*ret = (sas_Bool)tmp;
	return TRUE;
}

extern "C" SAS_CLIENT__FUNCTION sas_Bool SAS_CLIENT__CALL_CONVENTION sas_ConfigReader_getBoolEntry_2(sas_ConfigReader_T obj, const char * path, sas_Bool * ret, sas_Bool defaultValue, sas_ErrorCollector_T ec)
{
	assert(obj);
	assert(path);
	assert(ret);
	assert(ec);
	bool tmp;
	if (!obj->getBoolEntry(path, tmp, defaultValue != FALSE, *ec))
		return FALSE;
	*ret = (sas_Bool)tmp;
	return TRUE;
}

