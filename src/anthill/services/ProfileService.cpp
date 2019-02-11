
#include "anthill/services/ProfileService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <json/writer.h>

namespace online
{
    const std::string ProfileService::ID = "profile";
    const std::string ProfileService::API_VERSION = "0.2";
    
    ProfileServicePtr ProfileService::Create(const std::string& location)
    {
        ProfileServicePtr _object(new ProfileService(location));
        if (!_object->init())
            return ProfileServicePtr(nullptr);
        
        return _object;
    }
    
    ProfileService::ProfileService(const std::string& location) :
        Service(location)
    {
        
	}
	
    void ProfileService::getMassProfiles(const std::set<std::string>& accounts, const std::set<std::string>& profileFields, 
		const std::string& accessToken, GetMassUserProfilesCallback callback)
	{
		JsonRequestPtr request = JsonRequest::Create(getLocation() + "/profiles", Request::METHOD_GET);
        
        if (request)
        {
			request->setName("profile_profiles");
            request->setAPIVersion(API_VERSION);
        
			Json::Value accounts_(Json::ValueType::arrayValue);
			Json::Value profileFields_(Json::ValueType::arrayValue);

			for (const std::string& account: accounts)
			{
				accounts_.append(account);
			}

			for (const std::string& profileField: profileFields)
			{
				profileFields_.append(profileField);
			}
			
			Json::FastWriter w;
			
            request->setRequestArguments({
                {"access_token", accessToken },
                {"accounts", w.write(accounts_) },
                {"profile_fields", w.write(profileFields_) },
            });
            
            request->setOnResponse([=](const online::JsonRequest& request)
            {
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();

					if (value.isObject())
					{
						Profiles profiles;

						for (Json::ValueConstIterator it = value.begin(); it != value.end(); it++)
						{
							profiles[it.name()] = *it;
						}

						callback(*this, request.getResult(), request, profiles);
					}
					else
					{
						callback(*this, request.getResult(), request, Profiles());
					}
                   
				}
				else
				{
					callback(*this, request.getResult(), request, Profiles());
				}
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
	}

	void ProfileService::getMyProfile(const std::string& path, UserProfile& profile, const std::string& accessToken, GetUserProfileCallback callback)
	{
		getProfile("me", path, profile, accessToken, callback);
	}

	void ProfileService::setMyProfile(const std::string& path, const UserProfile& profile, bool merge, const std::string& accessToken, SetUserProfileCallback callback)
	{
		setProfile("me", path, profile, merge, accessToken, callback);
	}
    
    void ProfileService::getMyProfile(const std::string& path, const std::string& accessToken, GetProfileCallback callback)
    {
        getProfile("me", path, accessToken, callback);
    }
    
	void ProfileService::setMyProfile(const std::string& path, const Json::Value& profile, bool merge, const std::string& accessToken, SetProfileCallback callback)
    {
        setProfile("me", path, profile, merge, accessToken, callback);
    }

	void ProfileService::setProfile(const std::string& account, const std::string& path,
		const UserProfile& userProfile, bool merge, const std::string& accessToken, SetUserProfileCallback callback)
	{
		Json::Value value;
		userProfile.write(value);

		setProfile(account, path, value, merge, accessToken,
			[callback, &userProfile](const ProfileService& service, Request::Result result, const Request& request, const Json::Value& profile)
		{
			callback(service, result, request);
		});
	}

	void ProfileService::getProfile(const std::string& account, const std::string& path, 
		UserProfile& userProfile, const std::string& accessToken, GetUserProfileCallback callback)
	{
		getProfile(account, path, accessToken,
			[callback, &userProfile](const ProfileService& service, Request::Result result, const Request& request, const Json::Value& profile)
		{
			if (request.isSuccessful())
			{
				userProfile.read(profile);
			}

			callback(service, result, request);
		});
	}
    
    void ProfileService::getProfile(const std::string& account, const std::string& path, const std::string& accessToken, GetProfileCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/profile/" + account, Request::METHOD_GET);
        
        if (request)
        {
			request->setName("profile_profile");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"access_token", accessToken }
            });
            
            request->setOnResponse([=](const online::JsonRequest& request)
            {
               if (request.isSuccessful() && request.isResponseValueValid())
               {
				   const Json::Value& value = request.getResponseValue();
                   
                   callback(*this, request.getResult(), request, value);
               }
               else
               {
				   callback(*this, request.getResult(), request, Json::Value());
               }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    void ProfileService::setProfile(const std::string& account, const std::string& path,
		const Json::Value& profile, bool merge, const std::string& accessToken, SetProfileCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/profile/" + account, Request::METHOD_POST);
        
        if (request)
        {
			request->setName("profile_profile2");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"access_token", accessToken }
            });
            
            Json::FastWriter fastWriter;
            
            request->setPostFields({
                {"data", fastWriter.write(profile)},
                {"merge", merge ? "true" : "false"}
            });
            
            request->setOnResponse([=](const online::JsonRequest& request)
            {
               if (request.isSuccessful() && request.isResponseValueValid())
               {
                   // get merged profile
				   const Json::Value& value = request.getResponseValue();
                   
                   callback(*this, request.getResult(), request, value);
               }
               else
               {
				   callback(*this, request.getResult(), request, Json::Value());
               }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    ProfileService::~ProfileService()
    {
        //
    }
    
    bool ProfileService::init()
    {
        return true;
    }
}
