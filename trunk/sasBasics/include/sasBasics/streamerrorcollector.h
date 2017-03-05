/*
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
    along with sasBasics.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASBASICS_STREAMERRORCOLLECTOR_H_
#define INCLUDE_SASBASICS_STREAMERRORCOLLECTOR_H_

#include "config.h"
#include <sasCore/errorcollector.h>
#include <sstream>

namespace SAS {

template <class Stream_T = std::ostream>
class StreamErrorCollector : public SAS::ErrorCollector
{
public:
	StreamErrorCollector(Stream_T & os) :_os(os)
	{ }

	virtual ~StreamErrorCollector() { }

	virtual std::string add(long errorCode, const std::string & errorText) override
	{
		std::stringstream ss;
		ss << "[" << errorCode << "]" << " " << errorText << std::endl;
		_os << ss.str();
		return ss.str();
	}

private:
	Stream_T & _os;
};

}

#endif /* INCLUDE_SASBASICS_STREAMERRORCOLLECTOR_H_ */
