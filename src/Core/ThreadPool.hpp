#pragma once

#include <thread>
#include <functional>
#include <condition_variable>

template <class T>
class CThreadPool
{
public:
    CThreadPool(std::function<void(const T&)> func);
    ~CThreadPool();

    int32_t GetNumWorkers() const { return NumWorkers; }
    void SetNumWorkers(uint32_t num) { NumWorkers = num; }

    void Dispatch(uint32_t thread, const T& info);
    
    void Join();
    void Join(uint32_t thread);

private:
    void Work(uint32_t thread);

    std::function<void(const T&)> UserFunction;

    static const uint32_t MAX_WORKERS = 31;
    std::thread Workers[MAX_WORKERS];
    uint32_t NumWorkers;

    std::condition_variable WorkReady[MAX_WORKERS];
    std::mutex Mutex[MAX_WORKERS];

    T WorkerInfo[MAX_WORKERS];
    bool HaveWork[MAX_WORKERS];
};

template<class T>
inline CThreadPool<T>::CThreadPool(std::function<void(const T&)> func) : UserFunction(func)
{
    NumWorkers = std::thread::hardware_concurrency();
    NumWorkers = NumWorkers == 0 ? 8 : NumWorkers;
    --NumWorkers;
    
    for (uint32_t i = 0; i < NumWorkers; ++i)
    {
        Workers[i] = std::thread(&CThreadPool::Work, this, i);
        HaveWork[i] = false;
    }
}

template<class T>
inline CThreadPool<T>::~CThreadPool()
{
    for (uint32_t i = 0; i < NumWorkers; ++i)
    {
        Workers[i].detach();
    }
}

template<class T>
inline void CThreadPool<T>::Dispatch(uint32_t thread, const T& info)
{
    {
        std::unique_lock<std::mutex> lock(Mutex[thread]);
        WorkerInfo[thread] = info;
        HaveWork[thread] = true;
    }

    WorkReady[thread].notify_one();
}

template<class T>
inline void CThreadPool<T>::Join()
{
    for (uint32_t i = 0; i < NumWorkers; ++i)
    {
        std::unique_lock<std::mutex> lock(Mutex[i]);

        while (HaveWork[i])
        {
            WorkReady[i].wait(lock);
        }
    }
}

template<class T>
inline void CThreadPool<T>::Join(uint32_t thread)
{
    std::unique_lock<std::mutex> lock(Mutex[thread]);

    while (HaveWork[thread])
    {
        WorkReady[thread].wait(lock);
    }
}

template<class T>
inline void CThreadPool<T>::Work(uint32_t thread)
{
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(Mutex[thread]);

            while (!HaveWork[thread])
            {
                WorkReady[thread].wait(lock);
            }
        }

        UserFunction(WorkerInfo[thread]);

        {
            std::unique_lock<std::mutex> lock(Mutex[thread]);
            HaveWork[thread] = false;
        }

        WorkReady[thread].notify_one();
    }
}
