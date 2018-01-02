
#ifndef ONLINE_Leaderboard_Service_H
#define ONLINE_Leaderboard_Service_H

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include <json/value.h>

#include <set>
#include <unordered_map>
#include <list>

namespace online
{
    typedef std::shared_ptr< class LeaderboardService > LeaderboardServicePtr;
    
    class AnthillRuntime;
	
    typedef std::shared_ptr< class LeaderboardEntry > LeaderboardEntryPtr;

	class LeaderboardEntry
	{
		friend class LeaderboardService;
        
    public:
        LeaderboardEntry(const Json::Value& data);
    
        int getRank() const { return m_rank; }
        float getScore() const { return m_score; }
        const std::string& getDisplayName() const { return m_displayName; }
        const std::string& getAccount() const { return m_account; }
        const Json::Value& getProfile() const { return m_profile; }
        
    private:
        int m_rank;
        float m_score;
        std::string m_displayName;
        std::string m_account;
        Json::Value m_profile;
	};
    
    typedef std::list<LeaderboardEntryPtr> LeaderboardEntries;
    
    class LeaderboardService : public Service
    {
        friend class AnthillRuntime;
	public:
		typedef std::function< void(const LeaderboardService& service, Request::Result result,
            const Request& request) > PostLeaderboardEntryCallback;
            
		typedef std::function< void(const LeaderboardService& service, Request::Result result,
            const Request& request) > DeleteLeaderboardEntryCallback;
        
		typedef std::function< void(const LeaderboardService& service, Request::Result result,
            const Request& request, const LeaderboardEntries& entries) > GetLeaderboardEntriesCallback;
        
    public:
        static const std::string ID;
        static const std::string API_VERSION;
        
    public:
        static LeaderboardServicePtr Create(const std::string& location);
        virtual ~LeaderboardService();
        
		void getLeaderboardEntries(const std::string& name, const std::string& order,
			const std::string& accessToken, GetLeaderboardEntriesCallback callback);
        
		void deleteLeaderboardEntry(const std::string& name, const std::string& order,
			const std::string& accessToken, DeleteLeaderboardEntryCallback callback);
        
		void addLeaderboardEntry(const std::string& name, const std::string& order, float score,
            const std::string& displayName, int expireIn, const Json::Value& profile,
			const std::string& accessToken, PostLeaderboardEntryCallback callback);
        
		void addLeaderboardEntry(const std::string& name, const std::string& order, float score,
            const std::string& displayName, int expireIn,
			const std::string& accessToken, PostLeaderboardEntryCallback callback);

    protected:
        LeaderboardService(const std::string& location);
        bool init();
        
    private:
    };
};

#endif
