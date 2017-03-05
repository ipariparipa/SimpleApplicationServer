/*
    This file is part of sasBasics.

    sasBasics is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasBasics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with ${project_name}.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <sasCore/logging.h>

namespace SAS {

class ErrorCollector;

namespace Logging {

	bool init(int argc, char *argv[], ErrorCollector & ec);

	void writeUsage(std::ostream & os);

}}

#endif /* LOGGING_H_ */
