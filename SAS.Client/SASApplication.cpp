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

#include "SASApplication.h"
#include "SASObjectRegistry.h"
#include "SASErrorCollector.h"
#include "SASConfigReader.h"
#include "macros.h"

#include <sasCore/application.h>

#include <msclr/gcroot.h>

#include <string>
#include <memory>

using namespace msclr;

namespace SAS {
	namespace Client {

		struct SASApplication_um_priv
		{
			SASApplication_um_priv(SASApplication ^ mobj, array<System::String^> ^ args) :
				_args(args), 
				obj(mobj, _args.argv.size(), 
				(char**)_args.argv.data())
			{ }

			SASApplication_um_priv(SASApplication ^ mobj) :
				obj(mobj)
			{ }

			ArgsHelper _args;

			WApplication obj;
		};

		ref struct SASApplication_priv
		{
			SASApplication_priv(SASApplication ^ mobj, array<System::String^> ^ args) :
				um(new SASApplication_um_priv(mobj, args)),
				objectRegistry(gcnew SASObjectRegistry(um->obj.objectRegistry()))
			{ }

			SASApplication_priv(SASApplication ^ mobj) :
				um(new SASApplication_um_priv(mobj)),
				objectRegistry(gcnew SASObjectRegistry(um->obj.objectRegistry()))
			{ }

			!SASApplication_priv()
			{ delete um; }

			SASApplication_um_priv * um;

			SASObjectRegistry ^  objectRegistry;
		};

		SASApplication::SASApplication(array<System::String^> ^ args) : 
			priv(gcnew SASApplication_priv(this, args))
		{ }

		SASApplication::SASApplication() : 
			priv(gcnew SASApplication_priv(this))
		{ }

		//property 
		SASObjectRegistry ^ SASApplication::ObjectRegistry::get()
		{
			return priv->objectRegistry;
		}

		//virtual property 
		System::String ^ SASApplication::Version::get()
		{
			return nullptr;
		}

		bool SASApplication::Init(ISASErrorCollector ^ ec)
		{
			return priv->um->obj.init(WErrorCollector(ec));
		}

		void SASApplication::Deinit()
		{
			priv->um->obj.deinit();
		}
	}

	
	ArgsHelper::ArgsHelper()
	{ }

	ArgsHelper::ArgsHelper(array<System::String^> ^ args) : _args(args->Length), argv(args->Length)
	{
		for (int i(0), l(args->Length); i < l; ++i)
		{
			System::String ^ s = args[i];
			argv[i] = (_args[i] = TO_STR(s)).c_str();
		}
	}


	struct WApplication_priv
	{
		WApplication_priv(Client::ISASApplication ^ mobj_) : mobj(mobj_)
		{ }

		gcroot<Client::ISASApplication^> mobj;
		std::unique_ptr<ConfigReader> cr;
	};

	WApplication::WApplication(Client::ISASApplication ^ mobj, int argc, char ** argv) : Application(argc, argv), priv(new WApplication_priv(mobj))
	{ }

	WApplication::WApplication(Client::ISASApplication ^ mobj) : Application(), priv(new WApplication_priv(mobj))
	{ }

	WApplication::~WApplication()
	{
		delete priv;
	}

	std::string WApplication::version() const
	{
		return TO_STR(priv->mobj->Version);
	}

	ConfigReader * WApplication::configReader()
	{
		if (!priv->cr.get())
			priv->cr.reset(new WConfigReader(priv->mobj->ConfigReader));
		return priv->cr.get();
	}

}

#pragma warning( pop )
