#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <list>
#include <functional>
#include <map>
#include <queue>

#include <thread>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentMap>
#include <QtCore>

#include "time_measure.h"
#include "config.h"
#include "additional.h"
#include "my_queue.h"


MyQueue<QString> mq_str;
MyQueue< std::map<QString, QAtomicInt> > mq_map;

QWaitCondition queueNotEmpty;
QMutex mtx;


class Indexer : public QThread
{
public:
    Indexer(QObject *parent = nullptr) : QThread(parent) {}

    void run() override
    {
        while (mq_str.can_try_pop(1)){
            mtx.lock();
            auto content = mq_str.pop(1);
            queueNotEmpty.wakeOne();
            mtx.unlock();

        }
    }
};


int main(int argc, char *argv[])
{
    std::cout << "\nOK\n" << std::endl;
    return 0;
}