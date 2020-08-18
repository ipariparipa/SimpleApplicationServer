/*
    This file is part of sasSQLClient.

    sasSQLClient is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasSQLClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasSQLClient.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef SC_MODULE_H_
#define SC_MODULE_H_

#include <sasCore/module.h>

namespace SAS {

class Application;

namespace SQLClient {

struct SC_Module_priv;

class SC_Module : public Module
{
	SAS_COPY_PROTECTOR(SC_Module)
public:
    SC_Module(Application * app, const std::string & name);
	virtual ~SC_Module();

	virtual std::string description() const final;
	virtual std::string version() const final;
	virtual std::string name() const final;

    bool init(ErrorCollector & ec);

protected:
	virtual Session * createSession(SessionID id, ErrorCollector & ec) final;

private:
	SC_Module_priv * priv;
};

}}

#endif /* SC_MODULE_H_ */
