/*
This file is part of SAS.Client.PIDL.

SAS.Client.PIDL is free software: you can redistribute it and/or modify
it under the terms of the Lesser GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SAS.Client.PIDL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with SAS.Client.PIDL.  If not, see <http://www.gnu.org/licenses/>
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using PIDL;

namespace SAS.Client.PIDL
{
	public class SAS_PIDLErrorCollector : IPIDLErrorCollector
	{
		ISASErrorCollector ec;

		public SAS_PIDLErrorCollector(ISASErrorCollector ec_)
		{
			ec = ec_;
		}

		public void Add(int code, string msg)
		{
			ec.Add(code, msg);
		}

		public void Add(PIDLError err)
		{
			ec.Add(err.code, err.msg);
		}

	}
}
