
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
