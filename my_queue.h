
#ifndef COUNTING_WORDS_3_MY_QUEUE_H
#define COUNTING_WORDS_3_MY_QUEUE_H

#include <iostream>
#include <mutex>
#include <queue>
#include <QMutex>
#include <QtCore/QWaitCondition>


template<typename T>
class MyQueue{

private:
    QMutex mtx;
    QWaitCondition qcv;

    QQueue<T> *q = new QQueue<T>;
    int producers_num = 0;
    bool notified = false;

public:
    void set_producers_number(int num){
        producers_num = num;
    }
    void finish(){
        mtx.lock();
        producers_num--;
        notified = true;
        qcv.wakeAll();
        mtx.unlock();
    }

    void push(T element){
        mtx.lock();
        q->enqueue(element);
        notified = true;
        qcv.wakeOne();
        mtx.unlock();
    }

    bool can_try_pop(int n){
        return (producers_num > 0) || (q->size() >= n);
    }

    std::vector<T> pop(int n){
        std::vector<T> some;
        std::unique_lock<std::mutex> lock(mtx);
        while ((producers_num > 0) || (q->size() >= n)) {
            while(!notified && (q->size() < n)){
                cv.wait(lock);
            }
            while(q->size() >= n) {
                while (n > 0) {
                    some.emplace_back(q->front());
                    q->pop();
                    n--;
                }
                return some;
            }
            notified = false;
        }
        return some;
    }
};


#endif //COUNTING_WORDS_3_MY_QUEUE_H
