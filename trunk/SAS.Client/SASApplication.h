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

#include <sasCore/application.h>

namespace SAS {

	namespace Client {

		ref class SASObjectRegistry;
		interface class ISASErrorCollector;
		interface class ISASConfigReader;

		ref struct SASApplication_priv;

		public interface class ISASApplication
		{
		public:
			property SASObjectRegistry ^ ObjectRegistry { virtual SASObjectRegistry ^ get() abstract; }

			property System::String ^ Version { virtual System::String ^ get() abstract; }

			virtual bool Init(ISASErrorCollector ^ ec) abstract;

			virtual void Deinit() abstract;

			property ISASConfigReader ^ ConfigReader { virtual ISASConfigReader ^ get() abstract; }

		};

		public ref class SASApplication abstract : public ISASApplication
		{
		public:
			SASApplication(array<System::String^> ^ args);
			SASApplication();

			property SASObjectRegistry ^ ObjectRegistry { virtual SASObjectRegistry ^ get(); }

			property System::String ^ Version { virtual System::String ^ get(); }

			virtual bool Init(ISASErrorCollector ^ ec);

			virtual void Deinit();

			property ISASConfigReader ^ ConfigReader { virtual ISASConfigReader ^ get() abstract; }
		private:
			SASApplication_priv ^ priv;
		};

	}

	struct ArgsHelper
	{
		ArgsHelper();

		ArgsHelper(array<System::String^> ^ args);

		std::vector<const char *> argv;
	private:
		std::vector<std::string> _args;
	};

	struct WApplication_priv;
	class WApplication : public Application
	{
	public:
		WApplication(Client::ISASApplication ^ mobj, int argc, char ** argv);

		WApplication(Client::ISASApplication ^ mobj);

		~WApplication();

		virtual std::string version() const final;

		virtual ConfigReader * configReader() final;

	private:
		WApplication_priv * priv;
	};

}

#pragma warning( pop ) 
