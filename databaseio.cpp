#include "databaseio.h"
#include "config.h"
#include "misc.h"
#include "cppconn/prepared_statement.h"
#include <sstream>
#include <memory>
#include <iomanip>
#include <iostream>

DatabaseIo::DatabaseIo(){
    m_driver = sql::mysql::get_driver_instance();
    std::stringstream ss;
    ss << "tcp://" << Config::dbHost() << ":" << Config::dbPort();
    m_con = std::shared_ptr<sql::Connection>(m_driver->connect(ss.str().c_str(), Config::dbUser(), Config::dbPass()));
    m_stmt = std::shared_ptr<sql::Statement>(m_con->createStatement());
    selectDatabase(Config::dbName());
}

DatabaseIo::~DatabaseIo(){
}

void DatabaseIo::createDatabase(){
    std::string createDatabase = "CREATE DATABASE IF NOT EXISTS " + Config::dbName() + ";";

    std::string createProjects =
            "CREATE TABLE IF NOT EXISTS Projects ("
            "ProjectId INT UNSIGNED NOT NULL,"
            "OwnerId INT UNSIGNED NOT NULL,"
            "Timestamp INT UNSIGNED NOT NULL,"
            "ProjectName VARCHAR(255) NOT NULL,"
            "OwnerName VARCHAR(255) NOT NULL,"
            "PRIMARY KEY (ProjectId)"
            "); ";

    std::string createLanguages =
            "CREATE TABLE IF NOT EXISTS Languages ("
            "ProjectId INT UNSIGNED NOT NULL,"
            "Language VARCHAR(255) NOT NULL,"
            "Bytes INT UNSIGNED NOT NULL,"
            "FOREIGN KEY (ProjectId) REFERENCES Projects(ProjectId)"
            "); ";

    // TODO: statistics table

    try {
        executeQuery(createDatabase);
    } catch(...){
        std::cout << "WARNING: Database \"" << Config::dbName() << "\" already exists." << std::endl;
    }

    try {
        executeQuery(createProjects);
    } catch(...){
        std::cout << "WARNING: Table \"Projects\" already exists." << std::endl;
    }

    try {
        executeQuery(createLanguages);
    } catch(...){
        std::cout << "WARNING: Table \"Languages\" already exists." << std::endl;
    }
}

bool DatabaseIo::selectDatabase(const std::string& database){
    try {
        return m_stmt->execute("USE " + database);
    } catch(...){
        return true;
    }
}


// returns true on any error
bool DatabaseIo::insertProject(repoId_t repoId,
                               userId_t ownerId,
                               time_t timestamp,
                               const std::string& projectName,
                               const std::string& ownerName){
    std::stringstream ss;
    ss << "INSERT INTO Projects VALUES(" <<
          repoId << ", " <<
          ownerId << ", " <<
          timestamp << ", \"" <<
          projectName << "\", \"" <<
          ownerName << "\")";
    return execute(ss.str().c_str());
}

// returns true on any error
bool DatabaseIo::insertLanguage(repoId_t repoId,
                                const std::string& language,
                                uint32_t bytes){
    std::stringstream ss;
    ss << "INSERT INTO Languages VALUES(";
    ss << repoId << ", \"";
    ss << language << "\", ";
    ss << bytes << ")";
    return execute(ss.str().c_str());
}

bool DatabaseIo::execute(const std::string& statement){
    try {
        return m_stmt->execute(statement);
    } catch(...){
        return true;
    }
}


bool DatabaseIo::projectExists(repoId_t repoId,
                               const std::string& table){
    std::stringstream ss;
    try {
        ss << "SELECT ProjectId from " << table << " where ProjectId=" << repoId;
        sql::ResultSet* res = m_stmt->executeQuery(ss.str());
        return res->next();
    } catch(...){
        return false;
    }
}

void DatabaseIo::deleteProject(repoId_t repoId){
    execute("delete from Languages where ProjectId=" + std::to_string(repoId));
    execute("delete from Projects where ProjectId=" + std::to_string(repoId));
}

DatabaseIo::queryResult_t DatabaseIo::executeQuery(const std::string& query){
    return queryResult_t(m_stmt->executeQuery(query));
}

std::uint64_t DatabaseIo::getUniqueLanguagesCount(){
    queryResult_t res = executeQuery("SELECT COUNT(DISTINCT Language) from Languages order by Language");
    res->next();
    return res->getUInt64(1);
}

DatabaseIo::queryResult_t DatabaseIo::getUniqueLanguages(){
    return executeQuery("SELECT DISTINCT Language from Languages order by Language");
}

bool DatabaseIo::insertStatistics(const std::string& date,
                                  unsigned int projectCount,
                                  unsigned int partFuncProjs,
                                  unsigned int funcProjs,
                                  double partFuncProjsPct,
                                  double funcProjsPct){
    std::stringstream ss;
    ss << std::setprecision(20); // probably overkill. let the user truncate at their discretion

    ss << "INSERT INTO Statistics VALUES(\"";
    ss << date << "\", ";
    ss << projectCount << ", ";
    ss << partFuncProjs << ", ";
    ss << funcProjs << ", ";
    ss << partFuncProjsPct << ", ";
    ss << funcProjsPct << ")";

    return execute(ss.str());
}



