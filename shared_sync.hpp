#pragma once
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

using namespace boost::interprocess;

struct SyncBlock {
    interprocess_mutex mutex;
    interprocess_semaphore sem{0};
    int arrived = 0;

    void arrive_and_wait(int total) {
        bool is_last = false;

        {
            scoped_lock<interprocess_mutex> lock(mutex);
            arrived++;
            if (arrived == total) {
                is_last = true;
                for (int i = 0; i < total; ++i)
                    sem.post(); // release all
            }
        }

        sem.wait();
    }

    void reset() {
        scoped_lock<interprocess_mutex> lock(mutex);
        arrived = 0;
    }
};

inline SyncBlock* get_sync_block() {
    static managed_shared_memory segment(open_or_create, "SharedSync", 65536);
    return segment.find_or_construct<SyncBlock>("sync_block")();
}
