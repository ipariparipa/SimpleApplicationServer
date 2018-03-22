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

#include "include/sasTCL/tcllisthandler.h"

#include SAS_TCL__TCL_H

namespace SAS {

	struct TCLListHandler::Priv
	{
		TCLObjectRef obj;
		Tcl_Interp * interp = nullptr;
	};

	TCLListHandler::TCLListHandler() : priv(new Priv)
	{ }

	TCLListHandler::TCLListHandler(Tcl_Interp * interp) : priv(new Priv)
	{
		priv->obj = Tcl_NewListObj(0, NULL);
		priv->interp = interp;
	}

	TCLListHandler::TCLListHandler(Tcl_Interp * interp, const TCLObjectRef & obj) : priv(new Priv)
	{
		priv->interp = interp;
		priv->obj = obj;
	}

	TCLListHandler::TCLListHandler(const TCLListHandler & o) : priv(new Priv(*o.priv))
	{ }

	TCLListHandler::~TCLListHandler()
	{ delete priv; }

	TCLListHandler & TCLListHandler::operator = (const TCLListHandler & o)
	{
		*priv = *o.priv;
		return *this;
	}

	bool TCLListHandler::isNull() const
	{
		return priv->obj.isNull();
	}

	bool TCLListHandler::append(const std::string & str)
	{
		if (!priv->interp)
			return false;
		return (Tcl_ListObjAppendElement(priv->interp, priv->obj, TCLObjectRef(Tcl_NewStringObj(str.c_str(), -1))) == TCL_OK);
	}

	bool TCLListHandler::append(const TCLListHandler & lst)
	{
		return (Tcl_ListObjAppendElement(priv->interp, priv->obj, lst.priv->obj) == TCL_OK);
	}

	bool TCLListHandler::append(const TCLObjectRef & obj)
	{
		return (Tcl_ListObjAppendElement(priv->interp, priv->obj, obj) == TCL_OK);
	}

	int TCLListHandler::length() const
	{
		if (!priv->interp)
			return false;
		int ret;
		if (Tcl_ListObjLength(priv->interp, priv->obj, &ret) != TCL_OK)
			return -1;
		return ret;
	}

	bool TCLListHandler::fromString(const std::string & str)
	{
		TCLObjectRef lst_obj(Tcl_NewStringObj(str.c_str(), -1));
		if(lst_obj.isNull())
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

	std::string TCLListHandler::operator [] (int idx) const
	{
		if (!priv->interp)
			return std::string();
		Tcl_Obj * tmp;
		if (Tcl_ListObjIndex(priv->interp, priv->obj, idx, &tmp) != TCL_OK)
			return std::string();
		return Tcl_GetString(tmp);
	}

	std::string TCLListHandler::getString(int idx) const
	{
		if (!priv->interp)
			return std::string();
		Tcl_Obj * tmp;
		if (Tcl_ListObjIndex(priv->interp, priv->obj, idx, &tmp) != TCL_OK)
			return std::string();
		return Tcl_GetString(tmp);
	}
	
	TCLListHandler TCLListHandler::getList(int idx) const
	{
		if (!priv->interp)
			return TCLListHandler();
		Tcl_Obj * tmp;
		if (Tcl_ListObjIndex(priv->interp, priv->obj, idx, &tmp) != TCL_OK)
			return TCLListHandler();
		return TCLListHandler(priv->interp, tmp);
	}

	std::string TCLListHandler::toString() const
	{
		return Tcl_GetString(priv->obj);
	}

	TCLObjectRef TCLListHandler::obj() const
	{
		return priv->obj;
	}

}
