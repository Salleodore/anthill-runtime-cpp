
#include "anthill/services/GameService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"
#include "anthill/Log.h"

#include <json/writer.h>

namespace online
{
	const std::string GameService::ID = "game";
    const std::string GameService::API_VERSION = "0.2";
    
    PartySession::PartySession(const std::string& location, const PartySession::ListenerPtr& listener) :
        m_sockets(WebsocketRPC::Create()),
        m_location(location),
		m_listener(listener)
    {
        m_messageHandlers["player_joined"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            m_listener->onMemberJoined(PartyMember(payload));
        };
        
        m_messageHandlers["player_left"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            m_listener->onMemberLeft(PartyMember(payload));
        };
        
        m_messageHandlers["game_starting"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            m_listener->onGameStarting(payload);
        };
        
        m_messageHandlers["game_start_failed"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            m_listener->onGameStartFailed(payload["code"].asInt(), payload["reason"].asString());
        };
        
        m_messageHandlers["party_closed"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            m_listener->onPartyClosed(payload);
        };
        
        m_messageHandlers["custom"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            m_listener->onCustomMessage(messageType, payload);
        };
        
        m_messageHandlers["game_started"] = [=](const std::string& messageType, const Json::Value& payload)
        {
            std::string roomId = payload["id"].asString();
            std::string slot = payload["slot"].asString();
            std::string key = payload["key"].asString();

            const Json::Value& location = payload["location"];
            const Json::Value& roomSettings = payload["settings"];

            if (roomId.empty() || key.empty() || location.isNull() || roomSettings.isNull())
            {
                return;
            }

            std::string host = location["host"].asString();
            const Json::Value& portsValue = location["ports"];

            std::list<int> ports;

            for (Json::ValueConstIterator it = portsValue.begin(); it != portsValue.end(); it++)
            {
                ports.push_back(it->asInt());
            }

            m_listener->onGameStarted(roomId, slot, key, host, ports, roomSettings);
        };
    
        m_sockets->handle("message", [=](const Json::Value& params, JsonRPC::Success success, JsonRPC::Failture failture)
        {
            if (!params.isMember("message_type"))
            {
                failture(1400, "No message_type field", "");
                return;
            }
            
            if (!params.isMember("payload"))
            {
                failture(1400, "No payload field", "");
                return;
            }
            
            std::string messageType = params["message_type"].asString();
            const Json::Value& payload = params["payload"];
            
            std::unordered_map<std::string, PartyMessageHandler>::const_iterator it = m_messageHandlers.find(messageType);
            
            if (it == m_messageHandlers.end())
            {
                Log() << "Warning: no handler " << messageType << " is defined for PartySession." << std::endl;
                
                // respond with {} if there was no handler defined
                success(Json::Value(Json::ValueType::objectValue));
                return;
            }
            
            it->second(messageType, payload);
            
            success(Json::Value(Json::ValueType::objectValue));
        });
		
        m_sockets->handle("party", [=](const Json::Value& params, JsonRPC::Success success, JsonRPC::Failture failture)
        {   
			if (!params.isMember("party_info"))
				return;

			const Json::Value& partyInfo = params["party_info"];

			if (!partyInfo.isMember("party"))
				return;

			const Json::Value& partyValue = partyInfo["party"];
   
            Party party(partyValue);

			const Json::Value& partyMembers = partyInfo["members"];
			std::list<PartyMember> partyMembersList;

			for (Json::ValueConstIterator it = partyMembers.begin(); it != partyMembers.end(); it++)
			{
				partyMembersList.emplace_back(*it);
			}
   
            m_listener->onPartyInfoReceived(party, partyMembersList);
            
            success(Json::Value(Json::ValueType::objectValue));
		});
    }

    void PartySession::sendMessage(const Json::Value& payload, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        args["payload"] = payload;

        if (!m_sockets->isConnected())
        {
            failture(599, "Socket is closed", "");
            return;
        }
    
        m_sockets->request("send_message", success, failture, args, timeout);
    }
    
    void PartySession::closeParty(const Json::Value& message, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        args["message"] = message;

        if (!m_sockets->isConnected())
        {
            failture(599, "Socket is closed", "");
            return;
        }
    
        m_sockets->request("close_party", success, failture, args, timeout);
    }
    
    void PartySession::leaveParty(FunctionSuccessCallback success, FunctionFailCallback failture, float timeout)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        if (!m_sockets->isConnected())
        {
            failture(599, "Socket is closed", "");
            return;
        }
    
