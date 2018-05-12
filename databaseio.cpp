#include "databaseio.h"
#include "config.h"
#include "misc.h"
#include "cppconn/prepared_statement.h"
#include <sstream>
#include <memory>
#include <iomanip>

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

bool DatabaseIo::selectDatabase(const std::string& database){
    try {
        return m_stmt->execute("USE " + database);
    } catch(...){
        return true;
    }
}


// returns true on any error
bool DatabaseIo::insertProject(unsigned int id,
                               unsigned int owner_id,
                               time_t timestamp,
                               const std::string& project_name,
                               const std::string& owner_name){
    std::stringstream ss;
    ss << "INSERT INTO Projects VALUES(" <<
          id << ", " <<
          owner_id << ", " <<
          timestamp << ", \"" <<
          project_name << "\", \"" <<
          owner_name << "\")";
    return execute(ss.str().c_str());
}

// returns true on any error
bool DatabaseIo::insertLanguage(unsigned int id,
                                const std::string& language,
                                unsigned int bytes){
    std::stringstream ss;
    ss << "INSERT INTO Languages VALUES(";
    ss << id << ", \"";
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


bool DatabaseIo::projectExists(unsigned int projectId,
                               const std::string& table){
    std::stringstream ss;
    try {
        ss << "SELECT ProjectId from " << table << " where ProjectId=" << projectId;
        sql::ResultSet* res = m_stmt->executeQuery(ss.str());
        return res->next();
    } catch(...){
        return false;
    }
}

void DatabaseIo::deleteProject(unsigned int projectId){
    execute("delete from Languages where ProjectId=" + std::to_string(projectId));
    execute("delete from Projects where ProjectId=" + std::to_string(projectId));
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
                                  long double partFuncProjsPct,
                                  long double funcProjsPct){
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

