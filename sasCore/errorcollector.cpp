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

#include "include/sasCore/errorcollector.h"
#include <sstream>

namespace SAS {

	std::string ErrorCollector::add(long errorCode, const std::string & errorText)
	{
		append(errorCode, errorText);
		return toString(errorCode, errorText);
	}


	//static 
	std::string ErrorCollector::toString(long errorCode, const std::string & errorText)
	{
		std::stringstream ss;
		ss << "[" << errorCode << "] " << errorText;
		return ss.str();
	}

	struct SimpleErrorCollector_priv
	{
		std::function<void(long errorCode, const std::string & errorText)> fnct;
	};

	SimpleErrorCollector::SimpleErrorCollector(std::function<void(long errorCode, const std::string & errorText)> fnct) : priv(new SimpleErrorCollector_priv)
	{
		priv->fnct = fnct;
	}

	SimpleErrorCollector::~SimpleErrorCollector()
	{
		delete priv;
	}

	void SimpleErrorCollector::append(long errorCode, const std::string & errorText)
	{
		priv->fnct(errorCode, errorText);
	}

}
