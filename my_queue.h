
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

    QAtomicPointer<QQueue<T>> q = new QQueue<T>;
    QAtomicInt producers_num = 0;
    QAtomicInt is_notified = 0;

public:
    void set_producers_number(int num){
        producers_num = num;
    }
    void finish(){
        producers_num--;
        is_notified = 1;
        qcv.wakeAll();
    }

    void push(T element){
        q->enqueue(element);
        is_notified = 1;
        qcv.wakeOne();
    }

    bool can_try_pop(int n){
        return (producers_num > 0) || (q->size() >= n);
    }

    QVector<T> pop(int n){
        QVector<T> some;
        while ((producers_num > 0) || (q->size() >= n)) {
            while(!is_notified && (q->size() < n)){
                qcv.wait(&mtx);
            }
            while(q->size() >= n) {
                while (n > 0) {
                    some.push_back(q->front());
                    q->pop();
                    n--;
                }
            }
            is_notified = false;
        }
        return some;
    }
};


#endif //COUNTING_WORDS_3_MY_QUEUE_H
