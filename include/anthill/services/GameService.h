
#ifndef ONLINE_Game_Service_H
#define ONLINE_Game_Service_H

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include "anthill/Websockets.h"

#include <json/value.h>
#include "DiscoveryService.h"

namespace online
{
	typedef std::shared_ptr< class GameService > GameServicePtr;
    
    typedef std::shared_ptr< class PartySession > PartySessionPtr;
    typedef std::weak_ptr< class PartySession > PartySessionWPtr;

    class AnthillRuntime;
    class GameService;
    
    class PartySession: public std::enable_shared_from_this<PartySession>
    {
    public:
		typedef std::function<void (std::string partyId, int numMembers, int maxMembers, const Json::Value& partySettings, std::set<Json::Value> partyMembers)>
				PartyInfoCallback;

        typedef std::function< void(const Json::Value& callResult) > FunctionSuccessCallback;
        typedef std::function< void(int code, const std::string& reason, const std::string& data) > FunctionFailCallback;
        
        typedef std::function< void(const PartySession& session, Request::Result result) > SessionCreatedCallback;
        typedef std::function< void(int code, const std::string& reason) > SessionClosedCallback;
        
    public:
        PartySession(const std::string& location);
        
    public:
        static PartySessionPtr Create(const std::string& location);
    
        void sendMessage(const Json::Value& payload, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout=0);
        void closeParty(const Json::Value& message, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout=0);
        void leaveParty(FunctionSuccessCallback success, FunctionFailCallback failture, float timeout=0);
        void joinParty(const Json::Value& memberProfile, FunctionSuccessCallback success, FunctionFailCallback failture,
            const Json::Value& checkMembers = Json::Value::nullSingleton(), float timeout=0);
        void startGame(const Json::Value& message, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout=0);
        
		void handlePartyInfo(PartyInfoCallback handler);
        void handlePlayerJoined(JsonRPC::RequestHandler handler);
        void handlePlayerLeft(JsonRPC::RequestHandler handler);
        void handleGameIsAboutToStart(JsonRPC::RequestHandler handler);
        void handleGameStartFailed(JsonRPC::RequestHandler handler);
        void handleGameStarted(JsonRPC::RequestHandler handler);
        void handleCustom(JsonRPC::RequestHandler handler);
        void handlePartyClosed(JsonRPC::RequestHandler handler);
        
        void connect(
            const std::string& accessToken,
            const std::string& partyId,
            SessionCreatedCallback onCreated,
            SessionClosedCallback onClosed,
            bool autoJoin = true,
            const Json::Value& memberProfile = Json::Value::nullSingleton(),
            const Json::Value& checkMembers = Json::Value::nullSingleton());
        
        void createPartyAndConnect(
            const std::string& accessToken,
            const std::string& gameServerName,
            SessionCreatedCallback onCreated,
            SessionClosedCallback onClosed,
            int maxMembers = 0,
            const Json::Value& partySettings = Json::Value::nullSingleton(),
            const Json::Value& roomSettings = Json::Value::nullSingleton(),
            const std::string& region = "",
            bool autoStart = true,
            bool autoClose = true,
            const std::string& closeCallback = "",
            bool autoJoin = true,
            const Json::Value& memberProfile = Json::Value::nullSingleton());
        
        void findPartyAndConnect(
            const std::string& accessToken,
            const std::string& gameServerName,
            SessionCreatedCallback onCreated,
            SessionClosedCallback onClosed,
            const Json::Value& partyFilter = Json::Value::nullSingleton(),
            const Json::Value& memberProfile = Json::Value::nullSingleton(),
            bool autoCreate = true,
            int maxMembers = 0,
            const Json::Value& createPartySettings = Json::Value::nullSingleton(),
            const Json::Value& createRoomSettings = Json::Value::nullSingleton(),
            const std::string& region = "",
            bool createAutoStart = true,
            bool createAutoClose = true,
            const std::string& createCloseCallback = "");
        
        void disconnect(int code, const std::string& reason);

        void update();
        void waitForShutdown();
        
        bool isActive() const;
        
    protected:
        WebsocketRPCPtr m_sockets;
        std::string m_location;
        std::unordered_map<std::string, JsonRPC::RequestHandler> m_messageHandlers;
		PartyInfoCallback m_partyInfoHandler;
    };

	class GameService : public Service
	{
		friend class AnthillRuntime;

	public:
		static const std::string ID;
        static const std::string API_VERSION;
        
        typedef struct JoinResult_
        {
            std::string roomId;
            std::string key;
            std::string host;
            std::vector<int> ports;
            Json::Value settings;
        } JoinResult;

		typedef std::function< void(const GameService&, Request::Result result, const Request& request,
			const JoinResult& joinResult) > JoinRoomCallback;

	public:
		static GameServicePtr Create(const std::string& location);
		virtual ~GameService();

        void joinRoom(const std::string& roomId, const std::string& accessToken, JoinRoomCallback callback);
        
        void joinRoom(const std::string& accessToken, JoinRoomCallback callback, const std::string& gameServer, const Json::Value& settings, const Json::Value& createSettings, bool autoCreate, bool myRegionOnly = true);
        void joinRoom(const std::string& accessToken, JoinRoomCallback callback, const std::string& gameServer, bool autoCreate, bool myRegionOnly = true);

        // Opens a new party session
        // Please note this shared pointer should be stored somewhere, or the session will be terminated
        PartySessionPtr session();
        
	protected:
		GameService(const std::string& location);
		bool init();

	private:
	};
};

#endif
