#pragma once  // Ensures this header is only included once during compilation
#include <boost/interprocess/managed_shared_memory.hpp>     // Shared memory segment creation
#include <boost/interprocess/sync/interprocess_semaphore.hpp>  // Named semaphore for synchronization
#include <boost/interprocess/sync/interprocess_mutex.hpp>      // Mutex for critical sections
#include <boost/interprocess/sync/scoped_lock.hpp>             // RAII-style lock for mutex
using namespace boost::interprocess;

// Structure for synchronizing multiple parties in shared memory
struct SyncBlock {
    interprocess_mutex mutex;       // Mutual exclusion for critical updates
    interprocess_semaphore sem{0};  // Semaphore to signal all parties when ready
    int arrived = 0;                // Number of parties that have arrived

    // Blocks all parties until `total` have arrived, then releases all
    void arrive_and_wait(int total) {
        bool is_last = false;
        {
            scoped_lock<interprocess_mutex> lock(mutex);  // Safely increment arrival count
            arrived++;
            if (arrived == total) {
                is_last = true;
                for (int i = 0; i < total; ++i)
                    sem.post();  // Wake up all waiting threads
            }
        }
        sem.wait();  // Wait until signaled
    }
    // Resets arrival counter (not always used, but useful for cleanup or reruns)
    void reset() {
        scoped_lock<interprocess_mutex> lock(mutex);
        arrived = 0;
    }
};

// Returns a pointer to a shared `SyncBlock` object from shared memory
inline SyncBlock* get_sync_block() {
    static managed_shared_memory segment(open_or_create, "SharedSync", 65536);  // 64KB shared memory segment
    return segment.find_or_construct<SyncBlock>("sync_block")();  // Find or create the named SyncBlock
}
