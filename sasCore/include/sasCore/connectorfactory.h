/*
    This file is part of sasCore.

    sasCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef INCLUDE_SASCORE_CONNECTORFACTORY_H_
#define INCLUDE_SASCORE_CONNECTORFACTORY_H_

#include "object.h"

#define SAS_OBJECT_TYPE__CONNECTOR_FACTORY "connector_factory"

namespace SAS {

    class ErrorCollector;
    class Connector;

    class ConnectorFactory : public Object
    {
        SAS_COPY_PROTECTOR(ConnectorFactory)
    public:
        inline ConnectorFactory() : Object() { }

        virtual inline ~ConnectorFactory() override = default;

        std::string type() const final override
        { return SAS_OBJECT_TYPE__CONNECTOR_FACTORY; }

        virtual Connector * make(const std::string & name, const std::string & connectionString, ErrorCollector & ec) = 0;

    };

}

#endif // INCLUDE_SASCORE_CONNECTORFACTORY_H_
