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

std::string to_env(const std::string & path)
{
	return str_join(str_split(path, '/'), "__");
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
	std::list<std::string> tmp_ret;
	str_split(tmp, ';', tmp_ret);
	ret.resize(tmp_ret.size());
	std::copy(tmp_ret.begin(), tmp_ret.end(), ret.begin());
	return true;
}

bool EnvConfigReader::getEntryAsStringList(const std::string & path, std::vector<std::string> & ret, const std::vector<std::string> & defaultValue, ErrorCollector & ec)
{
	SAS_LOG_NDC();
	std::string tmp;
	if (!getEntryAsString(path, tmp, str_join(defaultValue, ';'), ec))
		return false;
	std::list<std::string> tmp_ret;
	str_split(tmp, ';', tmp_ret);
	ret.resize(tmp_ret.size());
	std::copy(tmp_ret.begin(), tmp_ret.end(), ret.begin());
	return true;
}

}
