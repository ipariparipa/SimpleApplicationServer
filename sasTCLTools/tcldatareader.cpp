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

#include "include/sasTCLTools/tcldatareader.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <stdint.h>
#include <cstring>

namespace SAS {

	struct TCLDataReader_priv
	{
		TCLDataReader_priv() : ver(0), logger(Logging::getLogger("SAS.TCLDataReader"))
		{ }
		int ver;
		std::list<std::string> tclResults;
		std::map<std::string, std::vector<char>> blobs;
		Logging::LoggerPtr logger;
	};

	TCLDataReader::TCLDataReader() : priv(new TCLDataReader_priv)
	{ }

	TCLDataReader::~TCLDataReader()
	{
		delete priv;
	}

	bool TCLDataReader::read(const std::vector<char> & data, ErrorCollector & ec)
	{
		SAS_LOG_NDC();
		auto & out = data;
		priv->tclResults.clear();
		priv->blobs.clear();
		size_t out_size = out.size();
		if (out_size < sizeof(uint16_t))
		{
			auto err = ec.add(-1, "missing data version");
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		const char * out_data = out.data();
		uint16_t version;
		std::memcpy(&version, out_data, sizeof(uint16_t));
		out_data += sizeof(uint16_t); out_size -= sizeof(uint16_t);

		SAS_LOG_VAR(priv->logger, version);

		switch (version)
		{
		case 1:
		{
			while (out_size)
			{
				if (out_size < 4 + sizeof(uint32_t))
				{
					auto err = ec.add(-1, "missing or invalid data header");
					SAS_LOG_ERROR(priv->logger, err);
					return false;
				}
				std::string format;
				format.append(out_data, 4);
				out_data += 4; out_size -= 4;

				uint32_t data_size;
				std::memcpy(&data_size, out_data, sizeof(uint32_t));
				out_data += sizeof(uint32_t); out_size -= sizeof(uint32_t);
				if (out_size < data_size)
				{
					auto err = ec.add(-1, "invalid size information in data header: '" + std::to_string(data_size) + "'");
					SAS_LOG_ERROR(priv->logger, err);
					return false;
				}

				if (format == "TCLR")
				{
					std::string result;
					result.append(out_data, data_size);
					out_data += data_size; out_size -= data_size;
					priv->tclResults.push_back(result);
				}
				else if (format == "BLOB")
				{
					if (data_size < sizeof(uint32_t))
					{
						auto err = ec.add(-1, "BLOB: missing size information for blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return false;
					}

					uint16_t name_size;
					std::memcpy(&name_size, out_data, sizeof(uint16_t));
					out_data += sizeof(uint16_t); out_size -= sizeof(uint16_t);  data_size -= sizeof(uint16_t);

					if (data_size < name_size)
					{
						auto err = ec.add(-1, "BLOB: missing blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return false;
					}
					std::string name;
					name.append(out_data, name_size);
					out_data += name_size; out_size -= name_size; data_size -= name_size;
					SAS_LOG_VAR(priv->logger, name);
					if (!name.length())
					{
						auto err = ec.add(-1, "BLOB: blob name is not specified");
						SAS_LOG_ERROR(priv->logger, err);
						return false;
					}
					std::vector<char> blob(data_size);
					std::memcpy(blob.data(), out_data, data_size);
					out_data += data_size; out_size -= data_size;
					priv->blobs[name] = blob;
				}
			}

			break;
		}
		default:
		{
			auto err = ec.add(-1, "unsupported data version: " + std::to_string(version));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		}
		return true;
	}

	short TCLDataReader::version() const
	{
		return priv->ver;
	}

	const std::list<std::string> & TCLDataReader::tclResults() const
	{
		return priv->tclResults;
	}

	const std::map<std::string, std::vector<char>> & TCLDataReader::blobs() const
	{
		return priv->blobs;
	}

}

