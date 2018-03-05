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

#ifndef sasTCL__tcllisthandler_h
#define sasTCL__tcllisthandler_h

#include "config.h"
#include <sasCore/defines.h>

#include <string>
#include SAS_TCL__TCL_H

namespace SAS {

	class SAS_TCL__CLASS TCLListHandler
	{
		struct Priv;
		Priv * priv;

	public:
		TCLListHandler();
		TCLListHandler(Tcl_Interp * interp);
		TCLListHandler(Tcl_Interp * interp, Tcl_Obj * ob);
		TCLListHandler(const TCLListHandler & o);
		~TCLListHandler();
		TCLListHandler & operator = (const TCLListHandler & o);
		bool isNull() const;

		bool append(const std::string & str);
		bool append(const TCLListHandler & lst);
		bool append(Tcl_Obj * obj);
		int length() const;

		bool fromString(const std::string & str);

		std::string operator [] (int idx) const;
		std::string getString(int idx) const;
		TCLListHandler getList(int idx) const;

		std::string toString() const;

		Tcl_Obj * obj();
	};

}

#endif // sasTCL__tcllisthandler_h
