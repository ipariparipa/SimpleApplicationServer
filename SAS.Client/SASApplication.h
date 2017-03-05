
#pragma once
#pragma warning( push )
#pragma warning( disable: 4461 )

namespace SAS {

	namespace Client {

		ref class SASObjectRegistry;
		interface class ISASErrorCollector;
		interface class ISASConfigReader;

		struct SASApplication_priv;

		public ref class SASApplication abstract
		{
		public:
			SASApplication();
			!SASApplication();

			property SASObjectRegistry ^ ObjectRegistry { SASObjectRegistry ^ get(); }

			property System::String ^ Version { virtual System::String ^ get(); }

			bool Init(ISASErrorCollector ^ ec);

			void deinit();

			property ISASConfigReader ^ ConfigReader { virtual ISASConfigReader ^ get() abstract; }
		private:
			SASApplication_priv * priv;

		};

	}
}

#pragma warning( pop ) 
