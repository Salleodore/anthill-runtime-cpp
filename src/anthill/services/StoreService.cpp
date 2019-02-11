
#include "anthill/services/StoreService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <json/writer.h>

namespace online
{
    const std::string StoreService::ID = "store";
    const std::string StoreService::API_VERSION = "0.2";
    
    
    /////////////////////////////////
    
    Store::Tier::Price::Price(const std::string& currency) :
        m_currency(currency)
    {
    }

	std::string Store::Tier::Price::getFormattedPrice( int amount ) const
	{
		std::ostringstream priceWithPrecision;
		priceWithPrecision << std::setprecision(2);
		priceWithPrecision << (m_price / 100.0f) * (float)amount;

		size_t plugPos = m_format.find( "{0}" );
		if( plugPos == std::string::npos )
			return priceWithPrecision.str();

		std::string result = m_format;
		result.replace( plugPos, 3, priceWithPrecision.str() );
		return result;
	}
    
    void Store::Tier::Price::read(const Json::Value& data, const Store& store)
    {
        if (data.isMember("format"))
            m_format = data["format"].asString();
        if (data.isMember("title"))
            m_title = data["title"].asString();
        if (data.isMember("symbol"))
            m_symbol = data["symbol"].asString();
        if (data.isMember("label"))
            m_label = data["label"].asString();
        
        if (data.isMember("price"))
            m_price = data["price"].asInt();
    }
    
    /////////////////////////////////
    
    void Store::Tier::read(const Json::Value& data, const Store& store)
    {
        if (data.isMember("product"))
        {
            m_product = data["product"].asString();
        }
        
        if (!data.isMember("prices"))
            return;
        
        const Json::Value& prices = data["prices"];
        
        for (Json::ValueConstIterator it = prices.begin(); it != prices.end(); it++)
        {
            const std::string& id = it.key().asString();
            
            std::pair<Prices::iterator, bool> insert = m_prices.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(id),
                std::forward_as_tuple(id));
            
            if (insert.second)
            {
                Price& price = insert.first->second;
                price.read(*it, store);
            }
        }
    }
    
    /////////////////////////////////
    
    void Store::Item::OfflineBilling::read(const Json::Value& data, const Store& store)
    {
        if (data.isMember("amount"))
        {
            m_amount = data["amount"].asInt();
        }
        
        if (data.isMember("currency"))
        {
            m_currency = data["currency"].asString();
        }
    }
    
    /////////////////////////////////
    
    void Store::Item::IAPBilling::read(const Json::Value& data, const Store& store)
    {
        if (data.isMember("tier"))
        {
            std::string tierName = data["tier"].asString();
            Store::Tiers::const_iterator it = store.getTiers().find(tierName);
            if (it != store.getTiers().end())
            {
                m_tier = &it->second;
            }
        }
    }
    
    /////////////////////////////////
    
    void Store::Item::read(const Json::Value& data, const Store& store)
    {
        if (data.isMember("id"))
            m_id = data["id"].asString();
        if (data.isMember("category"))
            m_category = data["category"].asString();
        if (data.isMember("public"))
            m_publicPayload = data["public"];
        
        if (data.isMember("billing"))
        {
            const Json::Value& billing = data["billing"];
            
            if (billing.isMember("type"))
            {
                std::string billingType = billing["type"].asString();
                
                if (billingType == "iap")
                {
                    m_billing = IAPBillingPtr(new IAPBilling());
                }
                else if (billingType == "offline")
                {
                    m_billing = OfflineBillingPtr(new OfflineBilling());
                }
                
                if (m_billing)
                {
                    m_billing->read(billing, store);
                }
            }
        }
    }
    
    /////////////////////////////////
    
    Store::Store(const std::string& name) :
        m_name(name)
    {
    }

    void Store::read(const Json::Value& data)
    {
        if (!data.isMember("store"))
            return;
        
        const Json::Value& store = data["store"];
        
        if (store.isMember("tiers"))
        {
            const Json::Value& tiers = store["tiers"];
            
            for (Json::ValueConstIterator it = tiers.begin(); it != tiers.end(); it++)
            {
                const std::string& id = it.key().asString();
                
                std::pair<Tiers::iterator, bool> insert = m_tiers.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(id),
                    std::forward_as_tuple());
                
                if (insert.second)
                {
                    Tier& tier = insert.first->second;
                    tier.read(*it, *this);
                }
            }
        }
        
        if (store.isMember("items"))
        {
            const Json::Value& items = store["items"];
            
            for (const Json::Value& it: items)
            {
                m_items.emplace_back();
                m_items.back().read(it, *this);
            }
        }
    }
    
    /////////////////////////////////
    
    StoreServicePtr StoreService::Create(const std::string& location)
    {
        StoreServicePtr _object(new StoreService(location));
        if (!_object->init())
            return StoreServicePtr(nullptr);
        
        return _object;
    }
    
    StoreService::StoreService(const std::string& location) :
        Service(location)
    {
        
	}
    
    void StoreService::getStore(const std::string& name, const std::string& accessToken, GetStoreCallback callback)
    {
        JsonRequestPtr request = JsonRequest::Create(
            getLocation() + "/store/" + name, Request::METHOD_GET);
        
        if (request)
        {
			request->setName("social_store");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"access_token", accessToken }
            });
            
            request->setOnResponse([=](const online::JsonRequest& request)
            {
               if (request.isSuccessful() && request.isResponseValueValid())
               {
                   StorePtr store = StorePtr(new Store(name));
                   store->read(request.getResponseValue());
                   
                   callback(*this, request.getResult(), request, store);
               }
               else
               {
				   callback(*this, request.getResult(), request, StorePtr());
               }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    StoreService::~StoreService()
    {
        //
    }
    
    bool StoreService::init()
    {
        return true;
    }
}
