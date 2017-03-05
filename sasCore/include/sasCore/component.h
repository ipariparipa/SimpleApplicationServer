/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASCORE_COMPONENT_H_
#define INCLUDE_SASCORE_COMPONENT_H_

#include "defines.h"

#include <string>

namespace SAS
{
	class Application;
	class ErrorCollector;

	class SAS_CORE__CLASS Component
	{
		SAS_COPY_PROTECTOR(Component)
	public:
		Component();
		virtual ~Component();

		virtual inline std::string name() const { return std::string(); }
		virtual inline std::string description() const { return std::string(); }
		virtual inline std::string version() const { return std::string(); }

		virtual bool init(Application * app, ErrorCollector & ec) = 0;

	};
}

/*
extern "C" SAS_XXX__FUNCTION SAS::Component * __sas_attach_component();
extern "C" SAS_XXX__FUNCTION void __sas_detach_component(SAS::Component * );
*/

#endif /* INCLUDE_SASCORE_COMPONENT_H_ */
