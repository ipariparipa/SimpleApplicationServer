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

#include "SASServer.h"

#include "macros.h"
#include "SASConfigReader.h"
#include "SASObjectRegistry.h"
#include "SASErrorCollector.h"

namespace SAS {
	namespace Client {


		struct SASServer_priv
		{
			SASServer_priv(SASServer ^ mobj, array<System::String^> ^ args) : _args(args), obj(mobj, _args.argv.size(), (char**) _args.argv.data())
			{ }

			ArgsHelper _args;

			struct WServer : public Server
			{
				WServer(SASServer ^ mobj_, int argc, char ** argv) : Server(argc, argv), mobj(mobj_)
				{ }

				virtual std::string version() const final
				{
					return TO_STR(mobj->Version);
				}

				virtual ConfigReader * configReader() final
				{
					if (!cr.get())
						cr.reset(new WConfigReader(mobj->ConfigReader));
					return cr.get();
				}

			private:
				gcroot<SASServer ^> mobj;
				std::unique_ptr<ConfigReader> cr;
			} obj;

		};
	
		SASServer::SASServer(array<System::String^> ^ args) : priv(new SASServer_priv(this, args))
		{ }

		SASServer::!SASServer()
		{
			delete priv;
		}

		//property 
		SASObjectRegistry ^ SASServer::ObjectRegistry::get()
		{
			return gcnew SASObjectRegistry(priv->obj.objectRegistry());
		}

		//property 
		System::String ^ SASServer::Version::get()
		{
			return nullptr;
		}

		bool SASServer::Init(ISASErrorCollector ^ ec)
		{
			return priv->obj.init(WErrorCollector(ec));
		}

		void SASServer::Deinit()
		{
			priv->obj.deinit();
		}

		void SASServer::run()
		{
			priv->obj.run();
		}
	}
}
