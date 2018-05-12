#ifndef GITCRAWLER_H
#define GITCRAWLER_H
#include "databaseio.h"
#include <curlpp/Easy.hpp>
#include <string>
#include <memory>


// error handling is incomplete
class GitCrawler
{
public:
    GitCrawler(bool verbose = false);
    ~GitCrawler();



    // Gets all repos at or after
    // output is inserted into the stream, not returned.
    static void getRepos(unsigned int since, std::ostream &output);

    // Parses the output from getRepos, returns the id of the last repo parsed (ascending order),
    // adds the repos to the database
    // only processes projects where startId <= projectId <= endId
    unsigned int parseRepos(std::stringstream& repos,
                            unsigned int startId = 0,
                            unsigned int endId = ~0);


    // Gets all languages used by the project
    static void getLanguages(const std::string& owner,
                             const std::string& project,
                             std::ostream& output);

    // Parses the output from getLanguages, inserts the languages into the database
    void parseLanguages(unsigned int projectId,
                        std::stringstream& languages);


    // Gets the datestamp for a given project
    static void getDatestamp(const std::string& owner,
                             const std::string& project,
                             std::ostream& output);

    // interprets the data returned from getDatestamp, returns solely the date stamp itself
    // important: note that, unlike parseRepos and parseLanguages, this returns the datestamp
    // while a consistent interface would be preferable, it isnt an option due to table structure within the database
    std::string parseDatestamp(unsigned int projectId,
                               const std::string &repo);


    static unsigned int getRemainingRequests();
    static void getRates(unsigned int& remainingRequests,
                         time_t& resetTime);

    // form: yyyy-MM-ddThh:mm:ssZ
    // expl: 2008-02-09T16:36:15Z
    // note: timezone conversion not performed, times are therefore only accurate +/-12hr
    static time_t dateToTimestamp(const std::string& str);

    static void setUserAgent(const std::string& userAgent);
    static void setAuthToken(const std::string& authToken);

private:
    DatabaseIo m_db;
    bool m_verbose;
    GitCrawler(const GitCrawler& src) = delete;
    GitCrawler(GitCrawler&& src) = delete;

    unsigned int m_projectId; // ill figure out a cleaner solution later

    // These are used internally for rate control
    static void getRateLimit(std::ostream& output);
    static unsigned int extractRemainingRequests(const std::string& request);
    static time_t extractResetTime(const std::string& request);

    // automatically specifies user-agent, authentication token, and sets up a few common parameters.
    static void makeRequest(const std::string& urlStr, std::ostream& output);
};

#endif // GITCRAWLER_H
