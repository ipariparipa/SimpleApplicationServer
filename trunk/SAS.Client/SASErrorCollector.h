
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
