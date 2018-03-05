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

#ifndef sasTCL__errorcodes_h
#define sasTCL__errorcodes_h

#include <sasCore/errorcodes.h>

#define _SAS_TCL__ERROR_BASE_ 4100

#define SAS_TCL__ERROR__CONFIG_READER__CANNOT_GET_ENTRY SAS_CORE__ERROR__CONFIG_READER__CANNOT_GET_ENTRY
#define SAS_TCL__ERROR__CONFIG_READER__ENTRY_NOT_FOUND SAS_CORE__ERROR__CONFIG_READER__ENTRY_NOT_FOUND
#define SAS_TCL__ERROR__CONFIG_READER__UNEXPECTED_ERROR SAS_CORE__ERROR__CONFIG_READER__UNEXPECTED_ERROR
#define SAS_TCL__ERROR__CONFIG_READER__INVALID_ARGUMENT SAS_CORE__ERROR__CONFIG_READER__INVALID_ARGUMENT
#define SAS_TCL__ERROR__CONFIG_READER__CANNOT_READ_CONFIG SAS_CORE__ERROR__CONFIG_READER__CANNOT_READ_CONFIG
#define SAS_TCL__ERROR__CONFIG_READER__INVALID_LIST_VALUE SAS_CORE__ERROR__CONFIG_READER__INVALID_LIST_VALUE
#define SAS_TCL__ERROR__INVOKER__INVALID_DATA SAS_CORE__ERROR__INVOKER__INVALID_DATA
#define SAS_TCL__ERROR__INVOKER__UNSUPPORTED_DATA SAS_CORE__ERROR__INVOKER__UNSUPPORTED_DATA
//#define SAS_TCL__ERROR__ SAS_CORE__ERROR__
//#define SAS_TCL__ERROR__ SAS_CORE__ERROR__
//#define SAS_TCL__ERROR__ SAS_CORE__ERROR__
//#define SAS_TCL__ERROR__ SAS_CORE__ERROR__
//#define SAS_TCL__ERROR__ SAS_CORE__ERROR__
//#define SAS_TCL__ERROR__ SAS_CORE__ERROR__

#define SAS_TCL__ERROR__EXECUTOR__CANNOT_RUN_SCRIPT _SAS_TCL__ERROR_BASE_+1
#define SAS_TCL__ERROR__BLOB_HANDLER__NOT_FOUND _SAS_TCL__ERROR_BASE_+2
#define SAS_TCL__ERROR__RUN_TIMEOUT _SAS_TCL__ERROR_BASE_+3
#define SAS_TCL__ERROR__UNEXPECTED _SAS_TCL__ERROR_BASE_+4
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+5
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+6
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+7
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+8
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+9
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+10
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+11
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+12
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+13
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+14
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+15
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+16
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+17
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+18
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+19
//#define SAS_TCL__ERROR__ _SAS_TCL__ERROR_BASE_+20

#endif //sasTCL__errorcodes_h
