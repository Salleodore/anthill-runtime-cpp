
#ifndef ONLINE_DLC_Service_H
#define ONLINE_DLC_Service_H

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"

#include <json/value.h>

#include <set>
#include <unordered_map>
#include <list>

namespace online
{
    typedef std::shared_ptr< class DLCService > DLCServicePtr;
    
    class AnthillRuntime;
    
    class DLCService : public Service
    {
        friend class AnthillRuntime;
        
    public:
        class Bundle
        {
            friend class DLCService;
        
        public:
            Bundle(
				const std::string& name,
				long size,
				const std::string& url,
				const std::string& sha256,
				const std::string& crc32,
				const Json::Value& payload ) :
                m_size(size),
                m_name(name),
                m_url(url),
                m_sha256(sha256),
				m_crc32(crc32),
                m_payload(payload)
            {
            }
        
            long getSize() const { return m_size; }
            const std::string& getName() const { return m_name; }
            const std::string& getUrl() const { return m_url; }
            const std::string& getSHA256() const { return m_sha256; }
			const std::string& getCRC32() const { return m_crc32; }
            const Json::Value& getPayload() const { return m_payload; }
            
        private:
            long m_size;
            std::string m_name;
            std::string m_url;
            std::string m_sha256;
			std::string m_crc32;
            Json::Value m_payload;
        };
        
        typedef std::list<Bundle> Bundles;
    public:
        typedef std::function< void(const DLCService& service, Request::Result result, const Request& request) > GetUpdatesCallback;
        
    public:
        static const std::string ID;
        static const std::string API_VERSION;
        
    public:
        static DLCServicePtr Create(const std::string& location);
        virtual ~DLCService();
        
        void getUpdates(Bundles& bundlesOutput, GetUpdatesCallback callback);
        void getUpdates(Bundles& bundlesOutput, GetUpdatesCallback callback, 
						const std::string& applicationName, const std::string& applicationVersion,
						const Json::Value& env);

    protected:
        DLCService(const std::string& location);
        bool init();
        
    private:
    };
};

#endif
