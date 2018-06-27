#include "config.h"
#include "misc.h"
#include <fstream>
#include <stdexcept>
#include <limits>

std::string Config::m_userAgent = "";
std::string Config::m_authToken = "";
std::string Config::m_dbHost = "";
std::string Config::m_dbPort = "";
std::string Config::m_dbUser = "";
std::string Config::m_dbPass = "";
std::string Config::m_dbName = "";

constexpr const char * const userAgentField = "UserAgent:";
constexpr const char * const authTokenField = "oAuth-Token:";
constexpr const char * const dbHostField = "DatabaseHost:";
constexpr const char * const dbPortField = "DatabasePort:";
constexpr const char * const dbUserField = "DatabaseUser:";
constexpr const char * const dbPassField = "DatabasePassword:";
constexpr const char * const dbNameField = "DatabaseName:";

void Config::readFile(const std::string& fileName){
    std::ifstream fin;
    fin.open(fileName.c_str());

    std::string field, val;

    while(fin >> field){
        if(field[0] == '#'){
            // comment line. ignore it.
            fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            // a valid line should have exactly two whitespace delimited tokens.
            if( !(fin >> val) ){
                throw std::runtime_error("Malformed config file detected.");
            }
            switch(hash32(field.c_str())){
            case hash32(userAgentField):
                m_userAgent = val;
                break;
            case hash32(authTokenField):
                m_authToken = val;
                break;
            case hash32(dbHostField):
                m_dbHost = val;
                break;
            case hash32(dbPortField):
                m_dbPort = val;
                break;
            case hash32(dbUserField):
                m_dbUser = val;
                break;
            case hash32(dbPassField):
                m_dbPass = val;
                break;
            case hash32(dbNameField):
                m_dbName = val;
                break;
            default:
                throw std::runtime_error("Unexpected field found in config file: " + field);
            }
        }
    }
}

void Config::createConfig(){
    std::ofstream fout;

    fout.open("config.txt");

    fout << "# This is an example of a config file. \n";
    fout << "# All lines should be blank, start with a '#', or specify a config parameter. \n";
    fout << "# The below config parameters are invalid, and must be filled in by you. \n\n";

    fout << "# " << userAgentField << " UserAgent: this is typically your Github account.\n";
    fout << userAgentField << " GithubNewbie\n\n";

    fout << "# " << authTokenField << " also known as a personal access token. Google it, follow the guide. \n";
    fout << authTokenField << " 7876ab765c6556s4432f\n\n";

    fout << "# " << dbHostField << " The IP address or hostname of your SQL server. \n";
    fout << dbHostField << " 127.0.0.1\n\n";

    fout << "# " << dbPortField << " The port number of your SQL server. \n";
    fout << dbPortField << " 3300\n\n";

    fout << "# " << dbUserField << " The username to connect SQL server. \n";
    fout << dbUserField << " EdJOlmos\n\n";

    fout << "# " << dbPassField << " The password for your SQL user. \n";
    fout << dbPassField << " password123\n\n";

    fout << "# " << dbNameField << " The database name to insert into. \n";
    fout << dbNameField << " GithubProjects\n\n";

    fout.flush();
    fout.close();
}

const std::string& Config::userAgent(){
    return m_userAgent;
}

const std::string& Config::authToken(){
    return m_authToken;
}

const std::string& Config::dbHost(){
    return m_dbHost;
}

const std::string& Config::dbPort(){
    return m_dbPort;
}

const std::string& Config::dbUser(){
    return m_dbUser;
}

const std::string& Config::dbPass(){
    return m_dbPass;
}

const std::string& Config::dbName(){
    return m_dbName;
}
