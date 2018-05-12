#include "handler.h"
#include <ctime>
#include <sstream>
#include <unistd.h>

Handler::Handler(repoId_t startRepo,
                 repoId_t endRepo,
                 unsigned int maxThreads,
                 unsigned int verbosity):
    m_nextRepo(startRepo),
    m_endRepo(endRepo),
    m_maxThreads(maxThreads),
    m_verbosity(verbosity),
    m_curThreads(0),
    m_threads(),
    m_isFinished(false){
    m_threads.resize(m_maxThreads);
    for(auto it = m_threads.begin(); it != m_threads.end(); ++it){
        *it = nullptr;
    }
}

void Handler::setNextRepo(unsigned int nextRepo){
    m_nextRepo = nextRepo;
}

void Handler::begin(){
    unsigned int remainingRequests, threadIdx;
    while(!m_isFinished){
        std::cout << "test1" << std::endl;
        remainingRequests = GitCrawler::getRemainingRequests();
        std::cout << "Remaining requests: " << remainingRequests << std::endl;
        if(!canSpawnThread(remainingRequests)){
            if(m_verbosity >= 1){
                std::cout << "Unable to spawn more threads. Going back to sleep. " << std::endl;
            }
            sleep(SLEEP_INTERVAL);
        } else {
            std::cout << "test2" << std::endl;
            while(canSpawnThread(remainingRequests)){
                std::cout << "spawning thread..." << std::endl;
                threadIdx = spawnThread();
                std::cout << "thread spawned, id: " << threadIdx << std::endl;
                if(m_nextRepo >= m_endRepo){
                    std::cout << "joining. " << std::endl;
                    std::cout << "thread joinable? " << m_threads[threadIdx]->joinable() << std::endl;
                    m_threads[threadIdx]->join();
                    return;
                } else {
                    std::cout << "detaching. " << std::endl;
                    m_threads[threadIdx]->detach();
                }
            }
        }
    }
}

bool Handler::canSpawnThread(unsigned int remainingRequests) const {
    return !m_isFinished && (m_curThreads < m_maxThreads) && (((m_curThreads+1) * REQUESTS_PER_REPO) < remainingRequests);
}

unsigned int Handler::spawnThread(){
    unsigned int idx = 0;
    while(idx < m_threads.size()){
        if(m_threads[idx] == nullptr){
            break;
        }
        idx++;
    }
    m_curThreads++;
    m_nextRepo += SKIP_COUNT;
    m_threads[idx] = new std::thread(&Handler::threadExec, this, idx, m_nextRepo - SKIP_COUNT);
    return idx;
}

// todo: implement start/end repo, in parseRepos.
// probably not necessary, though
void Handler::threadExec(unsigned int idx,
                         repoId_t startRepo){
    std::cout << "new thread executing, idx = " << idx << ", repo = " << startRepo << std::endl;
    GitCrawler gc(m_verbosity >= 2);
    unsigned int lastRepoParsed;

    if(m_verbosity >= 1){
        std::cout << "Beginning to parse repos starting at " << startRepo << ". " << std::endl;
    }

    lastRepoParsed = gc.parseRepos(std::move(gc.getRepos(startRepo-1)));

    if(m_verbosity >= 1){
        std::cout << "Finished parsing repos in [" << startRepo << "," << lastRepoParsed << "]. " << std::endl;
    }

    std::thread* tmp = m_threads[idx];
    m_threads[idx] = nullptr;
    m_curThreads--;
    m_isFinished = (lastRepoParsed == static_cast<unsigned int>(~0));
    delete tmp;
}

