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

#ifndef sasTCL__tcllist_h
#define sasTCL__tcllist_h

#include "config.h"
#include <sasCore/defines.h>

#include <string>
#include SAS_TCL__TCL_H

namespace SAS {

	struct TCLList_priv;
	class SAS_TCL__CLASS TCLList
	{
	public:
		TCLList();
		TCLList(Tcl_Interp * interp);
		TCLList(Tcl_Interp * interp, Tcl_Obj * ob);
		TCLList(const TCLList & o);
		~TCLList();
		TCLList & operator = (const TCLList & o);
		bool isNull() const;

		bool append(const std::string & str);
		bool append(const TCLList & lst);
		bool append(Tcl_Obj * obj);
		int length() const;

		bool fromString(const std::string & str);

		std::string operator [] (int idx) const;
		std::string getString(int idx) const;
		TCLList getList(int idx) const;

		std::string toString() const;

		Tcl_Obj * obj() const;

	private:
		TCLList_priv * priv;
	};

}

#endif // sasTCL__tcllist_h
