
#include "anthill/AnthillRuntime.h"

#include "anthill/requests/Request.h"
#include "anthill/Utils.h"
#include "anthill/Log.h"

#include "curl_ios.h"

namespace online
{
    StringStreamRequestPtr StringStreamRequest::Create(const std::string& location, Request::Method method)
    {
        StringStreamRequestPtr _object(new StringStreamRequest(location, method));
        if( !_object->init() )
            return StringStreamRequestPtr(nullptr);

        return _object;
    }
    
    void Request::cancel()
    {
        if (m_cancelled)
            return;
        
        m_cancelled = true;
    }
	
	void Request::addResponseHeader(const std::string& key, const std::string& value)
	{
		m_responseHeaders[key] = value;
	}
	
	std::string Request::getResponseHeader(const std::string& key) const
	{
		std::string key_ = key;
		std::transform(key_.begin(), key_.end(), key_.begin(), ::tolower);
		Fields::const_iterator it = m_responseHeaders.find(key_);

		if (it == m_responseHeaders.end())
		{
			return "";
		}

		return it->second;
	}

	size_t curl_header_function(void *buffer, size_t size, size_t nitems, void *userdata)
	{
		size_t numbytes = size * nitems;
		Request* request = (Request*)userdata;

		std::string headerLine((char*)buffer, numbytes);

		size_t index = headerLine.find(':') ;
		if (index != std::string::npos)
		{
			std::string key = headerLine.substr(0, index);
			std::string value = headerLine.substr(index + 1);
			
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			value.erase(value.find_last_not_of(" \n\r\t")+1);

			request->addResponseHeader(key, value);
		}

		return numbytes;
	}
    
	void Request::start()
	{
		OnlineAssert(m_status == NONE, "Request is already started.");

		if (!m_arguments.empty())
		{
			std::set<std::string> pairs;

			for (Request::Fields::const_iterator it = m_arguments.begin(); it != m_arguments.end(); it++)
			{
				std::string value = it->second;
				m_transport.escape(value);
				pairs.insert(it->first + "=" + value);
			}

			m_location += "?" + join(pairs, "&");
		}
  
        if (!m_APIVersion.empty())
        {
            m_headers.add("X-API-Version: " + m_APIVersion);
        }
        
        m_transport.add<CURLOPT_HTTPHEADER>(m_headers.get());
		m_transport.add<CURLOPT_HEADERFUNCTION>(&curl_header_function);
		m_transport.add<CURLOPT_HEADERDATA>(this);

		m_transport.add<CURLOPT_URL>(m_location.c_str());
        m_transport.add<CURLOPT_FOLLOWLOCATION>(m_followRedirects ? 1L : 0L);

		switch (m_method)
        {
            case Request::METHOD_GET:
            {
                break;
            }
			case Request::METHOD_POST:
			{
				m_transport.add<CURLOPT_POST>(1L);

				if (!m_postFields.empty())
				{
                    bool second = false;
                    std::stringstream data;
                    
					for (Request::Fields::const_iterator it = m_postFields.begin(); it != m_postFields.end(); it++)
                    {
                        if (second)
                        {
                            data << "&";
                        }
                        else
                        {
                            second = true;
                        }
                        
                        data << url_encode(it->first) << "=" << url_encode(it->second);
					}
                    
                    m_postFieldsData = data.str();

					m_transport.add<CURLOPT_POSTFIELDS>(m_postFieldsData.c_str());
				}

				break;
			}
			case Request::METHOD_DELETE:
			{
				m_transport.add<CURLOPT_CUSTOMREQUEST>("DELETE");

				if (!m_postFields.empty())
				{
                    bool second = false;
                    std::stringstream data;
                    
					for (Request::Fields::const_iterator it = m_postFields.begin(); it != m_postFields.end(); it++)
                    {
                        if (second)
                        {
                            data << "&";
                        }
                        else
                        {
                            second = true;
                        }
                        
                        data << url_encode(it->first) << "=" << url_encode(it->second);
					}
                    
                    m_postFieldsData = data.str();

					m_transport.add<CURLOPT_POSTFIELDS>(m_postFieldsData.c_str());
				}

				break;
			}
            case Request::METHOD_PUT:
            {
                m_transport.add<CURLOPT_CUSTOMREQUEST>("PUT");
                m_transport.add<CURLOPT_POSTFIELDS>(m_postFieldsData.c_str());

                break;
            }
		}

		AnthillRuntime::Instance().addRequest(shared_from_this());

		m_status = STARTED;
	}
 
    void Request::setAPIVersion(const std::string& APIVersion)
    {
        m_APIVersion = APIVersion;
    }
 
    void Request::setFollowRedirects(bool followRedirects)
    {
        m_followRedirects = followRedirects;
    }

	void Request::done()
	{
		//long headersSize = m_transport.get_info<CURLINFO_HEADER_SIZE>().get();

		m_result = (Request::Result)m_transport.get_info<CURLINFO_RESPONSE_CODE>().get();
        
		if (m_result != CONNECTION_ERROR)
		{
			m_responseContentType = m_transport.get_info<CURLINFO_CONTENT_TYPE>().get();
		}
        else
        {
            connectionError();
        }

		if (m_onResponse)
		{
			m_onResponse(*this);
		}

		m_status = COMPLETED;
	}
	
	void Request::setOnResponse(Request::ResponseCallback onResponse)
	{
		m_onResponse = onResponse;
	}

	bool Request::isSuccessful(Result result)
	{
		int responseCode = (int)result;

		return responseCode >= 200 && responseCode < 400 && responseCode != 300;
	}

	bool Request::isSuccessful() const
	{
		return isSuccessful(m_result);
	}

	curl::curl_easy& Request::getTransport()
	{
		return m_transport;
	}

	const std::string& Request::getResponseContentType() const
	{
		return m_responseContentType;
	}

	void Request::setResult(Request::Result result)
	{
		m_result = result;
	}

	Request::Result Request::getResult() const
	{
		return m_result;
	}

	Request::Status Request::getStatus() const
	{
		return m_status;
	}
 
    void Request::setPostField(const std::string& key, const std::string& value)
    {
        m_postFields[key] = value;
    }

	const Request::Fields& Request::getPostFields() const
	{
		return m_postFields;
	}

	void Request::setPostFields(const Request::Fields& fields)
	{
		m_postFields = fields;
	}
 
    void Request::setRequestBody(const std::string& contents)
    {
        m_postFieldsData = contents;
    }
    
    void Request::setRequestBody(std::istream& contents)
    {
        char buffer[4096];
        
        m_postFieldsData.reserve(contents.gcount());
        
        while (contents.read(buffer, sizeof(buffer)))
            m_postFieldsData.append(buffer, sizeof(buffer));
        
        m_postFieldsData.append(buffer, contents.gcount());
    }

	const Request::Fields& Request::getRequestArguments() const
	{
		return m_arguments;
	}

	void Request::setRequestArguments(const Request::Fields& fields)
	{
		m_arguments = fields;
	}

	Request::~Request()
	{
		//
	}

	bool Request::init()
	{
		return true;
	}
}
