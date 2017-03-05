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

#ifndef TOOLS_H_
#define TOOLS_H_

#include "corbasas.hh"
#include <vector>

namespace SAS { namespace CorbaTools {

extern std::vector<char> toByteArray(const CorbaSAS::SASModule::OctetSequence & data);

extern void toOctetSequence(const std::vector<char> & data, CorbaSAS::SASModule::OctetSequence_out & ret);
extern CorbaSAS::SASModule::OctetSequence_var toOctetSequence_var(const std::vector<char> & data);

}}

#endif /* TOOLS_H_ */