        m_sockets->request("leave_party", success, failture, args, timeout);
    }
    
    void PartySession::joinParty(const Json::Value& memberProfile, FunctionSuccessCallback success, FunctionFailCallback failture,
            const Json::Value& checkMembers, float timeout)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        args["member_profile"] = memberProfile;
        
        if (!checkMembers.isNull())
        {
            args["check_members"] = checkMembers;
        }

        if (!m_sockets->isConnected())
        {
            failture(599, "Socket is closed", "");
            return;
        }
    
        m_sockets->request("join_party", success, failture, args, timeout);
    }
    
    void PartySession::startGame(const Json::Value& message, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        args["message"] = message;

        if (!m_sockets->isConnected())
        {
            failture(599, "Socket is closed", "");
            return;
        }
    
        m_sockets->request("start_game", success, failture, args, timeout);
    }
    
    bool PartySession::isActive() const
    {
        return m_sockets->isConnected();
    }
    
    void PartySession::connect(
        const std::string& accessToken,
        const std::string& partyId,
        SessionCreatedCallback onCreated,
        SessionClosedCallback onClosed,
        bool autoJoin,
        const Json::Value& memberProfile,
        const Json::Value& checkMembers)
    {
        WebsocketRPC::Options options = {
            {"access_token", accessToken},
            {"auto_join", autoJoin ? "true" : "false"}
        };
        
        if (autoJoin)
        {
            if (!memberProfile.isNull())
            {
                options["member_profile"] = Json::FastWriter().write(memberProfile);
            }
            
            if (!checkMembers.isNull())
            {
                options["check_members"] = Json::FastWriter().write(checkMembers);
            }
        }
        
        std::string path = m_location + "/party/" + partyId + "/session";
        
        std::map<std::string, std::string> extraHeaders = {
            {"X-Api-Version", GameService::API_VERSION}
        };
        
        m_sockets->connect(path, options,
            [=](bool success, int code)
        {
            if (success)
            {
                onCreated(*this, Request::SUCCESS);
            }
            else
            {
                onCreated(*this, (Request::Result)code);
            }
        }, onClosed, extraHeaders);
    }
    
    void PartySession::createPartyAndConnect(
        const std::string& accessToken,
        const std::string& gameServerName,
        SessionCreatedCallback onCreated,
        SessionClosedCallback onClosed,
        int maxMembers,
        const Json::Value& partySettings,
        const Json::Value& roomSettings,
        const std::string& region,
        bool autoStart,
        bool autoClose,
        const std::string& closeCallback,
        bool autoJoin,
        const Json::Value& memberProfile)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        WebsocketRPC::Options options = {
            {"access_token", accessToken},
            {"auto_join", autoJoin ? "true" : "false"},
            {"auto_start", autoStart ? "true" : "false"},
            {"auto_close", autoClose ? "true" : "false"}
        };
        
        if (!partySettings.isNull())
            options["party_settings"] = Json::FastWriter().write(partySettings);
        
        if (!roomSettings.isNull())
            options["room_settings"] = Json::FastWriter().write(roomSettings);
        
        if (autoJoin && !memberProfile.isNull())
            options["member_profile"] = Json::FastWriter().write(memberProfile);
        
        if (maxMembers > 0)
            options["max_members"] = std::to_string(maxMembers);
        
        if (!region.empty())
            options["region"] = region;
        
        if (!closeCallback.empty())
            options["close_callback"] = closeCallback;
        
        std::string path = m_location + "/party/create/" + applicationInfo.applicationName + "/" +
            applicationInfo.applicationVersion + "/" + gameServerName + "/session";
        
        std::map<std::string, std::string> extraHeaders = {
            {"X-Api-Version", GameService::API_VERSION}
        };
        
        m_sockets->connect(path, options,
            [=](bool success, int code)
        {
            if (success)
            {
                onCreated(*this, Request::SUCCESS);
            }
            else
            {
                onCreated(*this, (Request::Result)code);
            }
        }, onClosed, extraHeaders);
    }
    
    void PartySession::findPartyAndConnect(
            const std::string& accessToken,
            const std::string& gameServerName,
            SessionCreatedCallback onCreated,
            SessionClosedCallback onClosed,
            const Json::Value& partyFilter,
            const Json::Value& memberProfile,
            bool autoCreate,
            int maxMembers,
            const Json::Value& createPartySettings,
            const Json::Value& createRoomSettings,
            const std::string& region,
            bool createAutoStart,
            bool createAutoClose,
            const std::string& createCloseCallback)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        WebsocketRPC::Options options = {
            {"access_token", accessToken},
            {"party_filter", Json::FastWriter().write(partyFilter) },
            {"auto_create", autoCreate ? "true" : "false"},
            {"create_auto_start", createAutoStart ? "true" : "false"},
            {"create_auto_close", createAutoClose ? "true" : "false"}
        };
        
        if (autoCreate)
        {
    
            if (!createPartySettings.isNull())
                options["create_party_settings"] = Json::FastWriter().write(createPartySettings);
            
            if (!createRoomSettings.isNull())
                options["create_room_settings"] = Json::FastWriter().write(createRoomSettings);
            
            if (!memberProfile.isNull())
                options["member_profile"] = Json::FastWriter().write(memberProfile);
            
            if (maxMembers > 0)
                options["maxMembers"] = std::to_string(maxMembers);
            
            if (!region.empty())
                options["region"] = region;
            
            if (!createCloseCallback.empty())
                options["create_close_callback"] = createCloseCallback;
        }
        
        std::string path = m_location + "/parties/" + applicationInfo.applicationName + "/" +
            applicationInfo.applicationVersion + "/" + gameServerName + "/session";
        
        std::map<std::string, std::string> extraHeaders = {
            {"X-Api-Version", GameService::API_VERSION}
        };
        
        m_sockets->connect(path, options,
            [=](bool success, int code)
        {
            if (success)
            {
                onCreated(*this, Request::SUCCESS);
            }
            else
            {
                onCreated(*this, (Request::Result)code);
            }
        }, onClosed, extraHeaders);
    }
    
    void PartySession::update()
    {
        m_sockets->update();
    }
    
    void PartySession::waitForShutdown()
    {
        m_sockets->waitForShutdown();
    }

    void PartySession::disconnect(int code, const std::string& reason)
    {
        m_sockets->disconnect(code, reason);
    }
    
    PartySessionPtr PartySession::Create(const std::string& location, const PartySession::ListenerPtr& listener)
    {
        return std::make_shared<PartySession>(location, listener);
    }
    
    //////////////////////

	GameServicePtr GameService::Create(const std::string& location)
	{
		GameServicePtr _object(new GameService(location));
		if (!_object->init())
			return GameServicePtr(nullptr);

		return _object;
	}

	GameService::GameService(const std::string& location) :
		Service(location)
	{
		
    }
    
    void GameService::joinRoom(const std::string& roomId, const std::string& accessToken, GameService::JoinRoomCallback callback)
	{
		const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/room/" + applicationInfo.applicationName + "/" + roomId + "/join", Request::METHOD_POST);
        
		if (request)
        {
            request->setAPIVersion(API_VERSION);
        
            Json::FastWriter fastWriter;
            
            request->setPostFields({
                {"access_token", accessToken }
            });
            
			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful())
				{
					const Json::Value& value = request.getResponseValue();

					if (value.isMember("location") && value.isMember("key") && value.isMember("id"))
					{
                        Json::Value location = value["location"];
                        
                        GameService::JoinResult joinResult;
                        
                        joinResult.roomId = value["id"].asString();
                        joinResult.key = value["key"].asString();
                        joinResult.host = location["host"].asString();
                        
                        for (Json::Value port : location["ports"])
                        {
                            joinResult.ports.push_back(port.asInt());
                        }
                        
                        if (value.isMember("settings"))
                        {
                            joinResult.settings = value["settings"];
                        }
                        
						callback(*this, request.getResult(), request, joinResult);
					}
					else
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, GameService::JoinResult());
					}
				}
				else
				{
					callback(*this, request.getResult(), request, GameService::JoinResult());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
    
    void GameService::joinRoom(const std::string& accessToken, JoinRoomCallback callback, const std::string& gameServer, bool autoCreate, bool myRegionOnly)
    {
        joinRoom(accessToken, callback, gameServer, Json::Value(Json::objectValue), Json::Value(Json::objectValue), autoCreate, myRegionOnly);
    }
    
    void GameService::joinRoom(const std::string& accessToken, GameService::JoinRoomCallback callback, const std::string& gameServer,
                               const Json::Value& settings, const Json::Value& createSettings, bool autoCreate, bool myRegionOnly)
	{
		const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/join/" +
                applicationInfo.applicationName + "/" + gameServer + "/" + applicationInfo.applicationVersion, Request::METHOD_POST);
        
		if (request)
        {
            request->setAPIVersion(API_VERSION);
        
            Json::FastWriter fastWriter;
            
            request->setPostFields({
                {"access_token", accessToken },
                {"settings", fastWriter.write(settings) },
                {"create_settings", fastWriter.write(createSettings) },
                {"auto_create", autoCreate ? "true": "false" },
                {"my_region_only", myRegionOnly ? "true": "false" }
            });
            
			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful())
				{
					const Json::Value& value = request.getResponseValue();

					if (value.isMember("location") && value.isMember("key") && value.isMember("id"))
					{
                        Json::Value location = value["location"];
                        
                        GameService::JoinResult joinResult;
                        
                        joinResult.roomId = value["id"].asString();
                        joinResult.key = value["key"].asString();
                        joinResult.host = location["host"].asString();
                        
                        for (Json::Value port : location["ports"])
                        {
                            joinResult.ports.push_back(port.asInt());
                        }
                        
                        if (value.isMember("settings"))
                        {
                            joinResult.settings = value["settings"];
                        }
                        
						callback(*this, request.getResult(), request, joinResult);
					}
					else
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, GameService::JoinResult());
					}
				}
				else
				{
					callback(*this, request.getResult(), request, GameService::JoinResult());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
    
    PartySessionPtr GameService::session(const PartySession::ListenerPtr& listener)
    {
        return PartySession::Create(getLocation(), listener);
    }

	GameService::~GameService()
	{
		//
	}

	bool GameService::init()
	{
		return true;
	}
}
