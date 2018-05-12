#ifndef STATISTICS_H
#define STATISTICS_H
#include "databaseio.h"
#include <vector>
#include <string>
#include <memory>
#include <ios>

template<typename T>
using vec2d = std::vector<std::vector<T>>;

// repos...
// begin at 2007-10-29
// end at 2017-06-01
class Statistics
{
    enum class ProjectType {
        Nonfunc,
        PartFunc,
        Func,
        Empty,
        Blocked,
    };

public:
    Statistics();


    void processProjects(unsigned int beginId,
                         unsigned int endId);



    // computes derived statistcs such as functional projects as a percentage of all projects
    // always call this AFTER reducing instances of this class
    void computeDerivedStatistics();


    // inserts all statistics into db.
    void insertDb();

    static void reduce(Statistics& dst,
                       const Statistics& src);


    bool isFuncLang(const std::string& lang);
    void printTotals(std::ostream& os); // prints blocked projects, empty projects, project count
    static void init();
private:
    DatabaseIo m_db;
    unsigned int m_blockedProjs; // coutn of blocked repos (e.g. cant access languages due to DMCA)
    unsigned int m_emptyProjs; // count of repos without any languages associated with them.
    unsigned int m_projCount;


    // how many projects with func languages used sorted by year/month, beginning in 2007, jan.
    // e.g. [0][1] is how many projects which use at least one functional language in 2007, feb
    vec2d<unsigned int> m_partFuncProjs;
    // same as above, except purely func languages
    vec2d<unsigned int> m_funcProjs;
    // just a count of how many projects were created in this month. same indexing scheme
    vec2d<unsigned int> m_projectCounts;

    // partially functional projects as a percentage of all projects per month
    // only computed by calling computeDerivedStatistics()
    std::unique_ptr<vec2d<double>> m_partFuncProjsPct;

    // fully functional projects as a percentage of all projects per month
    // only computed by calling computeDerivedStatistics()
    std::unique_ptr<vec2d<double>> m_funcProjsPct;

    // static std::vector<std::string> m_languages;
    static std::vector<std::string> m_funcLanguages;

    ProjectType getProjectType(unsigned int projectId);

    Statistics(const Statistics&) = delete;
    Statistics(Statistics&&) = delete;
};



#endif // STATISTICS_H
