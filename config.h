#ifndef CONFIG_H
#define CONFIG_H
#include <string>

// performs pretty much no input error checking.
class Config
{
public:
    static void readFile(const std::string& fileName = "config.txt");
    static void createConfig(); // creates an empty config file. throws std::ifstream::failure, on failure

    // returns empty string if no config read.
    static const std::string& userAgent();
    static const std::string& authToken();
    static const std::string& dbHost();
    static const std::string& dbPort();
    static const std::string& dbUser();
    static const std::string& dbPass();
    static const std::string& dbName();

private:
    static std::string m_userAgent;
    static std::string m_authToken;
    static std::string m_dbHost;
    static std::string m_dbPort;
    static std::string m_dbUser;
    static std::string m_dbPass;
    static std::string m_dbName;

    Config() = delete;
};

#endif // CONFIG_H
