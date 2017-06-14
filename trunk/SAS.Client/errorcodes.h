/*
This file is part of SAS.Client.

SAS.Client is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SAS.Client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with SAS.Client.  If not, see <http://www.gnu.org/licenses/>
*/

#pragma once

#include <sasCore/errorcodes.h>

#define _SAS_CLIENT__ERROR__BASE_ 6100

#define SAS_CLIENT__ERROR__CONFIG_READER__CANNOT_GET_ENTRY SAS_CORE__ERROR__CONFIG_READER__CANNOT_GET_ENTRY
#define SAS_CLIENT__ERROR__CONFIG_READER__CANNOT_READ_CONFIG SAS_CORE__ERROR__CONFIG_READER__CANNOT_READ_CONFIG
#define SAS_CLIENT__ERROR__CONFIG_READER__ENTRY_NOT_FOUND SAS_CORE__ERROR__CONFIG_READER__ENTRY_NOT_FOUND
#define SAS_CLIENT__ERROR__CONFIG_READER__INVALID_LIST_VALUE SAS_CORE__ERROR__CONFIG_READER__INVALID_LIST_VALUE
#define SAS_CLIENT__ERROR__CONFIG_READER__TYPE_MISMATCH SAS_CORE__ERROR__CONFIG_READER__TYPE_MISMATCH
#define SAS_CLIENT__ERROR__CONFIG_READER__UNEXPECTED_ERROR SAS_CORE__ERROR__CONFIG_READER__UNEXPECTED_ERROR
//#define SAS_CLIENT__ERROR__ SAS_CORE__ERROR__

#define SAS_CLIENT__ERROR__TCL_DATA_READER__INVALID_DATA _SAS_CLIENT__ERROR__BASE_+1
#define SAS_CLIENT__ERROR__TCL_DATA_READER__UNSUPPORTED_VERSION _SAS_CLIENT__ERROR__BASE_+2
#define SAS_CLIENT__ERROR__TCL_DATA_WRITER__UNSUPPORTED_VERSION _SAS_CLIENT__ERROR__BASE_+3
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+4
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+5
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+6
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+7
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+8
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+9
//#define SAS_CLIENT__ERROR__ _SAS_CLIENT__ERROR__BASE_+10
