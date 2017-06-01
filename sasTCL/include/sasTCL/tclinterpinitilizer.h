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

#ifndef sasTCL__tclinterpinitializer_h
#define sasTCL__tclinterpinitializer_h

#include "config.h"
#include SAS_TCL__TCL_H

namespace SAS {

	class SAS_TCL__CLASS TCLInterpInitializer
	{
	public:
		virtual void init(Tcl_Interp * interp) = 0;
	};

}


#endif // sasTCL__tclinterpinitializer_h
