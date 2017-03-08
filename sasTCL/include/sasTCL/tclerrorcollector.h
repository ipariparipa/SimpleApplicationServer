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

#ifndef sasTCL__tclerrorcollector_h
#define sasTCL__tclerrorcollector_h

#include "config.h"
#include <sasCore/defines.h>
#include <sasCore/errorcollector.h>

#include "tcllist.h"

#include <tcl.h>

namespace SAS {

	struct TCLErrorCollector_priv;
	class SAS_TCL__CLASS TCLErrorCollector : public ErrorCollector
	{
		SAS_COPY_PROTECTOR(TCLErrorCollector)
	public:
		TCLErrorCollector(Tcl_Interp * interp);
		virtual ~TCLErrorCollector();

		TCLList errors() const;

	protected:
		void append(long errorCode, const std::string & errorText);

	private:
		TCLErrorCollector_priv * priv;
	};

}

#endif // sasTCL__tclerrorcollector_h
