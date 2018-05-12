#include "gitcrawler.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Types.hpp>
#include "misc.h"
#include "config.h"
#include <ctime>
#include <string>
#include <list>
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>

// Note that these WILL cause collisions if old files arent removed.
// If time allows, use more defensive programming.
const std::string reposDumpName = "repoDump";
const std::string languagesDumpName = "languageDump";
const std::string datestampDumpName = "datestampDump";
const std::string rateLimitDumpName = "rateLimitDump";

GitCrawler::GitCrawler(bool verbose):
    m_db(),
    m_verbose(verbose){
}

GitCrawler::~GitCrawler(){
}

std::stringstream GitCrawler::getRepos(unsigned int since){
    std::string urlStr("https://api.github.com/repositories?since=");
    urlStr += std::to_string(since);

    std::stringstream ss = makeRequest(urlStr);

    fdump(ss.str(), "./repos/" + std::to_string(since));
    return ss;
}

std::stringstream GitCrawler::getLanguages(const std::string& owner,
                              const std::string& project){
    // setup the url
    // https://api.github.com/repos/$OWNER/$PROJECT/languages
    std::stringstream request;
    request << "https://api.github.com/repos/" << owner << "/" << project << "/languages";
    return makeRequest(request.str());
}

std::stringstream GitCrawler::getDatestamp(const std::string& owner,
                              const std::string& project){
    // setup the url
    // https://api.github.com/repos/$OWNER/$PROJECT
    std::stringstream ss;
    ss << "https://api.github.com/repos/" << owner << "/" << project;
    return makeRequest(ss.str());
}

std::stringstream GitCrawler::getRateLimit(){
    return makeRequest("https://api.github.com/rate_limit");
}

unsigned int GitCrawler::extractRemainingRequests(const std::string& request){
    size_t idx = request.find("core", 0);
    if(idx == std::string::npos){
        // invalid input
        std::string fname = rateLimitDumpName + std::to_string(time(NULL));
        fdump(request, fname);
        throw std::runtime_error("Failed to parse rate limits. Dumping contents to file: " + fname);
    }
    idx = request.find("remaining", idx);

    return extractDigits(request, findDigit(request, idx));
}

time_t GitCrawler::extractResetTime(const std::string& request){
    size_t idx = request.find("core", 0);
    if(idx == std::string::npos){
        // invalid input
        std::string fname = rateLimitDumpName + std::to_string(time(NULL));
        fdump(request, fname);
        throw std::runtime_error("Failed to parse rate limits. Dumping contents to file: " + fname);
    }
    idx = request.find("reset", idx);

    return extractDigits(request, findDigit(request, idx));
}

