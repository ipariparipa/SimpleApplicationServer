/*
This file is part of sasTCLClient.

sasTCLClient is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

sasTCLClient is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with sasTCLClient.  If not, see <http://www.gnu.org/licenses/>
*/

#include "include/sasTCL/tcllist.h"

namespace SAS {

	struct TCLList_priv
	{
		Tcl_Obj * obj;
		Tcl_Interp * interp;
	};

	TCLList::TCLList() : priv(new TCLList_priv)
	{
		priv->interp = nullptr;
		priv->obj = nullptr;
	}

	TCLList::TCLList(Tcl_Interp * interp) : priv(new TCLList_priv)
	{
		priv->obj = Tcl_NewListObj(0, NULL);
		priv->interp = interp;
	}

	TCLList::TCLList(Tcl_Interp * interp, Tcl_Obj * obj) : priv(new TCLList_priv)
	{
		priv->interp = interp;
		priv->obj = obj;
	}

	TCLList::TCLList(const TCLList & o) : priv(new TCLList_priv(*o.priv))
	{ }

	TCLList::~TCLList()
	{
		delete priv;
	}

	TCLList & TCLList::operator = (const TCLList & o)
	{
		*priv = *o.priv;
		return *this;
	}

	bool TCLList::isNull() const
	{
		return !priv->interp;
	}

	bool TCLList::append(const std::string & str)
	{
		if (!priv->interp)
			return false;
		return (Tcl_ListObjAppendElement(priv->interp, priv->obj, Tcl_NewStringObj(str.c_str(), -1)) == TCL_OK);
	}

	bool TCLList::append(const TCLList & lst)
	{
		return (Tcl_ListObjAppendElement(priv->interp, priv->obj, lst.priv->obj) == TCL_OK);
	}

	int TCLList::length() const
	{
		if (!priv->interp)
			return false;
		int ret;
		if (Tcl_ListObjLength(priv->interp, priv->obj, &ret) != TCL_OK)
			return -1;
		return ret;
	}

	bool TCLList::fromString(const std::string & str)
	{
		auto lst_obj = Tcl_NewStringObj(str.c_str(), -1);
		if(!lst_obj)
			return false;
		int objc;
		Tcl_Obj **objv;
		if(Tcl_ListObjGetElements(priv->interp, lst_obj, &objc, &objv) != TCL_OK)
			return false;
		for(int i = 0; i < objc; ++i)
			if(Tcl_ListObjAppendElement(priv->interp, priv->obj, objv[i]) != TCL_OK)
				return false;
		return true;
	}

	std::string TCLList::operator [] (int idx) const
	{
		if (!priv->interp)
			return std::string();
		Tcl_Obj * tmp;
		if (Tcl_ListObjIndex(priv->interp, priv->obj, idx, &tmp) != TCL_OK)
			return std::string();
		return Tcl_GetString(tmp);
	}

	std::string TCLList::getString(int idx) const
	{
		if (!priv->interp)
			return std::string();
		Tcl_Obj * tmp;
		if (Tcl_ListObjIndex(priv->interp, priv->obj, idx, &tmp) != TCL_OK)
			return std::string();
		return Tcl_GetString(tmp);
	}
	
	TCLList TCLList::getList(int idx) const
	{
		if (!priv->interp)
			return TCLList();
		Tcl_Obj * tmp;
		if (Tcl_ListObjIndex(priv->interp, priv->obj, idx, &tmp) != TCL_OK)
			return TCLList();
		return TCLList(priv->interp, tmp);
	}

	Tcl_Obj * TCLList::obj() const
	{
		return priv->obj;
	}


}
