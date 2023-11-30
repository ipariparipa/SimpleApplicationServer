/*
This file is part of sasODBC.

sasODBC is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasODBC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasODBC.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasODBC__include_odbc_h
#define sasODBC__include_odbc_h

#include "config.h"

#if SAS_OS == SAS_OS_LINUX 
#  include <sql.h>
#  include <sqlext.h>

#  define SAS_ODBC__PCB_VALUE_TYPE SQLLEN
#  define SAS_ODBC__SIZE_TYPE SQLULEN
#  define SAS_ODBC__LENGTH_TYPE SQLLEN

#elif SAS_OS == SAS_OS_WINDOWS 
#  include <Windows.h>
#  include <sqlext.h> 

#endif

#endif // sasODBC__include_odbc_h
