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

using System.Xml.Linq;
using System.Runtime.Serialization.Json;
using System.IO;

using PIDL;

namespace SAS.Client.PIDL
{
	public class SASPIDLJSONClientHelper
    {
		ISASConnection conn;

		public SASPIDLJSONClientHelper(ISASConnection conn_)
		{
			conn = conn_;
		}

		public enum InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};
		public InvokeStatus invoke(XElement root, out XElement ret, IPIDLErrorCollector ec_)
		{
			var ec = new PIDL_SASErrorCollector(ec_);

			var in_stream = new MemoryStream();
			var in_writer = JsonReaderWriterFactory.CreateJsonWriter(in_stream);
			var in_doc = new XDocument();
			in_doc.Add(root);
			in_doc.WriteTo(in_writer);
			in_writer.Close();

			SASBinData out_data;
			switch(conn.Invoke(new SASBinData(in_stream.GetBuffer()), out out_data, ec))
			{
				case ISASInvoker.Status.Error:
					ret = null;
					return InvokeStatus.Error;
				case ISASInvoker.Status.FatalError:
					ret = null;
					return InvokeStatus.FatalError;
				case ISASInvoker.Status.NotImplemented:
					ret = null;
					return InvokeStatus.NotImplemented;
				case ISASInvoker.Status.OK:
					break;
			}
			
			var out_reader = JsonReaderWriterFactory.CreateJsonReader(out_data.AsByteArray, System.Xml.XmlDictionaryReaderQuotas.Max);
			ret = XElement.Load(out_reader);
			return InvokeStatus.Ok;
		}

    }
}
