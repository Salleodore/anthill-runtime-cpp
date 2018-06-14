
#ifndef ONLINE_Config_Service_H
#define ONLINE_Config_Service_H

#include "anthill/ApplicationInfo.h"
#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include <json/value.h>

#include <set>
#include <unordered_map>

namespace online
{
    typedef std::shared_ptr< class ConfigService > ConfigServicePtr;
    
    class AnthillRuntime;
    
    class ConfigService : public Service
    {
        friend class AnthillRuntime;
    public:
        typedef std::function< void(const ConfigService& service, Request::Result result, const Request& request,
                                    const std::string& configData) > GetConfigCallback;
        
    public:
        static const std::string ID;
        static const std::string API_VERSION;
        
    public:
        static ConfigServicePtr Create(const std::string& location);
        virtual ~ConfigService();
        
        void getConfig(GetConfigCallback callback);

    protected:
        ConfigService(const std::string& location, const std::string& configFileTempLocation);
        bool init();
        
    private:
        std::fstream m_configFile;
    };
};

#endif
