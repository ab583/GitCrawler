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
 *  -Improve file parsing for config file.
 *  -Finish implementing end repos. Currently will continue iterating over ALL repos
 *  -Allow greater control for step count, and specifying sequential iteration
 *  -Redesign end conditions for thread execution. Currently the way of checking if we've gone past the final repo, and if
 *      we've gone past the designated end (ie. last repo user wants to parse) is a mess, and in fact could result in a
 *      bug when the last created thread terminates before a thread slightly before it, due to the "creative" use of detach/join.
 *
 * -- test new dateToTimestamp, add handling for timezones
 *
 */

#include "databaseio.h"
#include "gitcrawler.h"
#include "misc.h"
#include "handler.h"
#include "config.h"
#include "statistics.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <cstdlib> // atoi
#include <unistd.h> // usleep
#include "tokenizer.h"


void printRemainingRequests();

// gets all repos in interval [argv[1], argv[2]], parses them, adds them to the database
int getParseRepos(int argc, const char* argv[]);

// calculates the statistics based on data obtained by getParseRepos
// and inserts them into the database table Statistics.
// takes params startId, endId
// threads not currently implemented, but statistics class is already thread-safe
void calculateStatistics(int argc, const char* argv[]);

int main(int argc, const char* argv[]){
    //getParseRepos(argc, argv);

    Statistics::init();
    Config::readFile("config.txt");

    Statistics::init();
    calculateStatistics(argc, argv);

    return 0;

}
/*
 *
    void processProjects(unsigned int beginId,
                         unsigned int endId);


    // computes derived statistcs such as functional projects as a percentage of all projects
    // always call this AFTER reducing instances of this class
    void computeDerivedStatistics();


    // inserts all statistics into db.
    void insertDb();

    static void reduce(Statistics& dst,
                       const Statistics& src);
                       */

void calculateStatistics(int argc, const char *argv[]){
    Statistics s;
    //s.processProjects(0, 20000000);

    // repos have ids in range [1, 94m]
    for(std::size_t i = 0; i < 94; ++i){
        std::cout << "Beginning to process projects in interval [" << i*1000000 << "," << (i+1)*1000000 << "] " << std::endl;
        s.processProjects(i*1000000, (i+1)*1000000);
    }

    std::cout << "Computing derived statistics. " << std::endl;

    s.computeDerivedStatistics();

    s.printTotals(std::cout);
    //std::cout << "Inserting into database. " << std::endl;
    //s.insertDb();
}



int getParseRepos(int argc, const char* argv[]){
    assert(sizeof(unsigned int) >= 4); // Must be at least 32 bits to ensure its able to encode any given id
    if(argc != 3){
        std::cout << "Error: must pass the starting repo, end repo, and no other parameters at command line. " << std::endl;
        return 1;
    }

    Config::readFile("config.txt");
    Handler h(atoi(argv[1]), atoi(argv[2]), 3, 2);
    h.begin();

    return 0;
}

