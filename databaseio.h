#ifndef DATABASEIO_H
#define DATABASEIO_H
#include "misc.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include "cppconn/statement.h"
#include "cppconn/resultset.h"
#include <string>
#include <memory>
#include <cstdint>
#include <sstream>

// Note: prepared statements are causing a MethodNotImplementedException.
// Until mysql_cppconnector is updated, will have to prepare them here
class DatabaseIo {
public:
    typedef std::shared_ptr<sql::ResultSet> queryResult_t;

    DatabaseIo();
    ~DatabaseIo();

    void createDatabase();

    bool selectDatabase(const std::string& database);

    // adds the project values to a batch job
    void addProject(repoId_t repoId,
                    userId_t ownerId,
                    time_t timestamp,
                    const std::string& projectName,
                    const std::string& ownerName);

    // commits all queued projects in batch job
    bool commitProjects();

    // adds the language values to a batch job
    void addLanguage(repoId_t repoId,
                     const std::string& language,
                     uint32_t bytes); // its seriously unlikely any given language is going to have >4GB of source. would require tens of millions (very conservative estimate) to billions of LOC

    // commits all queued languages in batch job
    bool commitLanguages();

    bool projectExists(repoId_t repoId,
                       const std::string& table);

    void deleteProject(repoId_t repoId); // deletes languages as well. fails silently.

    // Catches any exceptions, simply returns true if the statement failed.
    bool execute(const std::string& statement);


    queryResult_t executeQuery(const std::string& query);

    std::uint64_t getUniqueLanguagesCount();
    queryResult_t getUniqueLanguages(); // returns a list of all unique languages

    bool insertStatistics(const std::string& date,
                          unsigned int projectCount,
                          unsigned int partFuncProjs,
                          unsigned int funcProjs,
                          double partFuncProjsPct,
                          double funcProjsPct);


private:
    sql::mysql::MySQL_Driver* m_driver; // doesnt take ownership, raw is fine
    std::unique_ptr<sql::Connection> m_con;
    std::unique_ptr<sql::Statement> m_stmt;
    std::unique_ptr<std::stringstream> m_projectsBatch;
    std::unique_ptr<std::stringstream> m_languagesBatch;

    // DB conns are not move or copy constructible.
    DatabaseIo(const DatabaseIo& src) = delete;
    DatabaseIo(DatabaseIo&& src) = delete;
};




#endif // DATABASEIO_H