unsigned int GitCrawler::parseRepos(std::stringstream repos,
                                    unsigned int startId,
                                    unsigned int endId){

    size_t idx = 0, lastProjectId = ~0;

    size_t projectId, ownerId;
    time_t timestamp;
    std::string projectName, ownerName, repostr = repos.str(), datestamp;
    ownerName.reserve(20);
    projectName.reserve(20);
    std::stringstream languagesStream, datesStream;

    if(repostr.find("[]") != std::string::npos){
        // gone past the end of the repos. done processing.
        return ~0;
    }

    if(repostr.find("collaborator", 0) == std::string::npos){
        // collaborator was chosen because its highly unlikely to be present in any non-standard response (e.g. errors, rate limit exceeded, etc)
        std::string fname = reposDumpName + std::to_string(time(NULL));
        fdump(repos, fname);
        throw std::runtime_error("Failed to parse repos. Dumping contents to file: " + fname);
    }
    while((idx = repostr.find("\"id\"", idx)) != std::string::npos){
        ownerId = 0;
        projectId = 0;
        ownerName = "";
        projectName = "";
        languagesStream.str(std::string());
        languagesStream.clear();
        datesStream.str(std::string());
        datesStream.clear();

        // found the id str. need to find the actual digits now.
        idx = findDigit(repostr, idx);
        projectId = extractDigits(repostr, idx);
        if(projectId < startId || projectId > endId){
            if(m_verbose){
                std::cout << "ID: " << projectId << " skipped, not within range. " << std::endl;
            }
            if(projectId > endId){
                // no point continuing
                return lastProjectId;
            }
            // need to skip past the next id string (since it contains the user id for the current repo)
            idx = 20 + repostr.find("\"id\"", idx); // 20 was chosen arbitrarily. long enough to skip the id
            // not long enough to skip past the next.
        } else {
            lastProjectId = projectId;
            m_projectId = projectId;
            // now extract the project name
            idx = repostr.find("\"name\"", idx);
            idx = repostr.find(":", idx);
            idx = 1 + repostr.find("\"", idx);
            while(repostr[idx] != '\"'){
                projectName += repostr[idx++];
            }

            // extract the owner name
            idx = repostr.find("\"login\"", idx);
            idx = repostr.find(":", idx);
            idx = 1 + repostr.find("\"", idx);
            while(repostr[idx] != '\"'){
                ownerName += repostr[idx++];
            }

            // owner id is next.
            idx = repostr.find("\"id\"", idx);
            idx = repostr.find(":", idx);
            idx = findDigit(repostr, idx);

            ownerId = extractDigits(repostr, idx);

            if(m_db.projectExists(projectId, "Projects")){
                if(m_verbose){
                    std::cout << "Project " << projectId << " already exists. Skipping. " << std::endl;
                }
            } else {
                // need to get datestamp now
                datesStream = getDatestamp(ownerName, projectName);
                datestamp = datesStream.str();
                timestamp = dateToTimestamp(parseDatestamp(projectId, datestamp));

                // now add it to the database
                if(m_verbose){
                    std::cout << "Inserting project: " << projectId << ", " << ownerId << ", " << timestamp << ", "
                              << ownerName << "/" << projectName << std::endl;
                }
                m_db.insertProject(projectId, ownerId, timestamp, projectName, ownerName);
                languagesStream = getLanguages(ownerName, projectName);
                parseLanguages(projectId, std::move(languagesStream));
            }
        }
    }

    return lastProjectId;
}

void GitCrawler::parseLanguages(unsigned int projectId,
                                std::stringstream languages){
    // final language does not have a delimiting comma. instead, just keep checking until finding a non-digit
    size_t idx = 0, bytes;

    std::string lang, languagesStr = languages.str();
    lang.reserve(20); // fair to assume length will be less than this

    if( (idx = languagesStr.find("message", 0)) != std::string::npos){
        // if we found this, its not a standard response. some kind of error, like "repo blocked".
        // just add it to the table and icnlude the reason. can exclude it in data analysis
        idx = languagesStr.find(":", idx);
        idx = 1 + languagesStr.find("\"", idx);
        std::string msg("");
        msg.reserve(20);
        while(languagesStr[idx] != '\"'){
            msg += languagesStr[idx];
            idx++;
        }
        if(m_verbose){
            std::cout << "Nonstandard languages response: " << msg << std::endl;
        }
        m_db.insertLanguage(projectId, msg, 0);
        return;
    }



    while(1){
        // this should work:
        // idx = 1+languagesStr.find("\"", idx);
        // but for some reason it isnt. figure it out later, use this mess of a work around for now
        while(languagesStr[idx] != '\"'){
            idx++;
            if(idx == languagesStr.length()){
                return;
            }
        }
        idx++;
        if(idx == languagesStr.length()){
            return;
        }

        lang = "";
        bytes = 0;

        while(languagesStr[idx] != '\"'){
            lang += languagesStr[idx];
            idx++;
        }
        // got the lang str, need the byte count
        
        idx = languagesStr.find(":", idx);
        idx = findDigit(languagesStr, idx);
        bytes = extractDigits(languagesStr, idx);

        if(m_verbose){
            std::cout << "Inserting language: " << lang << ", bytes: " << bytes << std::endl;
        }

        fdump(languagesStr, "./languages/" + std::to_string(projectId));
        
        // got the byte count. insert it
        m_db.insertLanguage(projectId, lang, bytes);

        if(languagesStr.find(",", idx) == std::string::npos){
            return;
        }
    }


}


