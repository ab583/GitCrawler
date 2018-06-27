/**
 *  requires/depends on:
 *      mysql connector
 *      boost
 *      curlpp
 *      curl
 *
 * Note that a config file, "config.txt", must be created. It must contain, in this order, with no other text*
 *      User-Agent
 *      OAuth token
 *      Database Host
 *      Database Port
 *      Database Username
 *      Database Password
 *      Database Name
 *
 * *Empty lines, and lines beginning with '#' are ignored.
 *
 *  TODO:
 *  -Check status of mysql statements.
 *  -Finish implementing end repos. Currently will continue iterating over ALL repos
 *  -Allow greater control for step count, and specifying sequential iteration
 *  -Redesign end conditions for thread execution. Currently the way of checking if we've gone past the final repo, and if
 *      we've gone past the designated end (ie. last repo user wants to parse) is a mess, and in fact could result in a
 *      bug when the last created thread terminates before a thread slightly before it, due to the "creative" use of detach/join.
 *
 * -- test new dateToTimestamp, add handling for timezones
 *
 *
 *  Start at:
 *      --re-test config file parsing. test variations of valid cfg files.
 *      --consider changing to a generic procedure for usage with JSON parsing.
 *          pros: interesting challenge, would make for a very neat implementation, cons: gotta deal with nested attributes.
 */

#include "databaseio.h"
#include "gitcrawler.h"
#include "misc.h"
#include "handler.h"
#include "config.h"
#include "statistics.h"
#include "boost/filesystem.hpp"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>


// calculates statistics based on data in GithubProjects database
// specifically, Projects and Languages tables
// inserts them into the database table Statistics.
// threads not currently implemented, but statistics class is already thread-safe
void calculateStatistics(int argc, const char* argv[]);



int main(int argc, const char* argv[]){
    unsigned int verbosity = 2;
    try {
    Config::readFile("config.txt");
    } catch (std::exception& e){
        if(verbosity >= 1){
            std::cout << "Failed to open config file. Creating new one, please open config.txt and enter the correct values. " << std::endl;
        }
        Config::createConfig();
        return 1;
    }

    // create folders if they dont exist
    boost::filesystem::create_directories("repositories");
    boost::filesystem::create_directories("languages");
    boost::filesystem::create_directories("dates");

    DatabaseIo db;


    // create database if it doesnt exist
    db.createDatabase();

    // Note: repos start at 1.
    Handler h(1, 5000, 3, 1);
    h.begin();



    return 0;
}

void calculateStatistics(){
    Statistics::init();
    Statistics s;


    // repos have ids in range [1, 94m]
    for(std::size_t i = 0; i < 94; ++i){
        std::cout << "Beginning to process projects in interval [" << i*1000000 << "," << (i+1)*1000000 << "] " << std::endl;
        s.processProjects(i*1000000, (i+1)*1000000);
    }

    std::cout << "Computing derived statistics. " << std::endl;

    s.computeDerivedStatistics();

    s.printTotals(std::cout);
    std::cout << "Inserting into database. " << std::endl;
    s.insertDb();
}


