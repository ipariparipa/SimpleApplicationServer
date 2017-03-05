
#pragma once
#pragma warning( push )
#pragma warning( disable: 4461 )

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

		struct SASObjectObj_priv;

		ref class SASObjectObj : public ISASObject
		{
		internal:
			SASObjectObj(SAS::Object * obj);
		public:
			!SASObjectObj();

			property System::String ^ Type { virtual System::String ^ get(); }
			property System::String ^ Name { virtual System::String ^ get(); }

		private:
			SASObjectObj_priv * priv;
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

#pragma warning( pop ) 
