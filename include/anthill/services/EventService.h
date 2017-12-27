
#ifndef ONLINE_Event_Service_H
#define ONLINE_Event_Service_H

#pragma warning(disable : 4503)

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include <json/value.h>

#include <set>
#include <unordered_map>

namespace online
{
    typedef std::shared_ptr< class EventService > EventServicePtr;
    typedef std::shared_ptr< class Event > EventPtr;

	class AnthillRuntime;
    
	class Event
	{
		friend class EventService;
  
    public:
        typedef time_t EventTime;
        
        enum class Kind
        {
            account,
            group,
            unknown
        };
        
    public:
        Event(const Json::Value& data);
        
        const std::string& getId() const { return m_id; }
        const std::string& getCategory() const { return m_category; }
        Kind getKind() const { return m_kind; }
        bool isEnabled() const { return m_enabled; }
        
        bool isActive() const;
        
        const Json::Value& getData() const { return m_data; }
        
        bool isJoined() const { return m_joined; }
        float getScore() const { return m_score; }
        const Json::Value& getProfile() const { return m_profile; }
        const Json::Value& getGroupProfile() const { return m_groupProfile; }
    
        bool hasTournament() const { return m_tournament; }
        const std::string& getLeaderboardName() const { return m_leaderboardName; }
        const std::string& getLeaderboardOrder() const { return m_leaderboardOrder; }
        
        EventTime getTimeStart() const { return m_timeStart; }
        EventTime getTimeEnd() const { return m_timeEnd; }
        int getTimeLeftSeconds() const;
        
    private:
        std::string m_id;
        std::string m_category;
        Kind m_kind;
        Json::Value m_data;
        bool m_enabled;
        
        bool m_joined;
        float m_score;
        Json::Value m_profile;
        Json::Value m_groupProfile;
        
        EventTime m_timeStart;
        EventTime m_timeEnd;
        int m_timeLeft;
        EventTime m_timeLeftNow;
        
        bool m_tournament;
        std::string m_leaderboardName;
        std::string m_leaderboardOrder;
	};
 
    struct EventLeaderboardInfo
    {
        EventLeaderboardInfo() :
            m_defined(false),
            m_displayName(""),
            m_expireIn(0)
        {}
    
        EventLeaderboardInfo(const std::string& displayName, uint64_t expireIn) :
            m_defined(true),
            m_displayName(displayName),
            m_expireIn(expireIn)
        {
        
        }
    
        bool m_defined;
        std::string m_displayName;
        uint64_t m_expireIn;
    };

	class EventService : public Service
	{
		friend class AnthillRuntime;

	public:
		static const std::string ID;
        static const std::string API_VERSION;
        
        typedef std::unordered_map<std::string, EventPtr> Events;
        
		typedef std::function< void(const EventService& service,
            Request::Result result, const Request& request, Events& events) > GetEventsCallback;
        
        typedef std::function< void(const EventService& service,
            Request::Result result, const Request& request, uint64_t updatedScore) > AddEventScoreCallback;

	public:
		static EventServicePtr Create(const std::string& location);
		virtual ~EventService();

		void getEvents(
			const std::string& accessToken,
            GetEventsCallback callback,
            int extraStartTime = 0,
            int extraEndTime = 0);

        void addScore(
            const std::string& eventId,
            uint64_t score,
            const std::string& accessToken,
            AddEventScoreCallback callback,
            EventLeaderboardInfo leaderboardInfo = EventLeaderboardInfo(),
            bool auto_join = true);
        
	protected:
		EventService(const std::string& location);
        bool init();

	private:
	};
};

#endif
