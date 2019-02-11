
#include "anthill/services/EventService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include "json/writer.h"
#include <numeric>

namespace online
{
	const std::string EventService::ID = "event";
    const std::string EventService::API_VERSION = "0.2";

    Event::Event(const Json::Value& data) :
		m_enabled(false),
		m_joined(false),
		m_score(0),
		m_timeStart((EventTime)0),
		m_timeEnd((EventTime)0),
		m_timeLeft(0),
		m_timeLeftNow((EventTime)0),
		m_tournament(false)
    {
        m_id = data["id"].asString();
        m_category = data["category"].asString();

		if( data.isMember( "enabled" ) )
		{
			m_enabled = data["enabled"].asBool();
		}

		if( data.isMember( "joined" ) )
		{
			m_joined = data["joined"].asBool();
		}
        
        std::string kind = data["kind"].asString();
        
        if (kind == "group")
        {
            m_kind = Kind::group;
        }
        else if (kind == "account")
        {
            m_kind = Kind::account;
        }
        else
        {
            m_kind = Kind::unknown;
        }
        
        if (data.isMember("score"))
        {
            m_score = data["score"].asFloat();
        }
    
        if (data.isMember("time"))
        {
            const Json::Value& time = data["time"];
            
            if (time.isMember("start"))
                m_timeStart = online::parse_time(time["start"].asString());
            
            if (time.isMember("end"))
                m_timeEnd = online::parse_time(time["end"].asString());
            
            if (time.isMember("left"))
            {
                m_timeLeft = time["left"].asInt();
                m_timeLeftNow = online::get_utc_timestamp();
            }
        }
        
        if (data.isMember("tournament"))
        {
            m_tournament = true;
            const Json::Value& tournament = data["tournament"];
            
            m_leaderboardName = tournament["leaderboard_name"].asString();
            m_leaderboardOrder = tournament["leaderboard_order"].asString();

			if (tournament.isMember("result"))
			{
				m_tournamentResult = tournament["result"].asInt();
			}
			else
			{
				m_tournamentResult = 0;
			}
        }
		else
		{
			m_tournamentResult = 0;
		}
    
        if (data.isMember("profile"))
        {
            m_profile = data["profile"];
        }
        
        if (data.isMember("group_profile"))
        {
            m_groupProfile = data["group_profile"];
        }
        
        m_data = data;
    }
    
    bool Event::isActive() const
    {
        if (!m_enabled)
            return false;
        
        EventTime now = online::get_utc_timestamp();
        
        if (now < m_timeStart)
            return false;
        
        if (now > m_timeEnd)
            return false;
        
        return true;
    }
    
    int Event::getTimeLeftSeconds() const
    {
        EventTime now = online::get_utc_timestamp();
        int passed = now - m_timeLeftNow;
        int left = m_timeLeft - passed;
        if (left > 0)
            return left;
        
        return 0;
    }

	EventServicePtr EventService::Create(const std::string& location)
	{
		EventServicePtr _object(new EventService(location));
		if (!_object->init())
			return EventServicePtr(nullptr);

		return _object;
	}

	EventService::EventService(const std::string& location) :
		Service(location)
	{
		
    }
    
	void EventService::getEvents(
        const std::string& accessToken,
        GetEventsCallback callback,
        int extraStartTime,
        int extraEndTime)
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/events", Request::METHOD_GET);

		if (request)
		{
			request->setName("events");
            request->setAPIVersion(API_VERSION);
        
			request->setRequestArguments({
                {"access_token", accessToken },
                {"extra_start_time", std::to_string(extraStartTime) },
                {"extra_end_time", std::to_string(extraEndTime) },
            });
        
			request->setOnResponse([=](const online::JsonRequest& request)
			{
                EventService::Events events;
                
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();
     
                    if (value.isMember("events"))
                    {
                        const Json::Value& events_ = value["events"];
                        
                        for (Json::ValueConstIterator it = events_.begin(); it != events_.end(); it++)
                        {
							std::string id = (*it)["id"].asString();
       
                            events[id] = std::make_shared<Event>(*it);
                        }
                    }
                    
					callback(*this, request.getResult(), request, events);
				}
				else
				{
					callback(*this, request.getResult(), request, events);
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
 
 
    void EventService::addScore(
        const std::string& eventId,
        uint64_t score,
        const std::string& accessToken,
        AddEventScoreCallback callback,
        EventLeaderboardInfo leaderboardInfo,
        bool auto_join)
    {
        JsonRequestPtr request = JsonRequest::Create(
            getLocation() + "/event/" + eventId + "/score/add",
            Request::METHOD_POST);

        if (request)
        {
			request->setName("event_score_add");
            request->setAPIVersion(API_VERSION);
        
            request->setPostFields({
                {"access_token", accessToken },
                {"auto_join", auto_join ? "true" : "false" },
                {"score", std::to_string(score) },
            });
            
            if (leaderboardInfo.m_defined)
            {
                Json::Value info(Json::ValueType::objectValue);
                
                info["display_name"] = leaderboardInfo.m_displayName;
                info["expire_in"] = (Json::UInt64)leaderboardInfo.m_expireIn;
                
                request->setPostField("leaderboard_info", Json::FastWriter().write(info));
            }
        
            request->setOnResponse([=](const online::JsonRequest& request)
            {
                if (request.isSuccessful() && request.isResponseValueValid())
                {
                    const Json::Value& value = request.getResponseValue();
                    
                    uint64_t updatedScore = 0;
     
                    if (value.isMember("score"))
                    {
                        updatedScore = value["score"].asUInt64();
                    }
                    
                    callback(*this, request.getResult(), request, updatedScore);
                }
                else
                {
                    callback(*this, request.getResult(), request, 0);
                }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }

        request->start();
    }

	EventService::~EventService()
	{
		//
	}

	bool EventService::init()
	{
		return true;
	}
}
