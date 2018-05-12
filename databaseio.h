#ifndef DATABASEIO_H
#define DATABASEIO_H
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

    bool selectDatabase(const std::string& database);

    // returns true on any error
    bool insertProject(unsigned int id,
                        unsigned int owner_id,
                        time_t timestamp,
                        const std::string& project_name,
                        const std::string& owner_name);

    // returns true on any error
    bool insertLanguage(unsigned int id,
                         const std::string& language,
                         unsigned int bytes);


    bool projectExists(unsigned int projectId,
                       const std::string& table);

    void deleteProject(unsigned int projectId); // deletes languages as well. fails silently.

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
                          long double partFuncProjsPct,
                          long double funcProjsPct);


private:
    sql::mysql::MySQL_Driver* m_driver; // doesnt take ownership, raw is fine
    std::shared_ptr<sql::Connection> m_con;
    std::shared_ptr<sql::Statement> m_stmt;

    // DB conns are not move or copy constructible.
    DatabaseIo(const DatabaseIo& src) = delete;
    DatabaseIo(DatabaseIo&& src) = delete;
};




#endif // DATABASEIO_H


