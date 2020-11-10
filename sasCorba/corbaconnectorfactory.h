/*
    This file is part of sasCorba.

    sasCorba is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCorba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCorba.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef sasCorba__CORBACONNECTORFACTORY_H
#define sasCorba__CORBACONNECTORFACTORY_H

#include <sasCore/connectorfactory.h>
#include <omniORB4/CORBA.h>

#include <memory>

namespace SAS {

    class Application;

    class CorbaConnectorFactory : public ConnectorFactory
    {
        SAS_COPY_PROTECTOR(CorbaConnectorFactory)

        struct Priv;
        std::unique_ptr<Priv> p;
    public:
        CorbaConnectorFactory(const std::string & name, Application * app);

        virtual ~CorbaConnectorFactory() override;

        std::string name() const final override;

        bool init(const CORBA::ORB_var & orb, const std::string & path, ErrorCollector & ec);

        Connector * make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) final override;
    };

}

#endif // sasCorba__CORBACONNECTORFACTORY_H
