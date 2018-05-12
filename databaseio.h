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


// Note: prepared statements are causing a MethodNotImplementedException.
// Until mysql_cppconnector is updated, will have to prepare them here
class DatabaseIo
{
public:
    typedef std::shared_ptr<sql::ResultSet> queryResult_t;

    DatabaseIo();
    ~DatabaseIo();

    void createDatabase();

    bool selectDatabase(const std::string& database);

    // returns true on any error
    bool insertProject(repoId_t repoId,
                       userId_t ownerId,
                       time_t timestamp,
                       const std::string& projectName,
                       const std::string& ownerName);

    // returns true on any error
    bool insertLanguage(repoId_t repoId,
                        const std::string& language,
                        uint32_t bytes); // its seriously unlikely the code will be >4GB


    bool projectExists(repoId_t repoId,
                       const std::string& table);

    void deleteProject(repoId_t repoId); // deletes languages as well. fails silently.

    // Catches any exceptions, simply returns true if the statement failed.
    bool execute(const std::string& statement);

    // Technically this should be a unique ptr, but that introduces issues with returning it
    // I think it can be remedied with std::move, check this later.
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
    std::shared_ptr<sql::Connection> m_con;
    std::shared_ptr<sql::Statement> m_stmt;

    // DB conns are not move or copy constructible.
    DatabaseIo(const DatabaseIo& src) = delete;
    DatabaseIo(DatabaseIo&& src) = delete;
};




#endif // DATABASEIO_H


