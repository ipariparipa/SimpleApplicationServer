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

#include "include/sasTCLTools/tcldatawriter.h"

#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>

#include <iterator>
#include <stdint.h>
#include <string.h>

namespace SAS {

	struct TCLDataWriter_priv
	{
		TCLDataWriter_priv() : logger(Logging::getLogger("SAS.TCLDataWriter")), ver(0)
		{ }

		std::vector<char> data;
		Logging::LoggerPtr logger;
		short ver;
	};

	TCLDataWriter::TCLDataWriter(short ver) : priv(new TCLDataWriter_priv)
	{
		priv->ver = ver;
		auto & in = priv->data;
		in.resize(2);
		uint16_t tmp_16 = ver;
		memcpy(in.data(), &tmp_16, sizeof(uint16_t));
	}

	TCLDataWriter::~TCLDataWriter()
	{
		delete priv;
	}

	bool TCLDataWriter::addScript(const std::string & script, ErrorCollector & ec)
	{
		auto & in = priv->data;

		switch (priv->ver)
		{
		case 1:
		{
			std::vector<char> header(4 + sizeof(uint32_t));
			char * header_data = header.data();
			memcpy(header_data, "TCLS", 4);
			header_data += 4;
			uint32_t tmp_32 = script.size();
			memcpy(header_data, &tmp_32, sizeof(uint32_t));
			header_data += sizeof(uint32_t);

			in.insert(std::end(in), header.begin(), header.end());
			in.insert(std::end(in), script.begin(), script.end());
			break;
		}
		default:
		{
			auto err = ec.add(-1, "unsupported data version: " + std::to_string(priv->ver));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		}
		return true;
	}

	bool TCLDataWriter::addBlobSetter(const std::string & blob_name, std::vector<char> & data, ErrorCollector & ec)
	{
		auto & in = priv->data;
		auto & blob = data;

		switch (priv->ver)
		{
		case 1:
		{
			std::vector<char> header(4 + sizeof(uint16_t) + sizeof(uint32_t));
			char * header_data = header.data();
			memcpy(header_data, "BADD", 4);
			header_data += 4;

			uint32_t tmp_32 = sizeof(uint16_t) + blob_name.size() + blob.size();
			memcpy(header_data, &tmp_32, sizeof(uint32_t));
			header_data += sizeof(uint32_t);

			uint16_t tmp_16 = blob_name.size();
			memcpy(header_data, &tmp_16, sizeof(uint16_t));
			header_data += sizeof(uint16_t);

			in.insert(std::end(in), header.begin(), header.end());
			in.insert(std::end(in), blob_name.begin(), blob_name.end());
			in.insert(std::end(in), blob.begin(), blob.end());
			break;
		}
		default:
		{
			auto err = ec.add(-1, "unsupported data version: " + std::to_string(priv->ver));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		}
		return true;
	}

	bool TCLDataWriter::addBlobGetter(ErrorCollector & ec)
	{
		return addBlobGetter(std::string(), ec);
	}

	bool TCLDataWriter::addBlobGetter(const std::string & blob_name, ErrorCollector & ec)
	{
		auto & in = priv->data;

		switch (priv->ver)
		{
		case 1:
		{
			std::vector<char> header(4 + sizeof(uint16_t) + sizeof(uint32_t));
			char * header_data = header.data();
			memcpy(header_data, "BGET", 4);
			header_data += 4;

			uint32_t tmp_32 = sizeof(uint16_t) + blob_name.size();
			memcpy(header_data, &tmp_32, sizeof(uint32_t));
			header_data += sizeof(uint32_t);

			uint16_t tmp_16 = blob_name.size();
			memcpy(header_data, &tmp_16, sizeof(uint16_t));
			header_data += sizeof(uint16_t);

			in.insert(std::end(in), header.begin(), header.end());
			in.insert(std::end(in), blob_name.begin(), blob_name.end());
			break;
		}
		default:
		{
			auto err = ec.add(-1, "unsupported data version: " + std::to_string(priv->ver));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		}
		return true;
	}

	bool TCLDataWriter::addBlobRemover(ErrorCollector & ec)
	{
		return addBlobRemover(std::string(), ec);
	}

	bool TCLDataWriter::addBlobRemover(const std::string & blob_name, ErrorCollector & ec)
	{
		auto & in = priv->data;

		switch (priv->ver)
		{
		case 1:
		{
			std::vector<char> header(4 + sizeof(uint16_t) + sizeof(uint32_t));
			char * header_data = header.data();
			memcpy(header_data, "BREM", 4);
			header_data += 4;

			uint32_t tmp_32 = sizeof(uint16_t) + blob_name.size();
			memcpy(header_data, &tmp_32, sizeof(uint32_t));
			header_data += sizeof(uint32_t);

			uint16_t tmp_16 = blob_name.size();
			memcpy(header_data, &tmp_16, sizeof(uint16_t));
			header_data += sizeof(uint16_t);

			in.insert(std::end(in), header.begin(), header.end());
			in.insert(std::end(in), blob_name.begin(), blob_name.end());
			break;
		}
		default:
		{
			auto err = ec.add(-1, "unsupported data version: " + std::to_string(priv->ver));
			SAS_LOG_ERROR(priv->logger, err);
			return false;
		}
		}
		return true;
	}

	const std::vector<char> & TCLDataWriter::data() const
	{
		return priv->data;
	}

}
