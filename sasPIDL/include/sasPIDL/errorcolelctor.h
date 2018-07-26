/*
    This file is part of sasPIDL.

    sasPIDL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasPIDL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasPIDL.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASPIDL_ERRORCOLELCTOR_H_
#define INCLUDE_SASPIDL_ERRORCOLELCTOR_H_

#include "config.h"

#include <sasCore/defines.h>

#include <pidlCore/errorcollector.h>
#include <sasCore/errorcollector.h>

namespace SAS {

	class SAS_PIDL__CLASS PIDL_SASErrorCollector : public ErrorCollector
	{
		SAS_COPY_PROTECTOR(PIDL_SASErrorCollector)
		struct Priv;
		Priv * priv;
	public:
		PIDL_SASErrorCollector(PIDL::ErrorCollector & ec);
		virtual ~PIDL_SASErrorCollector();

	protected:
		virtual void append(long code, const std::string & msg) override;
	};

	class SAS_PIDL__CLASS SAS_PIDLErrorCollector : public PIDL::ErrorCollector
	{
		SAS_COPY_PROTECTOR(SAS_PIDLErrorCollector)
		struct Priv;
		Priv * priv;
	public:
		SAS_PIDLErrorCollector(SAS::ErrorCollector & ec);
		virtual ~SAS_PIDLErrorCollector();

	protected:
		virtual void append(long code, const std::string & msg) override;
	};

}

#endif /* INCLUDE_SASPIDL_ERRORCOLELCTOR_H_ */
