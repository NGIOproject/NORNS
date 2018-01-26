/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#ifndef __THREAD_POOL_QUEUE_HPP__
#define __THREAD_POOL_QUEUE_HPP__

#include <atomic>  
#include <mutex>
#include <queue>
#include <condition_variable>

template <typename T>
class queue {

public:

    queue() : m_valid(true) {}

    ~queue() {
        invalidate();
    }

    /* attempt to get the first value in the queue.
     * returns true if a value was successfully written to the out parameter */
    bool try_pop(T& out) {

        std::lock_guard<std::mutex> lock{m_mutex};

        if(m_queue.empty() || !m_valid) {
            return false;
        }

        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    /* get the first value in the queue.
     * blocks until a value is available unless clear is called or the instance is destroyed
     * returns true if a value was successfully written to the out parameter */
    bool wait_pop(T& out) {

        std::unique_lock<std::mutex> lock{m_mutex};

        m_condition.wait(lock, 
                [this]() {
                    return !m_queue.empty() || !m_valid;
                });

        // spurious wakeups with a valid but empty queue will not proceed, 
        // so only need to check for validity before proceeding.
        if(!m_valid) {
            return false;
        }

        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    /* push a new value into the queue */
    void push(T value) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_queue.push(std::move(value));
        m_condition.notify_one();
    }

    /* check whether the queue is empty */
    bool empty() const {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_queue.empty();
    }

    /* empty the queue */
    void clear() {
        std::lock_guard<std::mutex> lock{m_mutex};

        while(!m_queue.empty()) {
            m_queue.pop();
        }

        m_condition.notify_all();
    }

    /* invalidate the queue.
     * this ensures that no conditions are being waited on when a thread
     * or the application is trying to exit */
    void invalidate() {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_valid = false;
        m_condition.notify_all();
    }

    bool is_valid() const {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_valid;
    }

private:
    std::atomic<bool>       m_valid;
    mutable std::mutex      m_mutex;
    std::queue<T>           m_queue;
    std::condition_variable m_condition;

}; // class queue


#endif /* __THREAD_POOL_QUEUE_HPP__ */

