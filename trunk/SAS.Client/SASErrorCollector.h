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
#pragma warning( push )
#pragma warning( disable: 4461 )

#include <sasCore/errorcollector.h>

namespace SAS {
	namespace Client {

		public interface class ISASErrorCollector
		{
		public:
			virtual System::String ^ Add(long errorCode, System::String ^ errorText) abstract;
		};

		struct SASErrorCollectorObj_priv;

		ref class SASErrorCollectorObj : public ISASErrorCollector
		{
		internal:
			SASErrorCollectorObj(ErrorCollector & obj);
		public:
			!SASErrorCollectorObj();

			virtual System::String ^ Add(long errorCode, System::String ^ errorText);

		private:
			SASErrorCollectorObj_priv * priv;
		};

	}


	struct WErrorCollector_priv;
	class WErrorCollector :  public ErrorCollector
	{
	public:
		WErrorCollector(Client::ISASErrorCollector ^ mobj);
		virtual ~WErrorCollector();

		virtual std::string add(long errorCode, const std::string & errorText) final;

	private:
		WErrorCollector_priv * priv;
	};

}

#pragma warning( pop ) 
