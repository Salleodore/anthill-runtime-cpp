
#include "anthill/AnthillRuntime.h"
#include "anthill/JsonRPC.h"

#include "json/reader.h"
#include "json/writer.h"

namespace online
{
    JsonRPC::JsonRPC() :
        m_nextId(1)
	{

	}

	JsonRPC::~JsonRPC()
	{
	}
    
    Json::Value JsonRPC::serializeError(int code, const std::string& message, const std::string& data)
    {
        Json::Value error;
        
        error["code"] = code;
        error["message"] = message;
        
        if (!data.empty())
        {
            error["data"] = data;
        }
        
        return error;
    }
    
    void JsonRPC::writeError(int code, const std::string& message, const std::string& data, int id)
    {
        Json::Value toWrite;
        
        toWrite["jsonrpc"] = "2.0";
        toWrite["error"] = serializeError(code, message, data);
        
        if (id >= 0)
        {
            toWrite["id"] = id;
        }
        
        Json::FastWriter writer;
        write(writer.write(toWrite));
    }
    
    void JsonRPC::writeResponse(const Json::Value result, int id)
    {
        Json::Value toWrite;
        
        toWrite["jsonrpc"] = "2.0";
        toWrite["result"] = result;
        
        if (id >= 0)
        {
            toWrite["id"] = id;
        }
        
        Json::FastWriter writer;
        write(writer.write(toWrite));
    }
    
    void JsonRPC::update()
    {
        std::string data;
        while (read(data))
        {
            received(data);
        }
    }
    
    void JsonRPC::received(const std::string& message)
    {
        Json::Value msg;
        
        if (!Json::Reader().parse(message, msg))
        {
            writeError(-32700, "Parse error");
            return;
        }
        
        if (!msg.isMember("jsonrpc"))
        {
            writeError(-32600, "Invalid Request", "No 'jsonrpc' field.");
            return;
        }
        
        if (msg["jsonrpc"].asString() != "2.0")
        {
            writeError(-32600, "Bad version of 'jsonrpc': " + msg["jsonrpc"].asString() + ".");
            return;
        }
        
        bool hasId = msg.isMember("id") && msg["id"].asInt() > 0;
        bool hasMethod = msg.isMember("method") && !msg["method"].asString().empty();
        bool hasParams = msg.isMember("params");
        bool hasResult = msg.isMember("result");
        bool hasError = msg.isMember("error");
        
        const Json::Value& params = hasParams ? msg["params"] : Json::Value::null;
        std::string method = hasMethod ? msg["method"].asString() : "";
        int id = hasId ? msg["id"].asInt() : 0;
        
        const Json::Value& _error = hasError ? msg["error"] : Json::Value::null;
        const Json::Value& result = hasResult ? msg["result"] : Json::Value::null;
        
        if (hasId && hasMethod)
        {
            RequestHandlers::iterator it = m_handlers.find(method);
            
            // a request
            if (it != m_handlers.end())
            {
                RequestHandler handler = it->second;
            
                bool called = false;
                
                handler(params, [&](const Json::Value& response)
                {
                    if (called) return; called = true;
                    
                    if (response != Json::Value::null)
                    {
                        writeResponse(response, id);
                    }
                    else
                    {
                        writeError(-32603, "Internal error", "Response cannot be null", id);
                    }
                }, [&](int code, const std::string& message, const std::string& data)
                {
                    if (called) return; called = true;
                    writeError(code, message, data, id);
                });
            }
            else
            {
                writeError(-32601, "Method not found");
            }
        }
        else if (hasId)
        {
            if (hasError == hasResult)
            {
                writeError(-32600, "Invalid Request", "Should be (only) one 'result' or 'error' field.");
                return;
            }
            
            // a success
            
            ResponseHandlers::iterator it = m_responseHandlers.find(id);
            
            if (it != m_responseHandlers.end())
            {
                ResponseHandler handler = it->second;
                m_responseHandlers.erase(it);
                
                if (handler.m_future)
                {
                    AnthillRuntime::Instance().getFutures().cancel(handler.m_future);
                }
                
                if (hasResult)
                {
                    handler.m_success(result);
                }
                else
                {
                    if (_error.isMember("code") &&
                        _error.isMember("message"))
                    {
                        int responseCode = _error["code"].asInt();
                        std::string responseMessage = _error["message"].asString();
                        std::string responseData = _error.isMember("data") ? _error["data"].asString() : "";
                        
                        // hasError
                        if (handler.m_failture)
                        {
                            handler.m_failture(responseCode, responseMessage, responseData);
                        }
                    }
                    else
                    {
                        writeError(-32600, "Invalid Request", "Bad 'error' field.");
                    }
                }
            }
            else
            {
                writeError(-32600, "Invalid Request", "No such handler.", id);
            }
        }
        else if (hasMethod)
        {
            RequestHandlers::iterator it = m_handlers.find(method);
            
            // an rpc
            if (it != m_handlers.end())
            {
                RequestHandler handler = it->second;
                
                handler(params, [=](const Json::Value& response)
                {
                    // ignore
                }, [=](int code, const std::string& message, const std::string& data)
                {
                    // ignore
                });
            }
        }
        else if (hasError)
        {
            if (_error.isMember("code") &&
                _error.isMember("message"))
            {
                int code = _error["code"].asInt();
                std::string errorMessage = _error["message"].asString();
                std::string data = _error.isMember("data") ? _error["data"].asString() : "";
                
                // hasError
                error(code, errorMessage, data);
            }
            else
            {
                writeError(-32600, "Invalid Request", "Bad 'error' field.");
            }
        }
        else
        {
            writeError(-32600, "Invalid Request", "No 'method' nor 'id' field.");
        }
    }
    
    void JsonRPC::handle(const std::string& method, RequestHandler handler)
    {
        m_handlers[method] = handler;
    }
    
    void JsonRPC::request(const std::string& method, Success success, Failture failture, const Json::Value& params, float timeout)
    {
        Json::Value toWrite;
        
        toWrite["jsonrpc"] = "2.0";
        toWrite["method"] = method;
        
        int currentId = m_nextId;
        
        toWrite["id"] = currentId;
        toWrite["params"] = params;
        
        int future = 0;
        
        if (timeout)
        {
            future = AnthillRuntime::Instance().getFutures().add(timeout, [this, currentId, timeout]()
            {
                ResponseHandlers::iterator it = m_responseHandlers.find(currentId);
                
                if (it == m_responseHandlers.end())
                    return;
                
                it->second.m_failture(599, "Request Timeout", std::to_string( timeout ));
                m_responseHandlers.erase(it);
            });
        }
        
        m_responseHandlers.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(currentId),
            std::forward_as_tuple(success, failture, future));
    
        m_nextId++;
        
        Json::FastWriter writer;
        write(writer.write(toWrite));
    }
    
    void JsonRPC::rpc(const std::string& method, const Json::Value& params)
    {
        Json::Value toWrite;
        
        toWrite["jsonrpc"] = "2.0";
        toWrite["method"] = method;
        toWrite["params"] = params;
        
        Json::FastWriter writer;
        write(writer.write(toWrite));
    }
    
    void JsonRPC::rejectAllResponseHandlers(int code, const std::string& message, const std::string& data)
    {
        Futures& futures = AnthillRuntime::Instance().getFutures();
        
        for (ResponseHandlers::const_iterator it = m_responseHandlers.begin(); it != m_responseHandlers.end(); it++)
        {
            if (it->second.m_future)
            {
                futures.cancel(it->second.m_future);
            }
        
            it->second.m_failture(code, message, data);
        }
    
        m_responseHandlers.clear();
    }
}