std::string GitCrawler::parseDatestamp(unsigned int projectId,
                                       const std::string& repo){
    std::string datestamp;
    fdump(repo, "./dates/" + std::to_string(projectId));
    size_t idx = repo.find("\"created_at\"", 0);
    if(idx == std::string::npos){
        std::string fname = datestampDumpName + std::to_string(time(NULL));
        fdump(repo, fname);
        throw std::runtime_error("Failed to parse datestamp for project " + std::to_string(m_projectId) + ". Dumping contents to file: " + fname);
    }
    idx = repo.find(":", idx);
    idx = findDigit(repo, idx);

    datestamp.reserve(20); // datestamp should be 20 chars long. no need for a bunch of reallocs
    while(repo[idx] != '\"'){
        datestamp += repo[idx];
        idx++;
    }
    return datestamp;
}

unsigned int GitCrawler::getRemainingRequests(){
    return extractRemainingRequests(getRateLimit().str());
}

void GitCrawler::getRates(unsigned int& remainingRequests,
                          time_t& resetTime){
    std::stringstream rateLimit = getRateLimit();
    std::string rateLimitStr = rateLimit.str();
    remainingRequests = extractRemainingRequests(rateLimitStr);
    resetTime = extractResetTime(rateLimitStr);
}


// a valid string has this form:    yyyy-MM-ddThh:mm:ssZ
// for example:                     2008-02-09T16:36:15Z
// note: g++ v <5 is missing std::get_time, so use c style code
time_t GitCrawler::dateToTimestamp(const std::string& str){
    // Some datestamps may include a timezone- for some reason, github doesnt translate them automatically
    // for now, just dont bother checking. this code will automatically disregard the timezone
    // leaving it with an error of +/-12 hours, and since the accuracy of the timestamps only matters to within
    // 1 month, this is negligible.
    std::tm t;
    std::istringstream ss(str);
    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
    return mktime(&t);
}

    /*
    char cstr[str.length()+1], delimiters[] = "-:+TZ";
    char* savePtr = cstr;
    strcpy(cstr, str.c_str());
    strtok_r(cstr, delimiters, savePtr);

    t.tm_year = atoi(strtok_r(cstr, delimiters, savePtr)) - 1900;
    t.tm_mon = tok.getInt() - 1;
    t.tm_mday = tok.getInt();
    t.tm_hour = tok.getInt();
    t.tm_min = tok.getInt();
    t.tm_sec = tok.getInt();
    t.tm_isdst = -1; // unsure if GMT uses DST.
    */

    /*
    Tokenizer tok(str, "-:+TZ");

    // Let the errors caused by tokenizer propagate, assuming the datestamp is invalid
    t.tm_year = tok.getInt() - 1900;
    t.tm_mon = tok.getInt() - 1;
    t.tm_mday = tok.getInt();
    t.tm_hour = tok.getInt();
    t.tm_min = tok.getInt();
    t.tm_sec = tok.getInt();
    t.tm_isdst = -1; // unsure if GMT uses DST.
    */

   // return mktime(&t);

std::stringstream GitCrawler::makeRequest(const std::string& urlStr){
    // no point in recreating this string every call, unless we allow reading a new config file
    const static std::string userAgentStr = "User-Agent: " + Config::userAgent();
    const static std::string authTokenStr = "Authorization: token " + Config::authToken();
    std::stringstream result;

    curlpp::Cleanup cleaner;
    curlpp::Easy request;
    curlpp::options::Url url(urlStr);
    curlpp::options::WriteStream ws(&result);

    // setup the header
    std::list<std::string> header;
    header.push_back(userAgentStr);
    header.push_back(authTokenStr);


    request.setOpt(new curlpp::options::Verbose(false));
    request.setOpt(new curlpp::options::HttpHeader(header));
    request.setOpt(url);
    request.setOpt(ws);


    request.perform();

    return result;
}
