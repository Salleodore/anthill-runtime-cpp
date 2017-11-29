
#ifndef ONLINE_Utils_H
#define ONLINE_Utils_H

#include <ctime>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <functional>

#ifndef NDEBUG
#define OnlineAssert(Expr, Msg) \
		online::_assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#define OnlineAssert(Expr, Msg) ;
#endif

namespace online
{
	class Credential
	{
	public:
		Credential(const std::string& raw);
		
		const std::string& getCredentialType() const { return m_credentialType; }
		const std::string& getUsername() const { return m_username; }

	private:
		std::string m_credentialType;
		std::string m_username;
	};

	std::string join(const std::set<std::string>& elements, const char* const separator);
	std::string random_string(size_t length);
    std::string url_encode(const std::string &value);
    std::string url_decode(const std::string &value);
    std::unordered_map<std::string, std::string> parse_query_arguments(const std::string &url);
    std::time_t get_utc_timestamp();
    std::time_t parse_time(const std::string& time);
	std::string dump_time(std::time_t time, bool includeDate = true, bool local = true);
    bool list_files_in_directory(const std::string& directory, std::list<std::string>& files, std::function<bool(const std::string&)> predicate = nullptr);

	void _assert(const std::string& expr_str, bool expr, const std::string& file, int line, const std::string& msg);
};

#endif
