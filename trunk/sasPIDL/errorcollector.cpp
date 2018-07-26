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

#include "include/sasPIDL/errorcollector.h"

namespace SAS {

struct PIDL_SASErrorCollector::Priv
{
	Priv(PIDL::ErrorCollector & ec_) : ec(ec_)
	{ }

	PIDL::ErrorCollector & ec;
};

PIDL_SASErrorCollector::PIDL_SASErrorCollector(PIDL::ErrorCollector & ec) :
		ErrorCollector(),
		priv(new Priv(ec))
{ }

PIDL_SASErrorCollector::~PIDL_SASErrorCollector()
{
	delete priv;
}

void PIDL_SASErrorCollector::append(long code, const std::string & msg)
{
	priv->ec.add(code, msg);
}



struct SAS_PIDLErrorCollector::Priv
{
	Priv(SAS::ErrorCollector & ec_) : ec(ec_)
	{ }

	SAS::ErrorCollector & ec;
};

SAS_PIDLErrorCollector::SAS_PIDLErrorCollector(SAS::ErrorCollector & ec) :
		PIDL::ErrorCollector(),
		priv(new Priv(ec))
{ }

SAS_PIDLErrorCollector::~SAS_PIDLErrorCollector()
{
	delete priv;
}

void SAS_PIDLErrorCollector::append(long code, const std::string & msg)
{
	priv->ec.add(code, msg);
}

}


