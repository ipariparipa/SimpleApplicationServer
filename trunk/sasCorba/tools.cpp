/*
    This file is part of sasCorba.

    sasCorba is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCorba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCorba.  If not, see <http://www.gnu.org/licenses/>
 */

#include "tools.h"
#include <sasCore/errorcollector.h>

namespace SAS { namespace CorbaTools {

	std::vector<char> toByteArray(const CorbaSAS::SASModule::OctetSequence & data)
	{
		std::vector<char> ret(data.length());
		for(size_t i(0), l(ret.size()); i < l; ++i)
			ret[i] = data[i];
		return ret;
	}

	extern void toOctetSequence(const std::vector<char> & data, CorbaSAS::SASModule::OctetSequence_out & ret)
	{
		ret = new CorbaSAS::SASModule::OctetSequence();
		ret->length(data.size());
		for(size_t i(0), l(data.size()); i < l; ++i)
			ret[i] = data[i];
	}

	extern CorbaSAS::SASModule::OctetSequence_var toOctetSequence_var(const std::vector<char> & data)
	{
		CorbaSAS::SASModule::OctetSequence_var ret = new CorbaSAS::SASModule::OctetSequence();
		ret->length(data.size());
		for(size_t i(0), l(data.size()); i < l; ++i)
			ret[i] = data[i];
		return ret;
	}

	extern void logException(Logging::LoggerPtr logger, CORBA::COMM_FAILURE & ex)
	{
		SAS_LOG_VAR(logger, ex._rep_id());
		SAS_LOG_VAR(logger, ex._name());
		SAS_LOG_VAR(logger, ex.minor());
		SAS_LOG_VAR(logger, ex.completed());
	}

	extern void logException(Logging::LoggerPtr logger, CORBA::Exception & ex)
	{
		SAS_LOG_VAR(logger, ex._rep_id());
		SAS_LOG_VAR(logger, ex._name());
	}

	extern void logException(Logging::LoggerPtr logger, CORBA::SystemException & ex)
	{
		SAS_LOG_VAR(logger, ex._rep_id());
		SAS_LOG_VAR(logger, ex._name());
		SAS_LOG_VAR(logger, ex.minor());
		SAS_LOG_VAR(logger, ex.completed());
	}

	extern void logException(Logging::LoggerPtr logger, omniORB::fatalException & ex)
	{
		SAS_LOG_VAR(logger, ex.file());
		SAS_LOG_VAR(logger, ex.line());
		SAS_LOG_VAR(logger, ex.errmsg());
	}

	extern void logException(Logging::LoggerPtr logger, CorbaSAS::ErrorHandling::ErrorException & ex, ErrorCollector &ec)
	{
		SAS_LOG_VAR(logger, ex.invoker);
		SAS_LOG_VAR(logger, ex.sas_module);
		for (size_t i(0), l(ex.err.length()); i < l; ++i)
		{
			auto err = ec.add(ex.err[i].error_code, ex.err[i].error_text._ptr);
			SAS_LOG_ERROR(logger, err);
		}
	}

	extern void logException(Logging::LoggerPtr logger, CorbaSAS::ErrorHandling::FatalErrorException & ex, ErrorCollector &ec)
	{
		SAS_LOG_VAR(logger, ex.invoker);
		SAS_LOG_VAR(logger, ex.sas_module);
		for (size_t i(0), l(ex.err.length()); i < l; ++i)
		{
			auto err = ec.add(ex.err[i].error_code, ex.err[i].error_text._ptr);
			SAS_LOG_ERROR(logger, err);
		}
	}

	extern void logException(Logging::LoggerPtr logger, CorbaSAS::ErrorHandling::NotImplementedException & ex, ErrorCollector &ec)
	{
		SAS_LOG_VAR(logger, ex.invoker);
		SAS_LOG_VAR(logger, ex.sas_module);
		for (size_t i(0), l(ex.err.length()); i < l; ++i)
		{
			auto err = ec.add(ex.err[i].error_code, ex.err[i].error_text._ptr);
			SAS_LOG_ERROR(logger, err);
		}
	}

}}
