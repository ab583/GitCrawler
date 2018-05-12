#ifndef HANDLER_H
#define HANDLER_H
#include "gitcrawler.h"
#include "databaseio.h"
#include "misc.h"
#include <vector>
#include <thread>
#include <memory>

// Essentially this class is a threadpool.
// Note: each thread uses approx. 60kBit/s down, 10kBit/s up
// 3 threads is sufficient to max out the 5000 requests/hour
// has automated rate-limiting to avoid hitting this limit
class Handler
{
public:
    Handler(repoId_t startRepo = 0,
            repoId_t endRepo = ~0,
            unsigned int maxThreads = 1,
            unsigned int verbosity = 0);

    void setNextRepo(unsigned int nextRepo);
    void begin();

private:
    bool canSpawnThread(unsigned int remainingRequests) const;
    unsigned int spawnThread(); // always call canSpawnThread before this. Returns the thread id.
    void threadExec(unsigned int idx,
                     repoId_t startRepo);

    repoId_t m_nextRepo;
    repoId_t m_endRepo;
    unsigned int m_maxThreads;
    unsigned int m_verbosity; // [0,2] valid options. >2 has same effect as 2. higher = more printing messages


    unsigned int m_curThreads;
    std::vector<std::thread*> m_threads; // some elements may be null due to thread sequencing.
    bool m_isFinished; // set to true when reached end of repos OR had major error.


    static const unsigned int REQUESTS_PER_REPO = 202;      // 100 language requests, 100 date requests, 1 repo request, 1 spare
    static const time_t SLEEP_INTERVAL = 30;   // time in seconds before re-checking rate limit, thread count, etc, possibly creating more
    static const unsigned int SKIP_COUNT = 100000;          // samples first 100 out of every SKIP_COUNT repo **IDs** (IDs can be vacated)
};

#endif // HANDLER_H
