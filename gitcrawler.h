#ifndef GITCRAWLER_H
#define GITCRAWLER_H
#include "databaseio.h"
#include <curlpp/Easy.hpp>
#include <string>
#include <memory>
#include <cstdint>


// error handling is incomplete
class GitCrawler
{
public:
    GitCrawler(bool verbose = false);
    ~GitCrawler();

    // Gets next 100 repos at or after <since>
    std::stringstream getRepos(repoId_t since);

    // Gets all languages used by the project
    std::stringstream getLanguages(const std::string& owner,
                                   const std::string& project);

    // Gets the datestamp for a given project
    std::stringstream getDatestamp(const std::string& owner,
                                   const std::string& project);

    // Parses the output from getRepos, returns the id of the last repo parsed (ascending order),
    // adds the repos to the database
    // only processes projects where startId <= projectId <= endId
    // returns ~0 when past final repo
    repoId_t parseRepos(std::stringstream repos,
                        repoId_t startId = 0,
                        repoId_t endId = ~0);

    // Parses the output from getLanguages, inserts the languages into the database
    void parseLanguages(repoId_t projectId,
                        std::stringstream languages);

    // interprets the data returned from getDatestamp, returns solely the date stamp itself
    // important: note that, unlike parseRepos and parseLanguages, this returns the datestamp
    // while a consistent interface would be preferable, it isnt an option due to table structure within the database
    std::string parseDatestamp(repoId_t projectId,
                               const std::string &repo);


    static unsigned int getRemainingRequests();
    void getRates(unsigned int& remainingRequests,
                  time_t& resetTime);

    // form: yyyy-MM-ddThh:mm:ssZ
    // expl: 2008-02-09T16:36:15Z
    // note: timezone conversion not performed, times are therefore only accurate +/-12hr
    time_t dateToTimestamp(const std::string& str);

private:
    DatabaseIo m_db;
    bool m_verbose;

    unsigned int m_projectId; // ill figure out a cleaner solution later

    // These are used internally for rate control
    static std::stringstream getRateLimit();
    static unsigned int extractRemainingRequests(const std::string& request);
    time_t extractResetTime(const std::string& request);

    // automatically specifies user-agent, authentication token, and sets up a few common parameters.
    static std::stringstream makeRequest(const std::string& urlStr);

    GitCrawler(const GitCrawler& src) = delete;
    GitCrawler(GitCrawler&& src) = delete;
};

#endif // GITCRAWLER_H
