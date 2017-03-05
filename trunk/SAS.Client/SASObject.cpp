
#include "SASObject.h"

#include "macros.h"

#include <msclr/gcroot.h>

using namespace msclr;

namespace SAS {
	namespace Client {

		struct SASObjectObj_priv
		{
			SASObjectObj_priv(SAS::Object * obj_) : obj(obj_)
			{ }

			SAS::Object * obj;
		};

		SASObjectObj::SASObjectObj(SAS::Object * obj) : priv(new SASObjectObj_priv(obj))
		{ }

		SASObjectObj::!SASObjectObj()
		{ 
			delete priv;
		}

		//property 
		System::String ^ SASObjectObj::Type::get()
		{
			return TO_MSTR(priv->obj->name());
		}

		//property 
		System::String ^ SASObjectObj::Name::get()
		{
			return TO_MSTR(priv->obj->type());
		}


	}


	struct WObject_priv
	{
		WObject_priv(Client::ISASObject ^ mobj_) : mobj(mobj_)
		{ }

		gcroot<Client::ISASObject ^> mobj;
	};

	WObject::WObject(Client::ISASObject ^ mobj) : Object(), priv(new WObject_priv(mobj))
	{ }

	WObject::~WObject()
	{
		delete priv;
	}

	std::string WObject::type() const
	{
		return TO_STR(priv->mobj->Type);
	}

	std::string WObject::name() const
	{
		return TO_STR(priv->mobj->Name);
	}
}
