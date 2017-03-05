/*
    This file is part of sasBasics.

    sasBasics is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasBasics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasBasics.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/sasBasics/envconfigreader.h"
#include <assert.h>
#include <sasCore/errorcollector.h>
#include <sstream>
#include <sasCore/logging.h>
#include <sasCore/tools.h>


namespace SAS {

void split(const std::string & s, char delim, std::vector<std::string> & elems)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
        elems.push_back(item);
}


std::vector<std::string> split(const std::string & s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

template<typename delim_T>
void join(const std::vector<std::string> & elems, const delim_T & delim, std::string & s)
{
	for(size_t i = 0, l = elems.size(); i < l; ++i)
	{
		s += elems[i];
		if(i + 1 < l)
			s += delim;
	}
}

template<typename delim_T>
std::string join(const std::vector<std::string> & elems, const delim_T & delim)
{
	std::string s;
	join<delim_T>(elems, delim, s);
	return s;
}

std::string to_env(const std::string & path)
{
	return join(split(path, '/'), "__");
}

EnvConfigReader::EnvConfigReader() : _logger(Logging::getLogger("SAS.EnvConfigurator"))
{ }

EnvConfigReader::~EnvConfigReader()
{ }

bool EnvConfigReader::getEntryAsString(const std::string & path, std::string & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	assert(path.length());
	std::string env_name = to_env(path);
	SAS_LOG_VAR(_logger, env_name);

#if SAS_OS == SAS_OS_LINUX
	char * tmp;
	if (!(tmp = getenv(env_name.c_str())))
	{
		SAS_LOG_TRACE(_logger, "environment variable is not set");
		return false;
	}
	ret = tmp;
#elif SAS_OS == SAS_OS_WINDOWS
	if (!win_getEnv(env_name, ret))
	{
		SAS_LOG_TRACE(_logger, "environment variable is not set");
		return false;
	}
#else
#  error to be implemented
#endif
	return true;
}

bool EnvConfigReader::getEntryAsString(const std::string & path, std::string & ret, const std::string & defaultValue, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	assert(path.length());
	std::string env_name = to_env(path);
	SAS_LOG_VAR(_logger, env_name);
#if SAS_OS == SAS_OS_LINUX
	char * tmp;
	ret = (tmp = getenv(env_name.c_str())) ? tmp : defaultValue;
#elif SAS_OS == SAS_OS_WINDOWS
	if (!win_getEnv(env_name, ret))
		ret = defaultValue;
#else
#  error to be implemented
#endif
	return true;
}

bool EnvConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::string tmp;
	if(!getEntryAsString(path, tmp, ec))
		return false;
	split(tmp, ';', ret);
	return true;
}

bool EnvConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::string tmp;
	if(!getEntryAsString(path, tmp, join(defaultValue, ';'), ec))
		return false;
	split(tmp, ';', ret);
	return true;
}

}
