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

#include "include/sasTCL/tclinvoker.h"
#include <sasCore/errorcollector.h>
#include <sasCore/logging.h>
#include <sasCore/thread.h>

#include "include/sasTCL/tcllisthandler.h"
#include "include/sasTCL/tclexecutor.h"
#include "include/sasTCL/errorcodes.h"

#include <memory>
#include <mutex>
#include <vector>
#include <algorithm>
#include <condition_variable>
#include <queue>
#include <cstring>

namespace SAS {


	struct TCLInvoker::Priv
	{
		Priv(TCLInvoker * obj_, const std::string & name_, TCLExecutorPool * exec_pool_) :
			obj(obj_), name(name_), exec_pool(exec_pool_),
			logger(Logging::getLogger("SAS.TCLInvoker."+name_))
		{ }

		TCLInvoker * obj;
		std::string name;

		TCLExecutorPool * exec_pool;
		TCLExecutor * exec = nullptr;

		std::mutex mut;
		Logging::LoggerPtr logger;


		struct DefaultBlobHandler : public TCLInvoker::BlobHandler
		{
			virtual void lock()
			{
				mut.lock();
			}

			virtual void unlock()
			{
				mut.unlock();
			}

			inline DefaultBlobHandler(const std::string & name) : logger(Logging::getLogger("SAS.TCLInvoker.DefaultBlobHandler." + name))
			{ }

			virtual inline ~DefaultBlobHandler() { }

			virtual void addBlob(const std::string & name, const std::vector<unsigned char> & data) final
			{
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "add blob '" + name + "' (" + std::to_string(data.size()) + ")");
				blobs[name] = data;
			}

			virtual void setBlob(const std::string & name, std::vector<unsigned char> *& data) final
			{
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "set blob");
				data = &blobs[name];
			}

