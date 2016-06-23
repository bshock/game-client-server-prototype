#ifndef PLAYERDELETELOCK_H
#define PLAYERDELETELOCK_H
#include <mutex>
#include <condition_variable>

class PlayerDeleteLock {
public:
    PlayerDeleteLock()
    : shared()
    , readerQ(), writerQ()
    , active_readers(0), waiting_writers(0), active_writers(0)
    {}

    //infinite readers and NO writers allowed during this
    void ReadLock() {
        std::unique_lock<std::mutex> lk(shared);
        while(active_writers != 0 ) {
            readerQ.wait(lk);
        }
        ++active_readers;
        lk.unlock();
    }

    void ReadUnlock() {
        std::unique_lock<std::mutex> lk(shared);
        --active_readers;
        lk.unlock();
        writerQ.notify_one();
    }

    //only 1 writer and NO readers allowed during this
    void WriteLock() {
        std::unique_lock<std::mutex> lk(shared);
        ++waiting_writers;
        while( active_readers != 0 || active_writers != 0 ) {
            writerQ.wait(lk);
        }
        ++active_writers;
        lk.unlock();
    }

    void WriteUnlock() {
        std::unique_lock<std::mutex> lk(shared);
        --active_writers;
        lk.unlock();
        writerQ.notify_one();
        readerQ.notify_all();
    }

private:
    std::mutex              shared;
    std::condition_variable readerQ;
    std::condition_variable writerQ;
    int                     active_readers;
    int                     waiting_writers;
    int                     active_writers;
};

#endif //PLAYERDELETELOCK_H
