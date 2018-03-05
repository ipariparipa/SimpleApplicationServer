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

#pragma warning( push )
#pragma warning( disable: 4461 )

#include "SASServer.h"

#include "macros.h"
#include "SASConfigReader.h"
#include "SASObjectRegistry.h"
#include "SASErrorCollector.h"

#include <msclr/gcroot.h>

using namespace msclr::interop;

namespace SAS {
	namespace Client {


		struct SASServer_um_priv
		{
			SASServer_um_priv(SASServer ^ mobj, array<System::String^> ^ args) : _args(args), obj(mobj, _args.argv.size(), (char**)_args.argv.data())
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

		ref struct SASServer_priv
		{
			SASServer_priv(SASServer ^ mobj, array<System::String^> ^ args) : 
				um(new SASServer_um_priv(mobj, args))
			{ }

			!SASServer_priv()
			{ delete um; }

			SASServer_um_priv * um;
		};

		SASServer::SASServer(array<System::String^> ^ args) : priv(gcnew SASServer_priv(this, args))
		{ }

		//property 
		SASObjectRegistry ^ SASServer::ObjectRegistry::get()
		{
			return gcnew SASObjectRegistry(priv->um->obj.objectRegistry());
		}

		//property 
		System::String ^ SASServer::Version::get()
		{
			return nullptr;
		}

		bool SASServer::Init(ISASErrorCollector ^ ec)
		{
			return priv->um->obj.init(WErrorCollector(ec));
		}

		void SASServer::Deinit()
		{
			priv->um->obj.deinit();
		}

		void SASServer::run()
		{
			priv->um->obj.run();
		}
	}
}

#pragma warning( pop )