			virtual void addBlob(const std::string & name, const unsigned char * data, size_t size) final
			{
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "add blob '" + name + "' (" + std::to_string(size) + ")");
				auto & b = blobs[name];
				b.resize(size);
				if (size)
					std::memcpy(b.data(), data, size);
			}

			virtual bool getBlob(const std::string & name, std::vector<unsigned char> *& ret, ErrorCollector & ec) final
			{
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "get blob '" + name + "'");
				if (!blobs.count(name))
				{
					auto err = ec.add(SAS_TCL__ERROR__BLOB_HANDLER__NOT_FOUND, "blob '" + name + "' is not found");
					SAS_LOG_ERROR(logger, err);
					return false;
				}
				ret = &blobs[name];
				return true;
			}

			virtual bool getAll(std::vector<std::pair<std::string, std::vector<unsigned char>*>> & ret, ErrorCollector & ec) final
			{
                (void)ec;
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "get all blob");
				ret.resize(blobs.size());
				size_t i(0);
				for (auto & b : blobs)
					ret[i++] = std::pair<std::string, std::vector<unsigned char>*>(b.first, &b.second);
				return true;
			}

			virtual void removeBlob(const std::string & name) final
			{
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "remove blob '"+name+"'");
				if (blobs.count(name))
					blobs.erase(name);
			}

			virtual void removeAll() final
			{
				SAS_LOG_NDC();
				SAS_LOG_TRACE(logger, "remove all blobs");
				blobs.clear();
			}

			std::map<std::string, std::vector<unsigned char>> blobs;
			Logging::LoggerPtr logger;
			std::mutex mut;
		};


		std::unique_ptr<TCLInvoker::BlobHandler> blobHandler;

	};

	TCLInvoker::TCLInvoker(const std::string & name, TCLExecutorPool * exec_pool) : Invoker(), priv(new Priv(this, name, exec_pool))
	{ }

	TCLInvoker::~TCLInvoker()
	{
		SAS_LOG_NDC();
		if (priv->exec)
			priv->exec_pool->release(priv->exec);
		delete priv;
	}

	bool TCLInvoker::init(ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		if (!priv->exec && !(priv->exec = priv->exec_pool->consume(ec)))
			return false;


		SAS_LOG_TRACE(priv->logger, "run initer");
		TCLExecutor::Run run;
		run.ec = &ec;
		run.operation = TCLExecutor::Run::Init;
		run.initer = this;
		if (!priv->exec->run(&run, ec))
			return false;

		SAS_LOG_TRACE(priv->logger, "create blob handler");
		priv->blobHandler.reset(createBlobHandler());

		return true;
	}

	//virtual 
	void TCLInvoker::init(Tcl_Interp *interp) //override
	{
        (void)interp;
		//TODO: add embedded TCL functions (e.g. blob handling)
	}

	TCLInvoker::Status TCLInvoker::invoke(const std::vector<char> & input, std::vector<char> & output, ErrorCollector & ec)
	{
		SAS_LOG_NDC();

		std::unique_lock<std::mutex> __locker(priv->mut);

		size_t in_size = input.size();
		if (in_size < sizeof(uint16_t))
		{
			auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "missing data version");
			SAS_LOG_ERROR(priv->logger, err);
			return TCLInvoker::Status::Error;
		}
		const char * in_data = input.data();
		uint16_t version;
		memcpy(&version, in_data, sizeof(uint16_t));
		in_data += sizeof(uint16_t); in_size -= sizeof(uint16_t);

		SAS_LOG_VAR(priv->logger, version);

		switch (version)
		{
		case 1:
		{
			output.resize(sizeof(uint16_t));
			memcpy(output.data(), &version, sizeof(uint16_t));

			bool has_script_error(false);

			while (in_size)
			{
				if (in_size < 4 + sizeof(uint32_t))
				{
					SAS_LOG_VAR(priv->logger, in_size);
					auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "missing or invalid data header");
					SAS_LOG_ERROR(priv->logger, err);
					return TCLInvoker::Status::Error;
				}
				std::string format;
				format.append(in_data, 4);
				in_data += 4; in_size -= 4;

				uint32_t data_size;
				memcpy(&data_size, in_data, sizeof(uint32_t));
				in_data += sizeof(uint32_t); in_size -= sizeof(uint32_t);
				if (in_size < data_size)
				{
					SAS_LOG_VAR(priv->logger, in_size);
					auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "invalid size information in data header: '" + std::to_string(data_size) + "'");
					SAS_LOG_ERROR(priv->logger, err);
					return TCLInvoker::Status::Error;
				}

				SAS_LOG_VAR(priv->logger, format);

				if (format == "TCLS")
				{
					TCLExecutor::Run run;
					run.ec = &ec;
					run.script.append(in_data, data_size);
					in_data += data_size; in_size -= data_size;

					SAS_LOG_VAR(priv->logger, run.script);

					if (!has_script_error)
					{
						if (run.script.length())
						{
							SAS_LOG_ASSERT(priv->logger, priv->exec, "executor must be set");
							if (!priv->exec->run(&run, ec))
								return TCLInvoker::Status::FatalError;
							
							if (!run.isOK)
							{
								has_script_error = true;
								//return TCLInvoker::Status::Error;
							}
							else
							{
								auto & res = run.result;
								std::vector<char> header(4 + sizeof(uint32_t));
								char * out_header = header.data();
								memcpy(out_header, "TCLR", 4);
								out_header += 4;
								uint32_t tmp_32(res.size());
								memcpy(out_header, &tmp_32, sizeof(uint32_t));
								output.insert(std::end(output), header.begin(), header.end());
								output.insert(std::end(output), res.begin(), res.end());
							}
						}
						else
						{
							SAS_LOG_DEBUG(priv->logger, "nothing to do");
							std::vector<char> header(4 + sizeof(uint32_t));
							char * out_data = header.data();
							memcpy(out_data, std::string("TCL\0").c_str(), 4);
							//size of result is 0(zero)
							output.insert(std::end(output), header.begin(), header.end());
						}
					}
					else
						SAS_LOG_TRACE(priv->logger, "script is ignored");
				}
				else if (format == "BADD")
				{
					std::unique_lock<BlobHandler> __blob_locler(*priv->blobHandler);
					if (data_size < sizeof(uint32_t))
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BADD: missing size information in for blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}

					uint16_t name_size;
					memcpy(&name_size, in_data, sizeof(uint16_t));
					in_data += sizeof(uint16_t); in_size -= sizeof(uint16_t);  data_size -= sizeof(uint16_t);

					if (data_size < name_size)
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BADD: missing blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}

					std::string name;
					name.append(in_data, name_size);
					in_data += name_size; in_size -= name_size; data_size -= name_size;
					SAS_LOG_VAR(priv->logger, name);
					if (!name.length())
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BADD: blob name is not specified");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}
					SAS_LOG_VAR(priv->logger, data_size);
					priv->blobHandler->addBlob(name, (const unsigned char *) in_data, data_size);
					in_data += data_size; in_size -= data_size;
				}
				else if (format == "BREM")
				{
					std::unique_lock<BlobHandler> __blob_locler(*priv->blobHandler);
					if (data_size < sizeof(uint16_t))
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BREM: missing size information in for blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}
					uint16_t name_size;
					memcpy(&name_size, in_data, sizeof(uint16_t));
					in_data += sizeof(uint16_t); in_size -= sizeof(uint16_t); data_size -= sizeof(uint16_t);

					if (data_size < name_size)
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BADD: missing blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}

					std::string name;
					name.append(in_data, name_size);
					in_data += name_size; in_size -= name_size; data_size -= name_size;
					SAS_LOG_VAR(priv->logger, name);
					if (name.length())
						priv->blobHandler->removeBlob(name);
					else
						priv->blobHandler->removeAll();
					in_data += data_size; in_size -= data_size;
				}
				else if (format == "BGET")
				{
					std::unique_lock<BlobHandler> __blob_locler(*priv->blobHandler);
					if (data_size < sizeof(uint16_t))
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BGET: missing size information in for blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}
					uint16_t name_size;
					memcpy(&name_size, in_data, sizeof(uint16_t));
					in_data += sizeof(uint16_t); in_size -= sizeof(uint16_t); data_size -= sizeof(uint16_t);

					if (data_size < name_size)
					{
						auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "BADD: missing blob name");
						SAS_LOG_ERROR(priv->logger, err);
						return TCLInvoker::Status::Error;
					}

					std::string name;
					name.append(in_data, name_size);
					in_data += name_size; in_size -= name_size, data_size -= name_size;
					SAS_LOG_VAR(priv->logger, name);
					std::vector<std::pair<std::string, std::vector<unsigned char>*>> blobs;
					if (name.length())
					{
						blobs.resize(1);
						if (!priv->blobHandler->getBlob(name, blobs[0].second, ec))
							return TCLInvoker::Status::Error;
						blobs[0].first = name;
					}
					else
					{
						if(!priv->blobHandler->getAll(blobs, ec))
							return TCLInvoker::Status::Error;
					}

					for (auto & blob : blobs)
					{
						SAS_LOG_VAR(priv->logger, blob.second->size());

						std::vector<char> header(4 + sizeof(uint32_t) + sizeof(uint16_t));
						char * out_header = header.data();
						memcpy(out_header, "BLOB", 4);
						out_header += 4;
						uint32_t tmp_32(sizeof(uint16_t) + blob.first.size() + blob.second->size());
						memcpy(out_header, &tmp_32, sizeof(uint32_t));
						out_header += sizeof(uint32_t);
						uint16_t tmp_16 = static_cast<uint16_t>(blob.first.size());
						memcpy(out_header, &tmp_16, sizeof(uint16_t));
						out_header += sizeof(uint16_t);
						output.insert(std::end(output), header.begin(), header.end());
						output.insert(std::end(output), blob.first.begin(), blob.first.end());
						output.insert(std::end(output), blob.second->begin(), blob.second->end());
					}
					in_data += data_size; in_size -= data_size;
				}
			}
			if (has_script_error)
				return TCLInvoker::Status::Error;
			break;
		}
		default:
		{
			auto err = ec.add(SAS_TCL__ERROR__INVOKER__INVALID_DATA, "unsupported data version: '" + std::to_string(version) + "'");
			SAS_LOG_ERROR(priv->logger, err);
			return TCLInvoker::Status::NotImplemented;
		}
		}

		return TCLInvoker::Status::OK;
	}

	TCLInvoker::BlobHandler * TCLInvoker::blobHandler() const
	{
		return priv->blobHandler.get();
	}

	//virtual
	TCLInvoker::BlobHandler * TCLInvoker::createBlobHandler() const
	{
		return new Priv::DefaultBlobHandler(priv->name);
	}

}
