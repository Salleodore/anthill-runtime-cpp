
#ifndef ONLINE_Exec_Service_H
#define ONLINE_Exec_Service_H

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include "anthill/Websockets.h"

#include <json/value.h>
#include "DiscoveryService.h"

#include <string>
#include <list>
#include <functional>

#include <uWS.h>

namespace online
{
	typedef std::shared_ptr< class ExecService > ExecServicePtr;
    
	typedef std::shared_ptr< class ExecSession > ExecSessionPtr;
    typedef std::shared_ptr< class ExecDebugSession > ExecDebugSessionPtr;
    
    typedef std::weak_ptr< class ExecSession > ExecSessionWPtr;
    typedef std::weak_ptr< class ExecDebugSession > ExecDebugSessionWPtr;

	class AnthillRuntime;
	class ExecService;
    
    class ExecSession: public std::enable_shared_from_this<ExecSession>
    {
    public:
		typedef std::function< void(const Json::Value& callResult) > FunctionSuccessCallback;
		typedef std::function< void(int code, const std::string& reason, const std::string& data) > FunctionFailCallback;
        
		typedef std::function< void(const ExecSession& session, Request::Result result) > SessionCreatedCallback;
		typedef std::function< void(int code, const std::string& reason) > SessionClosedCallback;
        
    public:
        ExecSession(const std::string& location, const std::string& mode = "session");
        
    public:
        static ExecSessionPtr Create(const std::string& location);
    
        void callMain(const Json::Value& arguments, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout=0);
        void call(const std::string& method, const Json::Value& arguments, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout=0);
        
        void disconnect(int code, const std::string& reason);
        void connect(const std::string& accessToken, const std::string& className, const Json::Value& args,
                     SessionCreatedCallback onCreated, SessionClosedCallback onClosed);

        void update();
		void waitForShutdown();
        
        bool isActive() const;
        
    protected:
        WebsocketRPCPtr m_sockets;
        std::string m_location;
        std::string m_mode;
    };
    
    class ExecDebugSession: public ExecSession
    {
    public:
        typedef std::function< void(bool success) > DebugStartCallback;
        typedef std::function< void(const std::string& message) > LogCallback;
        
    public:
        static ExecDebugSessionPtr Create(const std::string& location, LogCallback logCallback, const std::string& scriptsDirectory);
        ExecDebugSession(const std::string& location, LogCallback logCallback, const std::string& scriptsDirectory);
        
    public:
        void startDebugging(DebugStartCallback callback);
        void eval(const std::string& text, std::function<void(bool success, const std::string& result)> callback);
        
    private:
        void uploadFiles(std::function<void(bool success)> uploadCallback);
    
    private:
        std::string m_scriptsDirectory;
        std::list<std::string> m_filesToUpload;
    };

	class ExecService : public Service
	{
		friend class AnthillRuntime;

	public:
		static const std::string ID;
        static const std::string API_VERSION;
         
		typedef std::function< void(const ExecService&, Request::Result result, const Request& request,
			const Json::Value& callResult) > FunctionCallCallback;

	public:
		static ExecServicePtr Create(const std::string& location);
		virtual ~ExecService();

        void callMain(const std::string& accessToken, const Json::Value& arguments, FunctionCallCallback callback);
        void call(const std::string& accessToken, const std::string& method, const Json::Value& arguments, FunctionCallCallback callback);
        
        // Opens a new session instead of just calling a method
        // Please note this shared pointer should be stored somewhere, or the session will be terminated
        ExecSessionPtr session();
        
        // Opens a new debug session
        // logCallback will be called for each log() call on the js side
        // scriptsDirectory is a directory fist js scripts to upload for debugging
        // Please note this shared pointer should be stored somewhere, or the session will be terminated
        ExecDebugSessionPtr debugSession(ExecDebugSession::LogCallback logCallback, const std::string& scriptsDirectory);

	protected:
		ExecService(const std::string& location);
		bool init();

	private:
	};
};

#endif
