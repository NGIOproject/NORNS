#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <vector>
#include <thread>
#include <future>
#include <condition_variable>
#include <utility>

#include "common.hpp"
#include "thread-pool-queue.hpp"

namespace detail {


class task {

public:
    struct noop_epilog {
        constexpr noop_epilog() noexcept = default;
        void operator()(){};
    };

    task(void) = default;
    virtual ~task(void) = default;
    task(const task& /*rhs*/) = delete;
    task& operator=(const task& /*rhs*/) = delete;
    task(task&& /*other*/) = default;
    task& operator=(task&& /*other*/) = default;

    /**
        * Run the task.
        */
    virtual void execute() = 0;
};

}

class pool {

    template <typename CallableTask, typename CallableEpilog = detail::task::noop_epilog>
    class task : public detail::task {

    public: 

        task(CallableTask&& func)
            : m_func(std::move(func)) {}

        task(CallableTask&& func, CallableEpilog&& epilog)
            : m_func(std::move(func)),
              m_epilog(std::move(epilog)) {}

        ~task() override = default;
        task(const task& /*rhs*/) = delete;
        task& operator=(const task& /*rhs*/) = delete;
        task(task&& /*other*/) = default;
        task& operator=(task&& /*other*/) = default;

        void execute() override {
            m_func();
            m_epilog();
        }

    private:
        CallableTask m_func;
        CallableEpilog m_epilog;
    };

public:
    template <typename T>
    class task_future {
    public:
        task_future(std::future<T>&& future) 
            : m_future(std::move(future)) {}

        task_future(const task_future& rhs) = delete;
        task_future& operator=(const task_future& rhs) = delete;
        task_future(task_future&& other) = default;
        task_future& operator=(task_future&& other) = default;

        ~task_future() {
            if(m_future.valid()) {
                m_future.get();
            }
        }

        T get() {
            return m_future.get();
        }

    private:
        std::future<T> m_future;
    };


public:
    pool() 
        : pool(std::max(std::thread::hardware_concurrency(), 2u) - 1u) {}

    explicit pool(const uint32_t num_threads)
        : m_done(false) { 

        try {
            for(uint32_t i = 0; i < num_threads; ++i) {
                m_threads.emplace_back(&pool::worker, this);
            }
        }
        catch(...) {
            destroy();
            throw;
        }
    }

    pool(const pool& rhs) = delete;
    pool& operator=(const pool& rhs) = delete;

    ~pool() {
        destroy();
    }

    template <typename FuncType, typename... Args>
    auto submit_and_track(FuncType&& func, Args&&... args) -> 
        task_future<typename std::result_of<decltype(std::bind(std::forward<FuncType>(func), std::forward<Args>(args)...))()>::type> {

        auto bound_task = std::bind(std::forward<FuncType>(func), std::forward<Args>(args)...);
        using ResultType = typename std::result_of<decltype(bound_task)()>::type;
        using PackagedTaskType = std::packaged_task<ResultType()>;
        using TaskType = task<PackagedTaskType>;

        PackagedTaskType task{std::move(bound_task)};
        task_future<ResultType> result{task.get_future()};
        m_work_queue.push(std::make_unique<TaskType>(std::move(task)));

        return result;
    }

    template <typename FuncType, typename... Args>
    void submit_and_forget(FuncType&& func, Args&&... args) {

        auto bound_task = std::bind(std::forward<FuncType>(func), std::forward<Args>(args)...);
        using TaskType = task<decltype(std::bind(std::declval<FuncType>(), std::declval<Args>()...))>;

        TaskType task{std::move(bound_task)};

        m_work_queue.push(std::make_unique<TaskType>(std::move(task)));
    }

    template <typename CallableTask, typename CallableEpilog, typename... TaskArgs>
    void submit_with_epilog_and_forget(CallableTask&& func, CallableEpilog&& epilog, TaskArgs&&... func_args) {

        auto bound_task = std::bind(std::forward<CallableTask>(func), std::forward<TaskArgs>(func_args)...);
        auto bound_epilog = std::bind(std::forward<CallableEpilog>(epilog));
        using TaskType = task<decltype(std::bind(std::declval<CallableTask>(), std::declval<TaskArgs>()...)),
                              decltype(std::bind(std::declval<CallableEpilog>()))>;

        TaskType task{std::move(bound_task), std::move(bound_epilog)};

        m_work_queue.push(std::make_unique<TaskType>(std::move(task)));
    }

    void stop() {
        destroy();
    }

    
private:

    void worker() {

        while(!m_done) {
            std::unique_ptr<detail::task> task_ptr;

            if(m_work_queue.wait_pop(task_ptr)) {
                task_ptr->execute();
            }
        }
    }

    void destroy() {
        m_done = true;
        m_work_queue.invalidate();

        for(auto& th : m_threads) {

            if(th.joinable()) {
                th.join();
            }
        }
    }

private:
    std::atomic<bool> m_done;
    queue<std::unique_ptr<detail::task>> m_work_queue;
    std::vector<std::thread> m_threads;
};

#if 0
namespace default_pool {

    inline pool& get_pool() {
        static pool default_pool;
        return default_pool;
    }

    template <typename FuncType, typename... Args>
    inline auto submit(FuncType&& func, Args&&... args) {
        return get_pool().submit(std::forward<FuncType>(func), std::forward<Args>(args)...);

    }
}
#endif


#endif /* __THREAD_POOL_HPP__ */


