#include "statistics.h"
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>

std::vector<std::string> Statistics::m_funcLanguages;

inline double percent(double fst, double snd){
    if(!snd){
        return 0;
    }
    return 100 * (fst / snd);
}

Statistics::Statistics():
    m_db(),
    m_blockedProjs(0),
    m_emptyProjs(0),
    m_projCount(0),
    m_partFuncProjs(),
    m_funcProjs(),
    m_projectCounts(),
    m_partFuncProjsPct(),
    m_funcProjsPct(){

    // [2007,2017] ==> 11 for first dimension. 12 months for 2nd dimension
    // bad things will happen if these sizes change outside of the constructor.
    m_projectCounts.resize(11);
    for(auto it = m_projectCounts.begin(); it != m_projectCounts.end(); ++it){
        it->resize(12);
    }

    m_partFuncProjs.resize(11);
    for(auto it = m_partFuncProjs.begin(); it != m_partFuncProjs.end(); ++it){
        it->resize(12);
    }

    m_funcProjs.resize(11);
    for(auto it = m_funcProjs.begin(); it != m_funcProjs.end(); ++it){
        it->resize(12);
    }
}

void Statistics::processProjects(unsigned int beginId,
                                 unsigned int endId){
    std::stringstream ss;
    ss << "SELECT ProjectId as Id, ";
    ss << "EXTRACT(YEAR from FROM_UNIXTIME(Timestamp)) as Year, ";
    ss << "EXTRACT(MONTH from FROM_UNIXTIME(Timestamp)) as Month ";
    ss << "FROM Projects WHERE ProjectId>=" << beginId;
    ss << " AND ProjectId<=" << endId;
    DatabaseIo::queryResult_t res = m_db.executeQuery(ss.str());

    // year is in [2007, 2017] ==> bias of 2007
    // month is in [1,12] ==> bias of 1
    unsigned int id, year, month;
    ProjectType type;
    while(res->next()){
        m_projCount++;
        id = res->getUInt(1);
        year = res->getUInt(2);
        month = res->getUInt(3);
        type = getProjectType(id);
        //std::cout << "processing project: " << id << ", " << year << ", " << month << ": ";
        switch(type){
        case ProjectType::Blocked:
            //std::cout << " blocked." << std::endl;
            m_blockedProjs++;
            break;
        case ProjectType::Empty:
            //std::cout << " empty." << std::endl;
            m_emptyProjs++;
            break;
        case ProjectType::Func:
            //std::cout << " func." << std::endl;
            m_funcProjs[year-2007][month-1]++;
            m_projectCounts[year-2007][month-1]++;
            break;
        case ProjectType::Nonfunc:
            //std::cout << " nonfunc." << std::endl;
            m_projectCounts[year-2007][month-1]++;
            break;
        case ProjectType::PartFunc:
            //std::cout << " partfunc." << std::endl;
            m_partFuncProjs[year-2007][month-1]++;
            m_projectCounts[year-2007][month-1]++;
            break;
        }
    }
}

void Statistics::computeDerivedStatistics(){
    // no make_unique untill C++14
    m_partFuncProjsPct = std::unique_ptr<vec2d<double>>(new vec2d<double>());
    m_funcProjsPct = std::unique_ptr<vec2d<double>>(new vec2d<double>());

    m_partFuncProjsPct->resize(11);
    for(auto it = m_partFuncProjsPct->begin(); it != m_partFuncProjsPct->end(); ++it){
        it->resize(12);
    }

    m_funcProjsPct->resize(11);
    for(auto it = m_funcProjsPct->begin(); it != m_funcProjsPct->end(); ++it){
        it->resize(12);
    }

    for(std::size_t i = 0; i < m_projectCounts.size(); ++i){
        for(std::size_t j = 0; j < m_projectCounts[0].size(); ++j){
            (*m_partFuncProjsPct)[i][j] = percent(m_partFuncProjs[i][j], m_projectCounts[i][j]);
            (*m_funcProjsPct)[i][j] = percent(m_funcProjs[i][j], m_projectCounts[i][j]);
        }
    }
}

void Statistics::insertDb(){
    std::string date;
    unsigned int projectCount, partFuncProjs, funcProjs;
    double partFuncProjsPct, funcProjsPct;
    // i = year offset, j = month offset
    for(std::size_t i = 0; i < m_projectCounts.size(); ++i){
        for(std::size_t j = 0; j < m_projectCounts[0].size(); ++j){
            date = std::to_string(j + 1).append("-").append(std::to_string(i + 2007));
            projectCount = m_projectCounts[i][j];
            partFuncProjs = m_partFuncProjs[i][j];
            funcProjs = m_funcProjs[i][j];
            partFuncProjsPct = (*m_partFuncProjsPct)[i][j];
            funcProjsPct = (*m_funcProjsPct)[i][j];
            m_db.insertStatistics(date, projectCount, partFuncProjs, funcProjs, partFuncProjsPct, funcProjsPct);
        }
    }
}

void Statistics::reduce(Statistics& dst,
                        const Statistics& src){
    dst.m_blockedProjs += src.m_blockedProjs;
    dst.m_emptyProjs += src.m_emptyProjs;
    for(std::size_t i = 0; i < dst.m_projectCounts.size(); ++i){
        for(std::size_t j = 0; j < dst.m_projectCounts[i].size(); ++j){
            dst.m_projectCounts[i][j] += src.m_projectCounts[i][j];
            dst.m_partFuncProjs[i][j] += src.m_partFuncProjs[i][j];
            dst.m_funcProjs[i][j] += src.m_funcProjs[i][j];
        }
    }
}

bool Statistics::isFuncLang(const std::string& lang){
    return (std::find(m_funcLanguages.begin(), m_funcLanguages.end(), lang) != m_funcLanguages.end());
}

void Statistics::printTotals(std::ostream& os){ // prints blocked projects, empty projects, project count
    os << "Blocked Projects: " << m_blockedProjs << '\n';
    os << "Empty Projects: " << m_emptyProjs << '\n';
    os << "Project Count: " << m_projCount << std::endl;
}

void Statistics::init(){
    std::ifstream funcLangs("../funcLangs.txt");
    if(!funcLangs){
        throw std::runtime_error("Failed to open language files.");
    }
    std::string line;

    while(std::getline(funcLangs, line)){
        m_funcLanguages.push_back(line);
    }
}

Statistics::ProjectType Statistics::getProjectType(unsigned int projectId){
    DatabaseIo::queryResult_t res = m_db.executeQuery("SELECT Language FROM Languages WHERE ProjectId=" + std::to_string(projectId));
    ProjectType type;
    bool tmp;
    if(!res->next()){
        return ProjectType::Empty;
    } else {
        std::string line = res->getString(1);
        if(line == "Repository access bl"){
            return ProjectType::Blocked;
        } else {
            type = isFuncLang(line) ? ProjectType::Func : ProjectType::Nonfunc;
        }
        while(res->next()){
            line = res->getString(1);
            tmp = isFuncLang(line);
            if((tmp && (type == ProjectType::Nonfunc)) || (!tmp && (type == ProjectType::Func))){
                // i think this can be reduced to !(tmp ^ (type == ProjectType::Nonfunc))
                // too tired to be certain. think it over in morning.
                return ProjectType::PartFunc;
            }
        }
        return type;
    }
}
