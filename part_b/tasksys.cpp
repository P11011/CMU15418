#include "tasksys.h"


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    stop = false;
    taskid_cnt = 0;
    
    for(int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this]{
            while(1) {
                std::unique_lock<std::mutex> lock(lk);
                cv.wait(lock, [this] { return stop || !ready_tasks.empty(); });
                if (stop && ready_tasks.empty()) return;
                if(!ready_tasks.empty()) {
                    int tid = ready_tasks.front();
                    std::mutex& tlk = *tlks[tid];
                    Task& task = tasks[tid];
                    lock.unlock();
                    int work_id = -1;
                    int num_total_works;
                    IRunnable* runnable;
                    tlk.lock();
                    runnable = task.runnable;
                    num_total_works = task.num_total_works;
                    if(task.current_work_id < task.num_total_works) {
                        work_id = task.current_work_id++;
                    }
                    tlk.unlock();
                    if(work_id != -1) {
                        runnable->runTask(work_id, num_total_works);
                        bool is_task_done = false;
                        tlk.lock();
                        task.num_done_works++;
                        if(task.num_done_works == task.num_total_works) {
                            is_task_done = true;
                        }
                        tlk.unlock();
                        lock.lock();
                        if(is_task_done) {
                            ready_tasks.pop();
                            done_tasks.push_back(tid);
                            for(auto it = waiting_tasks.begin(); it != waiting_tasks.end(); ) {
                                TaskID wid = *it;
                                Task& wtask = tasks[wid];
                                tlk.lock();
                                if (std::find(wtask.deps.begin(), wtask.deps.end(), tid) != wtask.deps.end()) {
                                    wtask.num_left_deps--;
                                }
                                if(wtask.num_left_deps == 0) {
                                    it = waiting_tasks.erase(it);
                                    ready_tasks.push(wid);
                                } else {
                                    ++it;
                                }
                                tlk.unlock();
                            }
                        }
                        lock.unlock();
                        if(is_task_done)
                            cv.notify_all();
                    }
                }
            }
        });
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    stop = true;
    cv.notify_all();
    for(std::thread& thread: threads) {
       thread.join();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::vector<TaskID> nodeps;
    runAsyncWithDeps(runnable, num_total_tasks, nodeps);
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //
    lk.lock();
    tlks.push_back(std::make_shared<std::mutex>());
    TaskID tid = taskid_cnt++;
    int num_left_deps = deps.size();
    for(TaskID d: deps) {
        auto it = std::find(done_tasks.begin(), done_tasks.end(), d);
        if(it != done_tasks.end()) {
            num_left_deps--;
        }
    }
    if(num_left_deps == 0) {
        ready_tasks.push(tid);
    } else {
        waiting_tasks.push_back(tid);
    }
    tasks.emplace_back(runnable, num_total_tasks, deps, 0, 0, num_left_deps);
    lk.unlock();
    cv.notify_all();
    return tid;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    std::unique_lock<std::mutex> lock(lk);
    while (ready_tasks.size() || waiting_tasks.size()) {
        cv.wait(lock);
    }
}
