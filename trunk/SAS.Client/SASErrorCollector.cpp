/*
    This file is part of SAS.Client.

    SAS.Client is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SAS.Client is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SAS.Client.  If not, see <http://www.gnu.org/licenses/>
 */

#include "SASErrorCollector.h"

#include "macros.h"

#include <msclr/gcroot.h>

using namespace msclr;

namespace SAS {
	namespace Client {

		struct SASErrorCollectorObj_priv
		{
			SASErrorCollectorObj_priv(ErrorCollector & obj_) : obj(obj_)
			{ }

			ErrorCollector & obj;
		};

		SASErrorCollectorObj::SASErrorCollectorObj(ErrorCollector & obj) : priv(new SASErrorCollectorObj_priv(obj))
		{ }

		SASErrorCollectorObj::!SASErrorCollectorObj()
		{
			delete priv;
		}

		void SASErrorCollectorObj::Add(long errorCode, System::String ^ errorText)
		{
			priv->obj.add(errorCode, TO_STR(errorText));
		}

	}

	struct WErrorCollector_priv
	{
		WErrorCollector_priv(Client::ISASErrorCollector ^ mobj_) : mobj(mobj_)
		{ }

		gcroot<Client::ISASErrorCollector^> mobj;
	};

	WErrorCollector::WErrorCollector(Client::ISASErrorCollector ^ mobj) : ErrorCollector(), priv(new WErrorCollector_priv(mobj))
	{ }

	WErrorCollector::~WErrorCollector() 
	{
		delete priv;
	}

	void WErrorCollector::append(long errorCode, const std::string & errorText)
	{
		priv->mobj->Add(errorCode, TO_MSTR(errorText));
	}
}
