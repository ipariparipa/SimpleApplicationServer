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

#include "include/sasTCL/tclobjectref.h"

#include SAS_TCL__TCL_H

namespace SAS {

	struct TCLObjectRef::Priv
	{
		Tcl_Obj * obj = nullptr;
	};

	TCLObjectRef::TCLObjectRef() : priv(new Priv)
	{ }

	TCLObjectRef::TCLObjectRef(Tcl_Obj * obj) : priv(new Priv)
	{
		if((priv->obj = obj))
			Tcl_IncrRefCount(priv->obj);
	}

	TCLObjectRef::TCLObjectRef(const TCLObjectRef & o) : priv(new Priv)
	{
		if((priv->obj = o.priv->obj))
			Tcl_IncrRefCount(priv->obj);
	}

	TCLObjectRef::~TCLObjectRef()
	{
		if(priv->obj)
			Tcl_DecrRefCount(priv->obj);
		delete priv;
	}

	TCLObjectRef & TCLObjectRef::operator = (const TCLObjectRef & o)
	{
		if(priv->obj)
			Tcl_DecrRefCount(priv->obj);
		if((priv->obj = o.priv->obj))
			Tcl_IncrRefCount(priv->obj);
		return *this;
	}

	TCLObjectRef & TCLObjectRef::operator = (Tcl_Obj * obj)
	{
		if(priv->obj)
			Tcl_DecrRefCount(priv->obj);
		if((priv->obj = obj))
			Tcl_IncrRefCount(priv->obj);
		return *this;
	}

	bool TCLObjectRef::isNull() const
	{
		return !priv->obj;
	}

	TCLObjectRef::operator Tcl_Obj * () const
	{
		return priv->obj;
	}

	Tcl_Obj * TCLObjectRef::obj() const
	{
		return priv->obj;
	}
}
