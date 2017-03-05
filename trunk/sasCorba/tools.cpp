/*
    This file is part of sasCorba.

    sasCorba is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCorba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCorba.  If not, see <http://www.gnu.org/licenses/>
 */

#include "tools.h"

namespace SAS { namespace CorbaTools {

std::vector<char> toByteArray(const CorbaSAS::SASModule::OctetSequence & data)
{
	std::vector<char> ret(data.length());
	for(size_t i(0), l(ret.size()); i < l; ++i)
		ret[i] = data[i];
	return ret;
}

extern void toOctetSequence(const std::vector<char> & data, CorbaSAS::SASModule::OctetSequence_out & ret)
{
	ret = new CorbaSAS::SASModule::OctetSequence();
	ret->length(data.size());
	for(size_t i(0), l(data.size()); i < l; ++i)
		ret[i] = data[i];
}

CorbaSAS::SASModule::OctetSequence_var toOctetSequence_var(const std::vector<char> & data)
{
	CorbaSAS::SASModule::OctetSequence_var ret = new CorbaSAS::SASModule::OctetSequence();
	ret->length(data.size());
	for(size_t i(0), l(data.size()); i < l; ++i)
		ret[i] = data[i];
	return ret;
}

}}
