
#ifndef ONLINE_Storage_H
#define ONLINE_Storage_H

#include <memory>
#include <string>

namespace online
{
	typedef std::shared_ptr< class Storage > StoragePtr;

	class Storage
	{	
	public:
		static const std::string StorageUsernameField;
		static const std::string StoragePasswordField;
		static const std::string StorageAccessTokeneField;

	public:
		virtual void set(const std::string& key, const std::string& value) = 0;
		virtual std::string get(const std::string& key) const = 0;
		virtual bool has(const std::string& key) const = 0;
		virtual void remove(const std::string& key) = 0;
		virtual void save() = 0;
	};
};

#endif