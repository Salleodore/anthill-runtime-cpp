
#ifndef ONLINE_Message_Service_H
#define ONLINE_Message_Service_H

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/Websockets.h"
#include <json/value.h>

#include <set>
#include <unordered_map>

namespace online
{
    typedef std::shared_ptr< class MessageService > MessageServicePtr;
	typedef std::shared_ptr< class MessageSession > MessageSessionPtr;
    
    class AnthillRuntime;
    
    class MessageSession: public std::enable_shared_from_this<MessageSession>
    {
    public:
		typedef std::set<std::string> MessageFlags;

		typedef std::function< void(bool success, const std::string& response) > MessageSendCallback;
		typedef std::function< void(bool success, const std::string& reason) > ListenCallback;
		typedef std::function< void(int code, const std::string& reason) > ConnectionClosedCallback;
		typedef std::function< bool(const std::string& uuid, const std::string& sender, const std::string& recipientClass,
                                    const std::string& recipient, const std::string& messageType, const Json::Value& message, 
									const std::string& time, const MessageFlags& flags) > MessageCallback;
        
    public:
        static MessageSessionPtr Create(const std::string location);
        
        MessageSession(const std::string location);
        
        void listen(const std::string& accessToken, ListenCallback callback,
            ConnectionClosedCallback closed, MessageCallback message);
    
        void send(const std::string& recipientClass, const std::string& recipient,
            const std::string& messageType, const Json::Value& message,
            const std::set<std::string>& flags, MessageSendCallback callback);
        
        bool isConnected() const;
        
        void disconnect(int code, const std::string& reason);
        void connect(const std::string& location, const WebsocketRPC::Options& options,
            WebsocketRPC::ConnectCallback onConnect, WebsocketRPC::DisconnectCallback onDisconnect);
        
        void update();
		void waitForShutdown();

    private:
        WebsocketRPCPtr m_sockets;
        std::string m_location;
    };

    
    class MessageService : public Service
    {
        friend class AnthillRuntime;
    public:
        
		typedef std::function< void(const MessageService& service, Request::Result result, const Request& request,
			const std::string& replyToClass, const std::string& replyTo, const std::vector<Json::Value>& messages) > ReadMessagesCallback;
        typedef std::function< void(const MessageService& service, Request::Result result, const Request& request,
            const std::vector<std::string>&)> NewMessagesCallback;
		typedef std::function< void(const MessageService& service, Request::Result result, const Request& request,
            const std::string& replyToClass, const std::string& replyTo) > JoinCallback;
        
    public:
        static const std::string ID;
        static const std::string API_VERSION;
        
    public:
        static MessageServicePtr Create(const std::string& location);
        virtual ~MessageService();
            
        void readGroupInbox(const std::string& groupClass, const std::string& groupKey,
            const std::string& accessToken, ReadMessagesCallback callback, int limit = 100);
            
        void readRecipientMessages(const std::string& recipient,
            const std::string& accessToken, ReadMessagesCallback callback, int limit = 100);

        void readNewMessages(const std::string& accessToken, const std::string& newArg, 
            NewMessagesCallback callback, int limit = 100);
        
        void joinGroup(const std::string& groupClass, const std::string& groupKey,
            const std::string& role, const std::string& accessToken, JoinCallback callback);
            
        // Opens a new session
        // Please note this shared pointer should be stored somewhere, or the session will be terminated
        MessageSessionPtr session();
        
    protected:
        MessageService(const std::string& location);
        bool init();
    };
};

#endif
