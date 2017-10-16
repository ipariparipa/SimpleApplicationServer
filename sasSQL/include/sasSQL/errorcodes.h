/*
This file is part of sasSQL.

sasSQL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasSQL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasSQL.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef sasSQL__errorcodes_h
#define sasSQL__errorcodes_h

#define _SAS_SQL__ERROR__BASE_ 2100

#define SAS_SQL__ERROR__CANNOT_INIT_CONNECTOR_LIB  _SAS_SQL__ERROR__BASE_+1
#define SAS_SQL__ERROR__CANNOT_CONNECT_TO_DB_SEVICE  _SAS_SQL__ERROR__BASE_+2
#define SAS_SQL__ERROR__INVALID_CONFIG_VALUE  _SAS_SQL__ERROR__BASE_+3
#define SAS_SQL__ERROR__CANNOT_EXECUTE_STATEMENT  _SAS_SQL__ERROR__BASE_+4
#define SAS_SQL__ERROR__CANNOT_PREPARE_STATEMENT  _SAS_SQL__ERROR__BASE_+5
#define SAS_SQL__ERROR__CANNOT_BIND_PARAMETERS  _SAS_SQL__ERROR__BASE_+6
#define SAS_SQL__ERROR__CANNOT_BIND_RESULT  _SAS_SQL__ERROR__BASE_+7
#define SAS_SQL__ERROR__NO_META_INFO  _SAS_SQL__ERROR__BASE_+8
#define SAS_SQL__ERROR__NOT_SUPPORTED  _SAS_SQL__ERROR__BASE_+9
#define SAS_SQL__ERROR__NO_DATA  _SAS_SQL__ERROR__BASE_+10
#define SAS_SQL__ERROR__UNEXPECTED  _SAS_SQL__ERROR__BASE_+11

#endif // sasSQL__errorcodes_h
