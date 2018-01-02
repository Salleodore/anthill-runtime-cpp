
#ifndef ONLINE_Social_Service_H
#define ONLINE_Social_Service_H

#pragma warning(disable : 4503)

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include <json/value.h>

#include <set>
#include <unordered_map>
#include <list>

namespace online
{
	typedef std::shared_ptr< class SocialService > SocialServicePtr;

	class AnthillRuntime;
    
	class SocialConnection
	{
		friend class SocialService;
        
    public:
    
        struct CredentialInfo
        {
            CredentialInfo(const Json::Value& data);
        
            Json::Value social;
        };
    
        typedef std::unordered_map<std::string, CredentialInfo> Credentials;
        
    public:
        SocialConnection(const Json::Value& data);
    
        const Credentials& getCredentials() const { return m_credentials; }
		const Json::Value& getProfile() const { return m_profile; }
        
    private:
        Credentials m_credentials;
        Json::Value m_profile;
	};
	    
	class SocialRequest
	{

    public:		
		enum class Type
		{
			group = 1,
			account,
			unknown
		};
		
		enum class Kind
		{
			outgoing = 1,
			incoming,
			unknown
		};

		friend class SocialService;
    
    public:
        SocialRequest(const Json::Value& data);
    
        Type getType() const { return m_type; }
		Kind getKind() const { return m_kind; }
		const std::string& getObject() const { return m_object; }
		const std::string& getSender() const { return m_sender; }
		const Json::Value& getPayload() const { return m_payload; }
		const std::string& getKey() const { return m_key; }
		const Json::Value& getProfile() const { return m_profile; }
        
    private:
        Type m_type;
		Kind m_kind;
		std::string m_object;
		std::string m_sender;
		Json::Value m_payload;
		std::string m_key;
        Json::Value m_profile;
	};
    
	class SocialUniqueName
	{

    public:		

		friend class SocialService;
    
    public:
        SocialUniqueName(const Json::Value& data);
		
		const std::string& getName() const { return m_name; }
		const std::string& getAccount() const { return m_account; }
		const Json::Value& getProfile() const { return m_profile; }
        
    private:

		std::string m_name;
		std::string m_account;
        Json::Value m_profile;
	};

    typedef std::unordered_map<std::string, SocialConnection> SocialConnections;
	
    typedef std::list<SocialRequest> SocialRequests;
    typedef std::list<SocialUniqueName> SocialUniqueNames;

	class SocialService : public Service
	{
		friend class AnthillRuntime;

	public:
		static const std::string ID;
        static const std::string API_VERSION;
		
		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request,
			const SocialConnections& connections) > GetConnectionsCallback;
		
		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request,
			const std::string& key) > AddConnectionsCallback;
		
		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request,
			const SocialRequests& requests) > GetRequestsCallback;

		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request,
			std::set<std::string> accountIds) > GetAccountIdsCallback;
		
		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request,
			const SocialUniqueNames& names) > GetUniqueNamesCallback;

		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request) > DeleteConnectionsCallback;
		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request) > ApproveConnectionsCallback;
		typedef std::function< void(const SocialService& service, Request::Result result, const Request& request) > RejectConnectionsCallback;
		
	public:
		static SocialServicePtr Create(const std::string& location);
		virtual ~SocialService();

		void getConnections(
			const std::string& accessToken,
            const std::set<std::string>& profileFields,
            GetConnectionsCallback callback);

		void getIncomingRequests(
			const std::string& accessToken,
            const std::set<std::string>& profileFields,
            GetRequestsCallback callback);

		void getOutgoingRequests(
			const std::string& accessToken,
            const std::set<std::string>& profileFields,
            GetRequestsCallback callback);
        
		void getRequests(
			const std::string& accessToken,
            const std::set<std::string>& profileFields,
            GetRequestsCallback callback);

		void getAccountIdsByCredentials(
			const std::set< std::string >& credentials,
			const std::string& accessToken,
            const std::set<std::string>& profileFields,
            GetAccountIdsCallback callback);
        
		void addConnection(
			const std::string& account,
			const std::string& accessToken,
            AddConnectionsCallback callback,
			bool approval = true,
			const Json::Value& notify = Json::Value(),
			const Json::Value& payload = Json::Value());
		
		void deleteConnection(
			const std::string& account,
			const std::string& accessToken,
            DeleteConnectionsCallback callback,
			const Json::Value& notify = Json::Value());

		void approveConnection(
			const std::string& account,
			const std::string& key,
			const std::string& accessToken,
            ApproveConnectionsCallback callback,
			const Json::Value& notify = Json::Value());

		void rejectConnection(
			const std::string& account,
			const std::string& key,
			const std::string& accessToken,
            RejectConnectionsCallback callback,
			const Json::Value& notify = Json::Value());
		
		void getUniqueNames(
			const std::string& kind,
			const std::string& query,
			const std::string& accessToken,
            GetUniqueNamesCallback callback,
			bool requestProfiles = false,
			const std::set<std::string>& profileFields = {});

	protected:
		SocialService(const std::string& location);
        bool init();

	private:
	};
};

#endif
