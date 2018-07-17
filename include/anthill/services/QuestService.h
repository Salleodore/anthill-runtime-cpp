
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
        typedef time_t QuestTime;
        
        enum class Kind
        {
            account,
            group,
            unknown
        };
        
    public:
        Quest(const Json::Value& data);
        
        const std::string& getId() const { return m_id; }
        Kind getKind() const { return m_kind; }
        
        const Json::Value& getData() const { return m_data; }
        
        const Json::Value& getProfile() const { return m_profile; }
        
    private:
        std::string m_id;
        Kind m_kind;
        Json::Value m_data;
        
        Json::Value m_profile;
	};
 
    struct QuestLeaderboardInfo
    {
        QuestLeaderboardInfo() :
            m_defined(false),
            m_displayName(""),
            m_expireIn(0)
        {}
    
        QuestLeaderboardInfo(const std::string& displayName, uint64_t expireIn) :
            m_defined(true),
            m_displayName(displayName),
            m_expireIn(expireIn)
        {
        
        }
    
        bool m_defined;
        std::string m_displayName;
        uint64_t m_expireIn;
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
            GetQuestsCallback callback,
            int extraStartTime = 0,
            int extraEndTime = 0);

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
