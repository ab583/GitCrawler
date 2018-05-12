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
    m_isFinished(false){
}

void Handler::setNextRepo(unsigned int nextRepo){
    m_nextRepo = nextRepo;
}

void Handler::begin(){
    unsigned int remainingRequests;
    while(!m_isFinished){
        remainingRequests = GitCrawler::getRemainingRequests();
        if(!canSpawnThread(remainingRequests)){
            if(m_verbosity >= 1){
                std::cout << "Unable to spawn more threads. Going back to sleep. " << std::endl;
            }
            sleep(SLEEP_INTERVAL);
        } else {
            while(canSpawnThread(remainingRequests)){
                m_curThreads++;
                m_nextRepo += SKIP_COUNT;
                std::thread thread(&Handler::threadExec, this, m_nextRepo - SKIP_COUNT);
                if(m_nextRepo >= m_endRepo){
                    thread.join();
                    return;
                } else {
                    thread.detach();
                }
            }
        }
    }
}

bool Handler::canSpawnThread(unsigned int remainingRequests) const {
    return !m_isFinished && (m_curThreads < m_maxThreads) && (((m_curThreads+1) * REQUESTS_PER_REPO) < remainingRequests);
}

// todo: implement start/end repo, in parseRepos.
// probably not necessary, though
void Handler::threadExec(repoId_t startRepo){
    std::cout << "new thread executing, repo = " << startRepo << std::endl;
    GitCrawler gc(m_verbosity >= 2);
    unsigned int lastRepoParsed;

    if(m_verbosity >= 1){
        std::cout << "Beginning to parse repos starting at " << startRepo << ". " << std::endl;
    }

    lastRepoParsed = gc.parseRepos(std::move(gc.getRepos(startRepo-1)));

    if(m_verbosity >= 1){
        std::cout << "Finished parsing repos in [" << startRepo << "," << lastRepoParsed << "]. " << std::endl;
    }

    m_curThreads--;
    m_isFinished = (lastRepoParsed == static_cast<unsigned int>(~0)); // parseRepos returns ~0 if its run past the end of repos
}

