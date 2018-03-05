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

#pragma once

#include <sasCore/object.h>

namespace SAS {

	class Object;

	namespace Client {


		public interface class ISASObject
		{
		public:
			property System::String ^ Type { virtual System::String ^ get() abstract; }
			property System::String ^ Name { virtual System::String ^ get() abstract; }
		};

		ref struct SASObjectObj_priv;

		ref class SASObjectObj : public ISASObject
		{
		internal:
			SASObjectObj(SAS::Object * obj);
		public:
			property System::String ^ Type { virtual System::String ^ get(); }
			property System::String ^ Name { virtual System::String ^ get(); }

		private:
			SASObjectObj_priv ^ priv;
		};
	}

	struct WObject_priv;
	class WObject : public Object
	{
	public:
		WObject(Client::ISASObject ^ mobj);
		virtual ~WObject();

		virtual std::string type() const final;
		virtual std::string name() const final;
	private:
		WObject_priv * priv;
	};
}
