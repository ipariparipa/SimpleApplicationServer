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

#ifndef sasTCL__tclobjectref_h
#define sasTCL__tclobjectref_h

#include "config.h"
#include <sasCore/defines.h>

struct Tcl_Obj;

namespace SAS {

	class SAS_TCL__CLASS TCLObjectRef
	{
		struct Priv;
		Priv * priv;

	public:
		TCLObjectRef();
		TCLObjectRef(Tcl_Obj * ob);
		TCLObjectRef(const TCLObjectRef & o);
		~TCLObjectRef();
		TCLObjectRef & operator = (const TCLObjectRef & o);
		TCLObjectRef & operator = (Tcl_Obj * o);

		bool isNull() const;

		operator Tcl_Obj * () const;
		Tcl_Obj * obj() const;
	};

}

#endif /* sasTCL__tclobjectref_h */
