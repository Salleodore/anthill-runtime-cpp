
#ifndef ONLINE_Quest_Service_H
#define ONLINE_Quest_Service_H

#pragma warning(disable : 4503)

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include <json/value.h>

#include <set>
#include <unordered_map>

namespace online
{
    typedef std::shared_ptr< class QuestService > QuestServicePtr;
    typedef std::shared_ptr< class Quest > QuestPtr;

	class AnthillRuntime;
    
	class Quest
	{
		friend class QuestService;
  
    public:
        Quest(const Json::Value& data);
        
        const std::string& getId() const { return m_id; }
        const Json::Value& getData() const { return m_data; }
        const Json::Value& getProfile() const { return m_profile; }
        
    private:
        std::string m_id;
        Json::Value m_data;
        Json::Value m_profile;
	};
 
	class QuestService : public Service
	{
		friend class AnthillRuntime;

	public:
		static const std::string ID;
        static const std::string API_VERSION;
        
        typedef std::unordered_map<std::string, QuestPtr> Quests;
        
		typedef std::function< void(const QuestService& service,
            Request::Result result, const Request& request, Quests& quests) > GetQuestsCallback;
        
        typedef std::function< void(const QuestService& service,
            Request::Result result, const Request& request) > UpdateQuestPayloadCallback;

	public:
		static QuestServicePtr Create(const std::string& location);
		virtual ~QuestService();

		void getQuests(
			const std::string& accessToken,
            GetQuestsCallback callback );

        void updatePayload(
            const std::string& questId,
			const Json::Value& payload,
            const std::string& accessToken,
            UpdateQuestPayloadCallback callback );
        
	protected:
		QuestService(const std::string& location);
        bool init();

	private:
	};
};

#endif
