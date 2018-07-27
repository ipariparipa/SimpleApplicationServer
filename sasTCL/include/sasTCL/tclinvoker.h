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
#include "tclinterpinitilizer.h"
#include SAS_TCL__TCL_H

#include <string>

namespace SAS {

	class TCLExecutorPool;

	class SAS_TCL__CLASS TCLBlobHandler
	{
	public:
		virtual void lock() = 0;
		virtual void unlock() = 0;

		virtual inline ~TCLBlobHandler() { }
		virtual void addBlob(const std::string & name, const std::vector<unsigned char> & data) = 0;
		virtual void addBlob(const std::string & name, const unsigned char * data, size_t size) = 0;
		virtual void setBlob(const std::string & name, std::vector<unsigned char> *& data) = 0;
		virtual bool getAll(std::vector<std::pair<std::string, std::vector<unsigned char>*>> & ret, ErrorCollector & ec) = 0;
		virtual bool getBlob(const std::string & name, std::vector<unsigned char> *& data, ErrorCollector & ec) = 0;
		virtual void removeBlob(const std::string & name) = 0;
		virtual void removeAll() = 0;
	};

	class SAS_TCL__CLASS TCLInvoker : public Invoker, protected TCLInterpInitializer
	{
		struct Priv;
		Priv * priv;

		friend struct TCLInvoker_priv;

		SAS_COPY_PROTECTOR(TCLInvoker)
	public:
		typedef TCLBlobHandler BlobHandler;
		TCLInvoker(const std::string & name, TCLExecutorPool * exec_pool);
		virtual ~TCLInvoker();

		bool init(ErrorCollector & ec);

		virtual Status invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec) override;

		BlobHandler * blobHandler() const;

	protected:
		virtual void init(Tcl_Interp *interp) override;

		virtual BlobHandler * createBlobHandler() const;
	};
}

#endif // sasTCL__tclinvoker_h
