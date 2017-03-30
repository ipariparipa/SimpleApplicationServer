/*
    This file is part of sas.

    sas is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sas.  If not, see <http://www.gnu.org/licenses/>
 */

#include "sasserver.h"
#include "version.h"
#include <assert.h>
#include <sasBasics/envconfigreader.h>
#include <sasCore/componentloader.h>
#include <sasCore/component.h>
#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace SAS {

struct SASServer_priv
{
	SASServer_priv() : configreader(new EnvConfigReader())
	{ }

	std::unique_ptr<ConfigReader> configreader;

};

SASServer::SASServer(int argc, char ** argv) : Server(argc, argv), priv(new SASServer_priv)
{ }

SASServer::~SASServer()
{ delete priv; }

std::string SASServer::version() const
{
	return SAS_SERVER_VERSION;
}

ConfigReader * SASServer::configReader()
{
	return priv->configreader.get();
}

}
