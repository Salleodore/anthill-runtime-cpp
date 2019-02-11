
#include "anthill/services/LoginService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include "json/writer.h"

namespace online
{
	const std::string LoginService::ID = "login";
	const std::string LoginService::Anonymous = "anonymous";
    const std::string LoginService::API_VERSION = "0.2";

	LoginServicePtr LoginService::Create(const std::string& location)
	{
		LoginServicePtr _object(new LoginService(location));
		if (!_object->init())
			return LoginServicePtr(nullptr);

		return _object;
	}

	LoginService::LoginService(const std::string& location) :
		Service(location)
	{
		const ListenerPtr& onlineListener = AnthillRuntime::Instance().getListener();

		if (onlineListener && onlineListener->shouldHaveExternalAuthenticator())
		{
			setExternalAuthenticator(onlineListener->createExternalAuthenticator());
		}
	}

    void LoginService::extend(
        const std::string& accessToken,
        const std::string& extendWith,
        const Scopes& scopes,
        AuthenticationCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/extend", Request::METHOD_POST);

		if (request)
		{
			request->setName("login_extend");
            request->setAPIVersion(API_VERSION);
        
			Request::Fields arguments;

			arguments["access_token"] = extendWith;
			arguments["extend"] = accessToken;
			arguments["scopes"] = join(scopes, ",");

			request->setPostFields(arguments);

			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();

					if (!value.isMember("token"))
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, "", "", "", Scopes());

						return;
					}
                    
					std::string account;
					std::string credential;
                    Scopes scopes;

					if (value.isMember("account"))
					{
						account = value["account"].asString();
					}

					if (value.isMember("credential"))
					{
						credential = value["credential"].asString();
					}
                    
                    if (value.isMember("scopes"))
                    {
                        const Json::Value& scopes_ = value["scopes"];
                        
                        for (Json::ValueConstIterator it = scopes_.begin(); it != scopes_.end(); it++)
                        {
                            scopes.insert(it->asString());
                        }
                    }

