/*
    This file is part of sasMySQL.

    sasMySQL is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sasMySQL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sasMySQL.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef SASMYSQL_CONFIG_H
#define SASMYSQL_CONFIG_H

#include <sasCore/config.h>

#if SAS_OS == SAS_OS_LINUX 
#  define SAS_MYSQL__MYSQL_H <mysql/mysql.h>
#  define SAS_MYSQL__CLASS 
#  define SAS_MYSQL__FUNCTION 
#elif SAS_OS == SAS_OS_WINDOWS 
#  define SAS_MYSQL__MYSQL_H <mysql.h>
#  ifdef SAS_MYSQL__IMPL
#    define SAS_MYSQL__CLASS __declspec(dllexport)
#    define SAS_MYSQL__FUNCTION __declspec(dllexport)
#  else
#    define SAS_MYSQL__CLASS __declspec(dllimport)
#    define SAS_MYSQL__FUNCTION __declspec(dllimport)
#  endif
#endif

#endif // SASMYSQL_CONFIG_H
