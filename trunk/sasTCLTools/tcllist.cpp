/*
This file is part of sasTCLTools.

sasTCLTools is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCLTools is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCLTools.  If not, see <http://www.gnu.org/licenses/>
*/

#include "include/sasTCLTools/tcllist.h"

#include <list>

namespace SAS {

	struct TCLList_priv
	{
		TCLList_priv() : isNull(true)
		{ }

		std::list<std::string> lst;

		bool isNull;
	};

	TCLList::TCLList(const std::string & str) : priv(new TCLList_priv)
	{
		fromString(str);
	}

	TCLList::TCLList() : priv(new TCLList_priv)
	{ }

	TCLList::TCLList(const TCLList & o) : priv(new TCLList_priv(*o.priv))
	{ }

	TCLList::~TCLList()
	{
		delete priv;
	}

	bool TCLList::isNull() const
	{
		return priv->isNull;
	}

	size_t TCLList::length() const
	{
		return priv->lst.size();
	}

	TCLList & TCLList::operator = (const TCLList & o)
	{
		*priv = *o.priv;
		return *this;
	}

	bool TCLList::fromString(const std::string & str)
	{
		priv->lst.clear();
		priv->isNull = true;

		int cnt(0);
		bool escape_active(false);
		std::string str_tmp;

		auto add = [&](const std::string & str) -> bool
		{
			priv->lst.push_back(str);
			return true;
		};

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
				if (!add(str_tmp))
					return false;
				str_tmp.clear();
				continue;
			}

			if (ch == '{' && ++cnt == 1)
			{
				str_tmp.clear();
				continue;
			}

			if (ch == '}' && --cnt == 0)
			{
				if (!add(str_tmp))
					return false;
				str_tmp.clear();
				escape_active = true;
				continue;
			}

			str_tmp += ch;
		}
		if (str_tmp.length())
		{
			if (!add(str_tmp))
				return false;
		}
		if (cnt)
		{
			priv->lst.clear();
			priv->isNull = true;
			return false;
		}
		priv->isNull = false;
		return true;
	}

	TCLList & TCLList::operator << (const std::string & str)
	{
		append(str);
		return *this;
	}

	TCLList & TCLList::operator << (const TCLList & o)
	{
		append(o);
		return *this;
	}

	void TCLList::append(const TCLList & o)
	{
		priv->lst.push_back(o.toString());
	}

	void TCLList::append(const std::string & str)
	{
		priv->lst.push_back(str);
	}

	const std::string & TCLList::operator [] (size_t idx) const
	{
		return at(idx);
	}

	const std::string & TCLList::at(size_t idx) const
	{
		static const std::string null_value;
		if (idx >= priv->lst.size())
			return null_value;
		size_t i(0);
		for (const auto & d : priv->lst)
			if (i++ == idx)
				return d;
		return null_value;
	}

	TCLList TCLList::getList(size_t idx) const
	{
		return TCLList(at(idx));
	}

	std::string TCLList::toString() const
	{
		std::string ret;
		int last_idx = priv->lst.size() - 1;
		int idx(0);
		for each(auto e in priv->lst)
		{
			if (e.find(' ') == -1)
				ret += e;
			else
				ret += "{" + e + "}";

			if (idx++ < last_idx)
				ret += " ";
		}
		return ret;
	}

}
