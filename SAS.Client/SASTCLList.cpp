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

#include "SASTCLList.h"
#include "macros.h"

#include <msclr/gcroot.h>

using namespace msclr;

namespace SAS {
	namespace Client{

		struct SASTCLList_priv
		{
			gcroot<System::Collections::Generic::List<System::String ^> ^ > elements;
		};

		SASTCLList::SASTCLList(System::String ^ str) : priv(new SASTCLList_priv)
		{
			priv->elements = gcnew System::Collections::Generic::List<System::String ^>();
			int cnt(0);
			bool escape_active(false);
			auto str_tmp = System::String::Empty;
			for each(auto ch in str)
			{
				if (escape_active)
					escape_active = false;
				else if (ch == '\\')
				{
					escape_active = true;
					continue;
				}
				else if (cnt == 0 && ch == ' ')
				{
					priv->elements->Add(str_tmp);
					str_tmp = System::String::Empty;
					continue;
				}

				if (ch == '{' && ++cnt == 1)
				{
					str_tmp = System::String::Empty;
					continue;
				}
					
				if (ch == '}' && --cnt == 0)
				{
					priv->elements->Add(str_tmp);
					str_tmp = System::String::Empty;
					escape_active = true;
					continue;
				}

				str_tmp += ch;
			}
			if (!System::String::IsNullOrEmpty(str_tmp))
				priv->elements->Add(str_tmp);
		}

		SASTCLList::!SASTCLList()
		{
			delete priv;
		}

		//property 
		System::Collections::Generic::List<System::String ^> ^ SASTCLList::Elements::get()
		{
			return priv->elements;
		}

		System::String ^ SASTCLList::ToString()
		{
			auto ret = System::String::Empty;
			int len = priv->elements->Count;
			int idx(0);
			System::Collections::Generic::List<System::String ^> ^ tmp = priv->elements;
			for each(auto e in tmp)
			{
				if (e->IndexOf(" ") == -1)
					ret += e;
				else
					ret += "{" + e + "}";

				if (idx++ < len - 1)
					ret += " ";
			}
			return ret;
		}

		void SASTCLList::Add(System::String ^ str)
		{
			priv->elements->Add(str);
		}

		void SASTCLList::Add(SASTCLList ^ lst)
		{
			priv->elements->Add(lst->ToString());
		}


	}
}