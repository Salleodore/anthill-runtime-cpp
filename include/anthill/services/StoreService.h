
#ifndef ONLINE_Store_Service_H
#define ONLINE_Store_Service_H

#include "anthill/services/Service.h"
#include "anthill/requests/Request.h"
#include "anthill/ApplicationInfo.h"
#include <json/value.h>

#include <set>
#include <unordered_map>
#include <list>

namespace online
{
    typedef std::shared_ptr< class StoreService > StoreServicePtr;
    typedef std::shared_ptr< class Store > StorePtr;
    
    class AnthillRuntime;
    
    class Store
    {
    public:
    
        class Tier
        {
        public:
        
            class Price
            {
            public:
                Price(const std::string& currency);
                
            public:
                void read(const Json::Value& data, const Store& store);
                
            public:
                const std::string& getFormat() const { return m_format; }
                const std::string& getTitle() const { return m_title; }
                const std::string& getSymbol() const { return m_symbol; }
                const std::string& getLabel() const { return m_label; }
                const std::string& getCurrency() const { return m_currency; }
                
                int getPrice() const { return m_price; }
				std::string getFormattedPrice( int amount = 1 ) const;
                
            private:
                std::string m_currency;
                std::string m_format;
                std::string m_title;
                std::string m_symbol;
                std::string m_label;
                
                int m_price;
            };
            
        public:
            typedef std::unordered_map<std::string, Price> Prices;
            
        public:
            void read(const Json::Value& data, const Store& store);
            
        public:
            const Prices& getPrices() const { return m_prices; }
            const std::string& getProduct() const { return m_product; }
        
        private:
            Prices m_prices;
            std::string m_product;
        };
    
        class Item
        {
        public:
            class Billing
            {
            public:
                enum class Type
                {
                    IAP,
                    Offline
                };
            public:
                virtual void read(const Json::Value& data, const Store& store) = 0;
                virtual Type getType() const = 0;
            };
            
            class IAPBilling: public Billing
            {
            public:
                virtual void read(const Json::Value& data, const Store& store) override;
                virtual Type getType() const override { return Type::IAP; }
                
            public:
                const Tier* getTier() const { return m_tier; }
            private:
                const Tier* m_tier;
            };
            
            class OfflineBilling: public Billing
            {
            public:
                virtual void read(const Json::Value& data, const Store& store) override;
                virtual Type getType() const override { return Type::Offline; }
                
            public:
                const std::string& getCurrency() const { return m_currency; }
                int getAmount() const { return m_amount; }
            private:
                std::string m_currency;
                int m_amount;
            };
        
        public:
            typedef std::shared_ptr< class Billing > BillingPtr;
            typedef std::shared_ptr< class IAPBilling > IAPBillingPtr;
            typedef std::shared_ptr< class OfflineBilling > OfflineBillingPtr;
            
        public:
            void read(const Json::Value& data, const Store& store);
        
        public:
            const std::string& getCategory() const { return m_category; }
            const std::string& getId() const { return m_id; }
            
            const BillingPtr& getBilling() const { return m_billing; }
            const Json::Value& getPublicPayload() const { return m_publicPayload; }
            const Json::Value& getOptions() const { return m_options; }
        
        private:
            std::string m_category;
            std::string m_id;
            
            BillingPtr m_billing;
            Json::Value m_publicPayload;
            Json::Value m_options;
        };
    
    public:
        typedef std::list<Item> Items;
        typedef std::unordered_map<std::string, Tier> Tiers;
    
    public:
        Store(const std::string& name);
        void read(const Json::Value& data);
        
    public:
        const std::string& getName() const { return m_name; }
        const Items& getItems() const { return m_items; }
        const Tiers& getTiers() const { return m_tiers; }
    
    private:
        std::string m_name;
        Items m_items;
        Tiers m_tiers;
    };
    
    class StoreService : public Service
    {
        friend class AnthillRuntime;
    public:
        typedef std::function< void(const StoreService& service, Request::Result result, const Request& request,
                                    const StorePtr& store) > GetStoreCallback;
        
    public:
        static const std::string ID;
        static const std::string API_VERSION;
        
    public:
        static StoreServicePtr Create(const std::string& location);
        virtual ~StoreService();
        
        void getStore(const std::string& name, const std::string& accessToken, GetStoreCallback callback);

    protected:
        StoreService(const std::string& location);
        bool init();
        
    private:
    };
};

#endif
