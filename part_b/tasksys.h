#ifndef _TASKSYS_H
#define _TASKSYS_H
// 参考自https://zhuanlan.zhihu.com/p/711187442
#include "itasksys.h"
#include <condition_variable>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <list>
#include <algorithm>
struct Task{
    IRunnable* runnable;
    int num_total_works;
    std::vector<TaskID> deps;
    int current_work_id;
    int num_done_works;
    int num_left_deps;
    Task(IRunnable* runnable, int num_total_works, const std::vector<TaskID>& deps, int current_work_id, int num_done_works, int num_left_deps)
        : runnable(runnable), num_total_works(num_total_works), deps(deps),
          current_work_id(current_work_id), num_done_works(num_done_works), num_left_deps(num_left_deps) {}
};
/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
    private:
        bool stop;
        int num_threads;
        std::vector<std::thread> threads;
        
        std::vector<std::shared_ptr<std::mutex>> tlks;
        std::mutex lk;
        std::condition_variable cv;
        std::vector<Task> tasks;
        std::queue<TaskID> ready_tasks;
        std::list<TaskID> waiting_tasks;
        std::vector<TaskID> done_tasks;
        int taskid_cnt;
};

#endif
