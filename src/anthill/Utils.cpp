
#include "anthill/Utils.h"

#include <iterator>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <string>
#include <regex>

#if defined( WIN32 ) || defined( _WIN32 )
	#include <Windows.h>
#else
    #include <sys/types.h>
    #include <dirent.h>
#endif

namespace online
{
	Credential::Credential(const std::string& raw)
	{
		size_t pos = raw.find_first_of(':');

		if (pos == raw.npos)
			return;

		m_credentialType = raw.substr(0, pos);
		m_username = raw.substr(pos + 1);
	}

    std::time_t get_utc_timestamp()
    {
        std::time_t now = std::time(0);
#if defined( WIN32 ) || defined( _WIN32 )
        return mktime(std::gmtime(&now)) - _timezone;
#else
        return mktime(std::gmtime(&now)) - timezone;
#endif
    }
    
    std::time_t parse_time(const std::string& time)
    {
        std::tm tm = {};
        
#if defined( WIN32 ) || defined( _WIN32 )
        std::stringstream ss(time);
        ss >> std::get_time(&tm, "%Y-%m-%d  %H:%M:%S");
        return std::mktime(&tm) - _timezone;
#else
        strptime(time.c_str(), "%Y-%m-%d  %H:%M:%S", &tm);
        return std::mktime(&tm) - timezone;
#endif
    }

	std::string dump_time(std::time_t time, bool includeDate, bool local)
	{
		std::tm * ptm = local ? std::localtime( &time ) : std::gmtime( &time );
#if defined( WIN32 ) || defined( _WIN32 )      
		std::stringstream ss;
		ss << std::put_time( ptm, includeDate ? "%Y-%m-%d %H:%M:%S" : "%H:%M:%S" );
		return ss.str();
#else
		char buffer[32];
		std::strftime( buffer, 32, includeDate ? "%Y-%m-%d %H:%M:%S" : "%H:%M:%S", ptm );
		return std::string( buffer );
#endif
	}

    std::string url_encode(const std::string &value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
            std::string::value_type c = (*i);

            // Keep alphanumeric and other accepted characters intact
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
                continue;
            }

            // Any other characters are percent-encoded
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char) c);
            escaped << std::nouppercase;
        }

        return escaped.str();
    }

    std::string url_decode(const std::string &value)
    {
        std::string ret;
        char ch;
        size_t i, ii;
        
        for (i=0; i<value.length(); i++)
        {
            if (int(value[i])==37)
            {
                sscanf(value.substr(i+1,2).c_str(), "%x", &ii);
                ch=static_cast<char>(ii);
                ret+=ch;
                i=i+2;
            }
            else
            {
                ret+=value[i];
            }
        }
        return (ret);
    }
    
    std::unordered_map<std::string, std::string> parse_query_arguments(const std::string &url)
    {
        std::unordered_map<std::string, std::string> data;
        std::regex pattern("([\\w+%]+)=([^&]*)");
        auto words_begin = std::sregex_iterator(url.begin(), url.end(), pattern);
        auto words_end = std::sregex_iterator();

        for (std::sregex_iterator i = words_begin; i != words_end; i++)
        {
            std::string key = (*i)[1].str();
            std::string value = (*i)[2].str();
            data[key] = url_decode(value);
        }

        return data;
    }

	std::string join(const std::set<std::string>& elements, const char* const separator)
	{
		switch (elements.size())
		{
		case 0:
			return "";
		case 1:
			return *elements.begin();
		default:
			std::ostringstream os;
			std::copy(elements.begin(), std::prev(elements.end()), std::ostream_iterator<std::string>(os, separator));
			os << *elements.rbegin();
			return os.str();
		}
	}
 
    bool list_files_in_directory(const std::string& directory, std::list<std::string>& files, std::function<bool(const std::string&)> predicate)
    {
#if defined( WIN32 ) || defined( _WIN32 )
        std::string search_path = directory + "/*.*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
        
        if(hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                // read all (real) files in current folder
                // , delete '!' read other 2 default folder . and ..
                if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                {
                    std::string filename = fd.cFileName;
                    if (!predicate || predicate(filename))
                    {
                        files.push_back(filename);
                    }
                }
            }while(::FindNextFile(hFind, &fd));
            ::FindClose(hFind);
            return true;
        }
        
        return false;
#else
        DIR *dp;
        struct dirent *dirp;
        if((dp  = opendir(directory.c_str())) == NULL)
        {
            return false;
        }

        while ((dirp = readdir(dp)) != NULL)
        {
            std::string filename = std::string(dirp->d_name);
            if (!predicate || predicate(filename))
            {
                files.push_back(filename);
            }
        }

        closedir(dp);
        return true;
#endif
    }

	void _assert(const std::string& expr_str, bool expr, const std::string& file, int line, const std::string& msg)
	{
		if (!expr)
		{
			std::cerr << "Assert failed:\t" << msg << "\n"
				<< "Expected:\t" << expr_str << "\n"
				<< "Source:\t\t" << file << ", line " << line << "\n";
			abort();
		}
	}

	std::string random_string(size_t length)
	{
		auto randchar = []() -> char
		{
			const char charset[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[rand() % max_index];
		};
		std::string str(length, 0);
		std::generate_n(str.begin(), length, randchar);
		return str;
	}
}
