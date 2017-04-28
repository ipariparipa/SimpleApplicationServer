/*
This file is part of sasTCL.

sasTCL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCL.  If not, see <http://www.gnu.org/licenses/>
*/

#include "include/sasTCL/tclerrorcollector.h"
#include <sstream>

namespace SAS {
	
	struct TCLErrorCollector_priv
	{
		TCLErrorCollector_priv(Tcl_Interp * interp_) : interp(interp_), lst(interp_)
		{ }

		Tcl_Interp * interp;
		TCLListHandler lst;
	};

	TCLErrorCollector::TCLErrorCollector(Tcl_Interp * interp) : ErrorCollector(), priv(new TCLErrorCollector_priv(interp))
	{ }

	TCLErrorCollector::~TCLErrorCollector()
	{
		delete priv;
	}

	void TCLErrorCollector::append(long errorCode, const std::string & errorText)
	{
		TCLListHandler err(priv->interp);
		err.append(std::to_string(errorCode));
		err.append(errorText);
		priv->lst.append(err);
	}

	TCLListHandler TCLErrorCollector::errors() const
	{
		return priv->lst;
	}

}
