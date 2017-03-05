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

#include "SASApplication.h"
#include "SASObjectRegistry.h"
#include "SASErrorCollector.h"
#include "SASConfigReader.h"

#include <sasCore/application.h>

#include <msclr/gcroot.h>

#include <memory>

using namespace msclr;

namespace SAS {
	namespace Client {


		struct SASApplication_priv
		{
			SASApplication_priv(SASApplication ^ mobj) : obj(mobj), objectRegistry(gcnew SASObjectRegistry(obj.objectRegistry()))
			{ }

			struct WApplication : public Application
			{
				WApplication(SASApplication ^ mobj_) : Application(), mobj(mobj_)
				{ }

				virtual std::string version() const final
				{ return std::string(); }

				virtual ConfigReader * configreader() final
				{
					if (!cr.get())
						cr.reset(new WConfigReader(mobj->ConfigReader));
					return cr.get();
				}

			private:
				gcroot<SASApplication^> mobj;
				std::unique_ptr<ConfigReader> cr;
			} obj;
			gcroot<SASObjectRegistry^> objectRegistry;
		};

		SASApplication::SASApplication() : priv(new SASApplication_priv(this))
		{ }

		SASApplication::!SASApplication()
		{
			delete priv;
		}

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
			return priv->obj.init(WErrorCollector(ec));
		}

		void SASApplication::deinit()
		{
			priv->obj.deinit();
		}
	}
}
