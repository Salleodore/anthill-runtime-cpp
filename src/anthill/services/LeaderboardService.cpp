
#include "anthill/services/LeaderboardService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <json/writer.h>
#include <string>

namespace online
{
    const std::string LeaderboardService::ID = "leaderboard";
    const std::string LeaderboardService::API_VERSION = "0.2";
    
    //////////////////
    
    LeaderboardEntry::LeaderboardEntry(const Json::Value& data)
    {
        m_displayName = data["display_name"].asString();
        m_score = data["score"].asFloat();
        m_rank = data["rank"].asInt();
        m_account = data["account"].asString();
        m_profile = data["profile"];
    }
    
    //////////////////
    
    LeaderboardServicePtr LeaderboardService::Create(const std::string& location)
    {
        LeaderboardServicePtr _object(new LeaderboardService(location));
        if (!_object->init())
            return LeaderboardServicePtr(nullptr);
        
        return _object;
    }
    
    LeaderboardService::LeaderboardService(const std::string& location) :
        Service(location)
    {
        
	}

	
    void LeaderboardService::getLeaderboardEntries(const std::string& name, const std::string& order,
        const std::string& accessToken, GetLeaderboardEntriesCallback callback, int limit)
    {
		if( name.empty() )
		{
			Log::get() << "Error! getLeaderboardEntries : name is empty!";
			return;
		}
			
		if( order.empty() )
		{
			Log::get() << "Error! getLeaderboardEntries : order is empty!";
			return;
		}

        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/leaderboard/" + order + "/" + name,
            Request::METHOD_GET);
        
        if (request)
        {
			request->setName("leaderboard_entries");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                { "access_token", accessToken },
                { "limit", std::to_string( limit ) }
            });
            
            request->setOnResponse([callback, this](const online::JsonRequest& request)
            {
                if (request.isSuccessful() && request.isResponseValueValid())
                {
					LeaderboardEntries entries;

				    const Json::Value& value = request.getResponseValue();
                   
                    if (value.isMember("data"))
                    {
                        const Json::Value& data = value["data"];
                        
                        for (Json::ValueConstIterator it = data.begin(); it != data.end(); it++)
                        {
                            const Json::Value& entry = *it;
							entries.push_back(std::make_shared<LeaderboardEntry>(entry));
                        }
                    }
                   
                    callback(*this, request.getResult(), request, entries);
                }
                else
                {
				    callback(*this, request.getResult(), request, LeaderboardEntries());
                }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    void LeaderboardService::deleteLeaderboardEntry(const std::string& name, const std::string& order,
        const std::string& accessToken, DeleteLeaderboardEntryCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/leaderboard/" + order + "/" + name + "/entry",
            Request::METHOD_DELETE);
        
        if (request)
        {
			request->setName("leaderboard_delete_entry");
            request->setAPIVersion(API_VERSION);
        
            request->setOnResponse([callback, this](const online::JsonRequest& request)
            {
                callback(*this, request.getResult(), request);
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->setRequestArguments({
            {"access_token", accessToken }
        });
        
        request->start();
    }
    
    void LeaderboardService::addLeaderboardEntry(const std::string& name, const std::string& order, float score,
        const std::string& displayName, int expireIn,
        const std::string& accessToken, PostLeaderboardEntryCallback callback)
    {
        addLeaderboardEntry(name, order, score, displayName, expireIn, Json::Value::nullSingleton(), accessToken, callback);
    }
    
    void LeaderboardService::addLeaderboardEntry(const std::string& name, const std::string& order, float score,
        const std::string& displayName, int expireIn, const Json::Value& profile,
        const std::string& accessToken, PostLeaderboardEntryCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/leaderboard/" + order + "/" + name,
            Request::METHOD_POST);
        
        if (request)
        {
			request->setName("leaderboard_add_entry");
            request->setAPIVersion(API_VERSION);
        
            request->setOnResponse([callback, this](const online::JsonRequest& request)
            {
                callback(*this, request.getResult(), request);
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        Request::Fields postFields = {
            {"score", std::to_string(score) },
            {"display_name", displayName },
            {"expire_in", std::to_string(expireIn) },
            {"access_token", accessToken }
        };
        
        if (!profile.isNull())
        {
            Json::FastWriter writer;
            postFields["profile"] = writer.write(profile);
        }
        
        request->setPostFields(postFields);
        request->start();
    }
    
    LeaderboardService::~LeaderboardService()
    {
        //
    }
    
    bool LeaderboardService::init()
    {
        return true;
    }
}
