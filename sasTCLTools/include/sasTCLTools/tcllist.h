/*
This file is part of sasTCLTools.

sasTCLTools is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCLTools is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCLTools.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasTCLTools__tcllist_h
#define sasTCLTools__tcllist_h

#include "config.h"

#include <string>

namespace SAS {

	struct TCLList_priv;

	class SAS_TCL_TOOLS__CLASS TCLList
	{
		TCLList(const std::string & str);
	public:
		TCLList();
		TCLList(const TCLList & o);
		~TCLList();
		TCLList & operator = (const TCLList & o);

		bool isNull() const;

		bool fromString(const std::string & str);

		size_t length() const;

		TCLList & operator << (const std::string & str);
		TCLList & operator << (const TCLList & o);

		void append(const TCLList & o);
		void append(const std::string & str);

		const std::string & operator [] (size_t idx) const;
		TCLList getList(size_t idx) const;
		const std::string & at(size_t idx) const;

		std::string toString() const;
	private:
		TCLList_priv * priv;
	};

}

#endif // sasTCLTools__tcllist_h
