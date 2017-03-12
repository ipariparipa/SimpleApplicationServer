/*
This file is part of sasTCL.

sasTCL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCL.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasTCL__tclinvoker_h
#define sasTCL__tclinvoker_h

#include "config.h"
#include <sasCore/invoker.h>
#include SAS_TCL__TCL_H

#include <string>

namespace SAS {

	struct TCLInvoker_priv;
	class SAS_TCL__CLASS TCLInvoker : public Invoker
	{
		friend struct TCLInvoker_priv;

		SAS_COPY_PROTECTOR(TCLInvoker)
	public:

		class BlobHandler
		{
		public:
			virtual void lock() = 0;
			virtual void unlock() = 0;

			virtual inline ~BlobHandler() { }
			virtual void addBlob(const std::string & name, const std::vector<char> & data) = 0;
			virtual void addBlob(const std::string & name, const char * data, size_t size) = 0;
			virtual void setBlob(const std::string & name, std::vector<char> *& data) = 0;
			virtual bool getAll(std::vector<std::pair<std::string, std::vector<char>*>> & ret, ErrorCollector & ec) = 0;
			virtual bool getBlob(const std::string & name, std::vector<char> *& data, ErrorCollector & ec) = 0;
			virtual void removeBlob(const std::string & name) = 0;
			virtual void removeAll() = 0;
		};

		TCLInvoker(const std::string & name, Tcl_Interp * interp);
		TCLInvoker(const std::string & name);
		virtual ~TCLInvoker();

		bool init(ErrorCollector & ec);

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) override;

		BlobHandler * blobHandler() const;

	protected:
		virtual void init(Tcl_Interp *interp);

		virtual BlobHandler * createBlobHandler() const;

	private:
		TCLInvoker_priv * priv;
	};
}

#endif // sasTCL__tclinvoker_h
