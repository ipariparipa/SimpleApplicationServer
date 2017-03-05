
#pragma once;

#include <msclr/marshal_cppstd.h>

#define TO_STR(mstr) (mstr ? msclr::interop::marshal_as<std::string>(mstr) : std::string())

#define TO_MSTR(str) (str.length() ? gcnew System::String(str.c_str()) : System::String::Empty)