					std::string token = value["token"].asString();
					callback(*this, request.getResult(), request, token, credential, account, scopes);
				}
				else
				{
					callback(*this, request.getResult(), request, "", "", "", Scopes());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
    }

	void LoginService::authenticate(
		const std::string& credentialType,
		const std::string& gamespace,
		const Scopes& scopes,
		const Request::Fields& other,
		LoginService::AuthenticationCallback callback,
        LoginService::MergeRequiredCallback mergeRequiredCallback,
        const Scopes& shouldHaveScopes)
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/auth", Request::METHOD_POST);

		if (request)
		{
			request->setName("login_auth");
            request->setAPIVersion(API_VERSION);
        
			Request::Fields arguments = other;

			arguments["credential"] = credentialType;
			arguments["gamespace"] = gamespace;
			arguments["scopes"] = join(scopes, ",");
			arguments["full"] = "true";
            arguments["should_have"] = join(shouldHaveScopes, ",");

			request->setPostFields(arguments);

			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();

					if (!value.isMember("token"))
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, "", "", "", Scopes());

						return;
					}

					std::string token = value["token"].asString();
                    
					std::string account;
					std::string credential;
                    Scopes scopes;

					if (value.isMember("account"))
					{
						account = value["account"].asString();
					}

					if (value.isMember("credential"))
					{
						credential = value["credential"].asString();
					}
                    
    
                    if (value.isMember("scopes"))
                    {
                        const Json::Value& scopes_ = value["scopes"];
                        
                        for (Json::ValueConstIterator it = scopes_.begin(); it != scopes_.end(); it++)
                        {
                            scopes.insert(it->asString());
                        }
                    }
                    
					callback(*this, request.getResult(), request, token, credential, account, scopes);
				}
				else
				{
					switch (request.getResult())
					{
						case Request::MULTIPLE_CHOISES:
						{
							const Json::Value& value = request.getResponseValue();

							if (value.isMember("result_id"))
							{
								std::string resultId = value["result_id"].asString();

								if (resultId == "multiple_accounts_attached")
								{
									if (value.isMember("accounts") && value.isMember("resolve_token"))
									{
										const Json::Value& accounts = value["accounts"];
										std::string resolveToken = value["resolve_token"].asString();

										MergeOptions mergeOptions;

										for (Json::ValueConstIterator it = accounts.begin(); it != accounts.end(); it++)
										{
											const Json::Value& account = (*it);

											if (account.isMember("account") && account.isMember("profile"))
											{
												MergeOption option;
												option.m_name = it.name();
												option.m_profile = account["profile"];
												option.m_credential = "";
												option.m_account = account["account"].asString();

												mergeOptions.push_back(option);
											}
											else
											{
												Log::get() << "Bad merge option: " << it.name() << std::endl;
												callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
												return;
											}
										}

										mergeRequiredCallback(*this, mergeOptions, 
											[=](const MergeOption& selectedOption)
										{
											Request::Fields other;

											other["attach_to"] = m_currentToken;

											resolve(resolveToken, "multiple_accounts_attached", selectedOption.m_name, scopes, other, callback, shouldHaveScopes);
										});
									}
									else
									{
										callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
									}
								}
								else
								{
									callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
								}
							}
							else
							{
								callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
							}

							break;
						}
						default:
						{
							callback(*this, request.getResult(), request, "", "", "", Scopes());
							break;
						}
                    }
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
	
	void LoginService::graphAuthCode(
		const std::string& credentialType,
		const std::string& accessToken,
		const std::string& gamespace,
		const std::string& redirectUri,
		LoginService::GraphCodeAuthCallback callback )
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/pre_auth/" + credentialType, Request::METHOD_GET);

		if (request)
		{
			request->setName("login_pre_auth");
            request->setAPIVersion(API_VERSION);
        
			request->setRequestArguments({
                {"access_token", accessToken },
                {"gamespace", gamespace },
                {"redirect_uri", redirectUri }
            });

			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();
					std::string code;
					if (value.isMember( "code" ))
					{
						code = value["code"].asString();
					}

					callback(*this, request.getResult(), request, code);
				}
				else
				{
					Log::get() << "graphAuthCode failed!" << std::endl;
					callback(*this, request.getResult(), request, std::string());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
	
	void LoginService::attach(
		const std::string& credentialType,
		const std::string& gamespace,
		const Scopes& scopes,
		const Request::Fields& other,
		LoginService::AuthenticationCallback callback,
		MergeRequiredCallback mergeRequiredCallback,
        const Scopes& shouldHaveScopes)
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/auth", Request::METHOD_POST);

		if (request)
		{
			request->setName("login_auth");
            request->setAPIVersion(API_VERSION);
        
			Request::Fields arguments = other;

			arguments["credential"] = credentialType;
			arguments["gamespace"] = gamespace;
			arguments["scopes"] = join(scopes, ",");
			arguments["full"] = "true";
            arguments["should_have"] = join(shouldHaveScopes, ",");
            arguments["attach_to"] = m_currentToken;

			request->setPostFields(arguments);

			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();

					if (!value.isMember("token"))
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, "", "", "", Scopes());

						return;
					}
                    
					std::string account;
					std::string credential;
                    Scopes scopes;

					if (value.isMember("account"))
					{
						account = value["account"].asString();
					}

					if (value.isMember("credential"))
					{
						credential = value["credential"].asString();
					}
    
                    if (value.isMember("scopes"))
                    {
                        const Json::Value& scopes_ = value["scopes"];
                        
                        for (Json::ValueConstIterator it = scopes_.begin(); it != scopes_.end(); it++)
                        {
                            scopes.insert(it->asString());
                        }
                    }

					std::string token = value["token"].asString();
					callback(*this, request.getResult(), request, token, credential, account, scopes);
				}
				else
				{
					switch (request.getResult())
					{
						case Request::CONFLICT:
						{
							const Json::Value& value = request.getResponseValue();

							if (value.isMember("result_id"))
							{
								std::string resultId = value["result_id"].asString();

								if (resultId == "merge_required")
								{
									if (value.isMember("accounts") && value.isMember("resolve_token"))
									{
										const Json::Value& accounts = value["accounts"];
										std::string resolveToken = value["resolve_token"].asString();

										MergeOptions mergeOptions;

										for (Json::ValueConstIterator it = accounts.begin(); it != accounts.end(); it++)
										{
											const Json::Value& account = (*it);

											if (account.isMember("account") && account.isMember("profile") && account.isMember("credential"))
											{
												MergeOption option;
												option.m_name = it.name();
												option.m_profile = account["profile"];
												option.m_credential = account["credential"].asString();
												option.m_account = account["account"].asString();

												mergeOptions.push_back(option);
											}
											else
											{
												Log::get() << "Bad merge option: " << it.name() << std::endl;
												callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
												return;
											}
										}

										if (mergeOptions.size() != 2)
										{
											Log::get() << "Bad number of merge options: " << mergeOptions.size() << std::endl;
											callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
											return;
										}

										mergeRequiredCallback(*this, mergeOptions, 
											[=](const MergeOption& selectedOption)
										{
											Request::Fields other;

											other["attach_to"] = m_currentToken;

											resolve(resolveToken, "merge_required", selectedOption.m_name, scopes, other, callback, shouldHaveScopes);
										});
									}
									else
									{
										callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
									}
								}
								else
								{
									callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
								}
							}
							else
							{
								callback(*this, Request::BAD_ARGUMENTS, request, "", "", "", Scopes());
							}

							break;
						}
						default:
						{
							callback(*this, request.getResult(), request, "", "", "", Scopes());
							break;
						}
					}
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
	
	void LoginService::resolve(
		const std::string& resolveToken,
		const std::string& methodToResolve,
		const std::string& resolveWith,
		const Scopes& scopes,
		const Request::Fields& other,
		AuthenticationCallback callback,
        const Scopes& shouldHaveScopes)
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/resolve", Request::METHOD_POST);

		if (request)
		{
			request->setName("login_resolve");
            request->setAPIVersion(API_VERSION);
        
			Request::Fields arguments = other;

			arguments["resolve_method"] = methodToResolve;
			arguments["resolve_with"] = resolveWith;
			arguments["scopes"] = join(scopes, ",");
			arguments["full"] = "true";
            arguments["should_have"] = join(shouldHaveScopes, ",");
            arguments["access_token"] = resolveToken;

			request->setPostFields(arguments);

			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();

					if (!value.isMember("token"))
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, "", "", "", Scopes());

						return;
					}
                    
					std::string account;
					std::string credential;
                    Scopes scopes;

					if (value.isMember("account"))
					{
						account = value["account"].asString();
					}

					if (value.isMember("credential"))
					{
						credential = value["credential"].asString();
					}
    
                    if (value.isMember("scopes"))
                    {
                        const Json::Value& scopes_ = value["scopes"];
                        
                        for (Json::ValueConstIterator it = scopes_.begin(); it != scopes_.end(); it++)
                        {
                            scopes.insert(it->asString());
                        }
                    }

					std::string token = value["token"].asString();
					callback(*this, request.getResult(), request, token, credential, account, scopes);
				}
				else
				{
					callback(*this, request.getResult(), request, "", "", "", Scopes());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}


	void LoginService::authenticateAnonymous(
		const std::string& username,
		const std::string& password,
		const std::string& gamespace,
		const Scopes& scopes,
		const Request::Fields& other,
		LoginService::AuthenticationCallback callback,
        LoginService::MergeRequiredCallback mergeRequiredCallback,
        const Scopes& shouldHaveScopes)
	{
		Request::Fields data = other;

		data["username"] = username;
		data["key"] = password;

		authenticate(Anonymous, gamespace, scopes, data, callback, mergeRequiredCallback, shouldHaveScopes);
	}
    
    void LoginService::getCredentials(const std::string& accessToken, GetCredentialsCallback callback)
    {
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/credentials", Request::METHOD_GET);

		if (request)
		{
			request->setName("login_credentials");
            request->setAPIVersion(API_VERSION);
        
			request->setRequestArguments({
                {"access_token", accessToken }
            });

			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();
                    
                    Credentials credentials;
    
                    if (value.isMember("credentials"))
                    {
                        const Json::Value& credentials_ = value["credentials"];
                        
                        for (Json::ValueConstIterator it = credentials_.begin(); it != credentials_.end(); it++)
                        {
                            credentials.insert(it->asString());
                        }
                    }

					callback(*this, request.getResult(), request, credentials);
				}
				else
				{
					callback(*this, request.getResult(), request, Credentials());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
    }
    
    void LoginService::getAccountIdsByCredentials(
                                                   const std::set< std::string >& credentials,
                                                   const std::string& accessToken,
                                                   GetAccountIdsCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(
                                                     getLocation() + "/accounts/credentials", Request::METHOD_GET);
        
        if (request)
        {
			request->setName("login_accounts_by_credentials");
            request->setAPIVersion(API_VERSION);
            
            Json::Value credentials_(Json::ValueType::arrayValue);
            for (const std::string& credential: credentials)
            {
                credentials_.append(credential);
            }
            
            Request::Fields fields = {
                {"access_token", accessToken },
                {"credentials", Json::FastWriter().write(credentials_)}
            };
            
            request->setRequestArguments(fields);
            
            request->setOnResponse([=](const online::JsonRequest& request)
                                   {
                                       if (request.isSuccessful() && request.isResponseValueValid())
                                       {
                                           const Json::Value& value = request.getResponseValue();
                                           std::set<std::string> accountIds;
                                           
                                           if (value.isMember("account_ids"))
                                           {
                                               const Json::Value& requestsJson = value["account_ids"];
                                               
                                               for (Json::ValueConstIterator it = requestsJson.begin(); it != requestsJson.end(); it++)
                                               {
                                                   accountIds.insert( it->asString() );
                                               }
                                           }
                                           
                                           callback(*this, request.getResult(), request, accountIds);
                                       }
                                       else
                                       {
                                           callback(*this, request.getResult(), request, {});
                                       }
                                   });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    void LoginService::getCredentials(GetCredentialsCallback callback)
    {
        getCredentials(getCurrentAccessToken(), callback);
    }

	void LoginService::validateAccessToken(const std::string& accessToken, ValidationCallback callback)
	{
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/validate", Request::METHOD_GET);

		if (request)
		{
			request->setName("login_validate");
            request->setAPIVersion(API_VERSION);
        
			request->setRequestArguments({
                {"access_token", accessToken }
            });

			request->setOnResponse([=](const online::JsonRequest& request)
			{
                if (request.isSuccessful() && request.isResponseValueValid())
                {
                    const Json::Value& value = request.getResponseValue();
                    
                    std::string credential;
                    std::string account;
                    Scopes scopes;
    
                    if (value.isMember("credential"))
                    {
                        credential = value["credential"].asString();
                    }
    
                    if (value.isMember("account"))
                    {
                        account = value["account"].asString();
                    }
    
                    if (value.isMember("scopes"))
                    {
                        const Json::Value& scopes_ = value["scopes"];
                        
                        for (Json::ValueConstIterator it = scopes_.begin(); it != scopes_.end(); it++)
                        {
                            scopes.insert(it->asString());
                        }

						if (scopes.empty())
						{
							Log::get() << "LoginService::validateAccessToken: scopes are empty!" << std::endl;
						}
                    }
					else
					{
						Log::get() << "LoginService::validateAccessToken: scopes field is not found!" << std::endl;
					}

                    callback(*this, request.getResult(), request, credential, account, scopes);
                }
                else
                {
					if (!request.isSuccessful())
						Log::get() << "LoginService::validateAccessToken: request failed!" << std::endl;
					else
					if (!request.isResponseValueValid())
						Log::get() << "LoginService::validateAccessToken: request response value is not valid!" << std::endl;

                    callback(*this, request.getResult(), request, "", "", {});
                }
                
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}
		
	bool LoginService::hasExternalAuthenticator() const
	{
		return (bool)m_externalAuthenticator;
	}
	
	bool LoginService::authenticateExternally(
		const std::string& gamespace,
		const LoginService::Scopes& scopes,
		const Request::Fields& other,
		LoginService::AuthenticationCallback callback,
		LoginService::MergeRequiredCallback mergeRequiredCallback,
        const LoginService::Scopes& shouldHaveScopes)
	{
		if (!m_externalAuthenticator)
			return false;

		m_externalAuthenticator->authenticate(
			*this,
			gamespace,
			scopes,
			other,
			callback,
			mergeRequiredCallback,
			shouldHaveScopes);

		return true;
	}
		
	bool LoginService::attachExternally(
		const std::string& gamespace,
		const LoginService::Scopes& scopes,
		const Request::Fields& other,
		LoginService::AuthenticationCallback callback,
		LoginService::MergeRequiredCallback mergeRequiredCallback,
        const LoginService::Scopes& shouldHaveScopes)
	{
		if (!m_externalAuthenticator)
			return false;

		m_externalAuthenticator->attach(
			*this,
			gamespace,
			scopes,
			other,
			callback,
			mergeRequiredCallback,
			shouldHaveScopes);
		
		return true;
	}

	void LoginService::validateAccessToken(ValidationCallback callback)
	{
		validateAccessToken(getCurrentAccessToken(), callback);
	}

	const std::string& LoginService::getCurrentAccessToken() const
	{
		return m_currentToken;
	}

	void LoginService::setCurrentAccessToken(const std::string& token)
	{
		m_currentToken = token;
	}

	LoginService::~LoginService()
	{
		//
	}

	bool LoginService::init()
	{
		return true;
	}
}
