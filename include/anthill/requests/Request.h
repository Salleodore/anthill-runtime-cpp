
#ifndef ONLINE_Requests_H
#define ONLINE_Requests_H

#define CURL_STATICLIB (1)
#include "anthill/Log.h"

#include "curl_easy.h"
#include "curl_header.h"

#include <functional>
#include <unordered_map>
#include <istream>

namespace online
{
	typedef std::shared_ptr< class Request > RequestPtr;

	class AnthillRuntime;

	class Request : public std::enable_shared_from_this<Request>
	{
		friend class AnthillRuntime;

	public:
		typedef std::unordered_map<std::string, std::string> Fields;
		typedef std::function< void(const Request&) > ResponseCallback;

		typedef enum Status
		{
			NONE = -1,
			STARTED = 0,
			COMPLETED = 1
		} Status_;

		typedef enum Result
		{
			NOT_INITIALIZED = -1,
			CONNECTION_ERROR = 0,

			SUCCESS = 200,
			CONFLICT = 409,
			MULTIPLE_CHOISES = 300,
			BAD_ARGUMENTS = 400,
			FORBIDDEN = 403,
			NOT_FOUND = 404,
            NOT_ACCEPTABLE = 406,
			LOCKED = 423,
			TOO_MANY_REQUESTS = 429,
			INTERNAL_ERROR = 500,

			MISSING_RESPONSE_FIELDS = 597,
			CLIENT_ERROR = 598
		} Result_;

		typedef enum Method
		{
			METHOD_GET = 0,
			METHOD_POST = 1,
            METHOD_DELETE = 2,
            METHOD_PUT = 3
		} Method_;

	public:
		virtual ~Request();

		curl::curl_easy& getTransport();
		Status getStatus() const;
		const std::string& getResponseContentType() const;

		void setOnResponse(ResponseCallback onResponse);

		void setResult(Result result);
		Result getResult() const;

        void setPostField(const std::string& key, const std::string& value);
		void setPostFields(const Fields& fields);
		const Fields& getPostFields() const;
  
        void setRequestBody(const std::string& contents);
        void setRequestBody(std::istream& contents);

		void setRequestArguments(const Fields& fields);
		const Fields& getRequestArguments() const;
  
        void setAPIVersion(const std::string& APIVersion);
        void setFollowRedirects(bool followRedirects);

		void addResponseHeader(const std::string& key, const std::string& value);
		std::string getResponseHeader(const std::string& key) const;
		const Fields& getResponseHeaders() const { return m_responseHeaders; }

		bool isSuccessful() const;
		static bool isSuccessful(Result result);
        bool isCancelled() const { return m_cancelled; }

		void start();
        void cancel();
        
        virtual std::string getResponseAsString() const = 0;

	protected:
		template<class T>
        Request(const std::string& location, Method method, curl::curl_ios<T> ios) :
            m_location(location),
            m_result(NOT_INITIALIZED),
            m_method(method),
            m_status(NONE),
            m_transport(ios),
            m_cancelled(false)
        {
        }
        
		virtual bool init();

		// called once the request is done
		virtual void done();
        
        // failed to connect to the server
        virtual void connectionError() = 0;

	private:
		std::string m_location;
		std::string m_responseContentType;
        std::string m_APIVersion;
		
		Fields m_responseHeaders;
		Fields m_arguments;
		Fields m_postFields;
		Result m_result;
		Method m_method;
		Status m_status;
        std::string m_postFieldsData;
        
		curl::curl_easy m_transport;
        curl::curl_header m_headers;
		ResponseCallback m_onResponse;
        bool m_cancelled;
        bool m_followRedirects;
	};
    
    typedef std::shared_ptr< class StringStreamRequest > StringStreamRequestPtr;

    class StringStreamRequest: public Request
    {
    public:
        virtual std::string getResponseAsString() const override
        {
            return m_response.str();
        }
        
        const std::stringstream& getResponse() const
        {
            return m_response;
        }
        
    public:
        static StringStreamRequestPtr Create(const std::string& location, Request::Method method);
        
    protected:
		StringStreamRequest(const std::string& location, Method method) :
            Request(location, method, curl::curl_ios<std::stringstream>(m_response))
        {}
        
        virtual void connectionError() override
        {
            m_response.clear();
            
            Log::get() << "<Connection Error>" << std::endl;
            m_response << "<Connection Error>";
        }
        
    private:
		std::stringstream m_response;
    };
    
    class FileStreamRequest: public Request
    {
    public:
        virtual std::string getResponseAsString() const override
        {
            return "";
        }
        
        std::fstream& getResponse() const
        {
            return m_response;
        }
        
    protected:
		FileStreamRequest(const std::string& location, Method method, std::fstream& file) :
            Request(location, method, curl::curl_ios<std::fstream>(file)),
            m_response(file)
        {
        }
        
        virtual void connectionError() override
        {
            Log::get() << "<Connection Error>" << std::endl;
        }
        
    private:
		std::fstream& m_response;
    };
};

#endif
