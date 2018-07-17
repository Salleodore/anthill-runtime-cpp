
#include "anthill/services/QuestService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include "json/writer.h"
#include <numeric>

namespace online
{
	const std::string QuestService::ID = "quest";
    const std::string QuestService::API_VERSION = "0.2";

    Quest::Quest(const Json::Value& data)
    {
        m_id = data["id"].asString();
        
        std::string kind = data["kind"].asString();
        
        if (data.isMember("profile"))
        {
            m_profile = data["profile"];
        }
        
        m_data = data;
    }
    
	QuestServicePtr QuestService::Create(const std::string& location)
	{
		QuestServicePtr _object(new QuestService(location));
		if (!_object->init())
			return QuestServicePtr(nullptr);

		return _object;
	}

	QuestService::QuestService(const std::string& location) :
		Service(location)
	{
		
    }
    
	void QuestService::getQuests(
        const std::string& accessToken,
        GetQuestsCallback callback )
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/quests", Request::METHOD_GET);

		if (request)
		{
            request->setAPIVersion(API_VERSION);
        
			request->setRequestArguments({
                {"access_token", accessToken }
            });
        
			request->setOnResponse([=](const online::JsonRequest& request)
			{
                QuestService::Quests quests;
                
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();
     
                    if (value.isMember("quests"))
                    {
                        const Json::Value& quests_ = value["quests"];
                        
                        for (Json::ValueConstIterator it = quests_.begin(); it != quests_.end(); ++it)
                        {
							std::string id = (*it)["id"].asString();
                            quests[id] = std::make_shared<Quest>(*it);
                        }
                    }
                    
					callback(*this, request.getResult(), request, quests);
				}
				else
				{
					callback(*this, request.getResult(), request, quests);
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
 
    void QuestService::updatePayload(
        const std::string& questId,
        const Json::Value& payload,
        const std::string& accessToken,
        UpdateQuestPayloadCallback callback )
    {
        JsonRequestPtr request = JsonRequest::Create(
            getLocation() + "/quest/" + questId + "/updatePayload",
            Request::METHOD_POST);

        if (request)
        {
            request->setAPIVersion(API_VERSION);
        
            request->setPostFields({
                {"access_token", accessToken }
            });

            request->setPostField("payload", Json::FastWriter().write(payload));
            
            request->setOnResponse([=](const online::JsonRequest& request)
            {
                if (request.isSuccessful() && request.isResponseValueValid())
                {
                    callback(*this, request.getResult(), request);
                }
                else
                {
                    callback(*this, request.getResult(), request);
                }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }

        request->start();
    }

	QuestService::~QuestService()
	{
		//
	}

	bool QuestService::init()
	{
		return true;
	}
}
