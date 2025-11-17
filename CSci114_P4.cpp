#include <iostream>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>

// Global variables
std::mutex m_mutex;
std::condition_variable cv;

std::vector<std::vector<int>> requests;

/*
 * These classes and functions are heavily referenced from figure 6.21 in the textbook, such as:
 * Class ResourceMgr, ResourceMgr::isSafe(), and ResourceMgr::wouldBeSafe(int threadID)
 */
class ResourceMgr {
private:
    int avail;
    int totalThreads;
    std::vector <int> max;
    std::vector<int> alloc;
    std::vector<int> need;

public:
    ResourceMgr(int N, int M) { // Constructor
        totalThreads = N;
        avail = M;

        // I'm able to fill a vector with a certain amount of 0's with this: assign(Input, 0)
        alloc.assign(N, 0);
        max.assign(N, 0);
        for (int i = 0; i < N; i++) {
            max[i] = requests[i][0];
        }
    }

    // Starter code thread_func function
    void thread_func(int tid) {
        for (unsigned int i = 0; i < requests[tid].size(); i++) {
            max[tid] = requests[tid][i];
            for (int j = 0; j < requests[tid].at(i); j++) {
                request(tid);
            }
            std::unique_lock<std::mutex> lock(m_mutex);
            avail += alloc[tid];
            alloc[tid] = 0;
            printCurrentAlloc();
            cv.notify_all();
        }
    }

    bool isSafe() {
        int toBeAvail = avail; // In figure 6.21, toBeAvail is an array, but we only have one type of resource in this assignment
        std::vector<int> toBeAlloc = alloc;
        std::vector<bool> finish(totalThreads, false);

        // Variables names and control flow are similar/identical to the example that is shown in figure 6.21
        while (true) {
            int j = -9999; // Representing a value that isn't represented for an index of an array. I tried using NULL but apparently it isn't recommended
            for (int i = 0; i < totalThreads; i++) {
                if (!finish[i] && need[i] <= toBeAvail) {
                    j = i; // Found a tid
                    break;
                }
            }
            if (j == -9999) { // If we didn't find any thread through our previous search
                for (bool thread : finish) {
                    if (thread != true) // If that thread isn't finished, automatically return false, because there's no possible allocation
                        return false;
                }
                return true;
            } else {
                // Comment from figure 6.21, "Thread j will eventually finish and return its current allocation to the pool"
                toBeAvail += toBeAlloc[j];
                finish[j] = true;
            }
        }
    };

    bool wouldBeSafe(int tid) {
        bool result = false;

        avail--; // This is one single-shared object, not multi-shared, so this is an int and not an array for resourceID's
        alloc[tid]++;
        if (isSafe()) {
            result = true;
        }
        avail++;
        alloc[tid]--;
        return result;
    };

    void request(int tid) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (isSafe()) {
            while (!wouldBeSafe(tid))
                cv.wait(lock);
            avail--;
            alloc[tid]++;
            printCurrentAlloc();
        }

        if (isSafe())
            cv.notify_all();
    }

    // Printing
    void printCurrentAlloc() {
        std::cout << "[ ";

        for (int i = 0; i < totalThreads; i++)
            std::cout << alloc[i] << " ";

        std::cout << "]" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Invalid command arguments\n" <<
                     "Example: ./P4.exe 5 6 (5 threads and capacity of 6)\n";
        return 1;
    }

    // N = threads     M = capacity
    int N = std::stoi(argv[1]);
    int M = std::stoi(argv[2]);
    int tid, units;

    requests.assign(N, std::vector<int>());

    // File processing
    std::ifstream file("requests-3.txt");

    // Starter code main()
    while (!file.eof()) {
        std::cin >> tid >> units;
        requests[tid].push_back(units);
    }

    ResourceMgr resource_mgr(N, M);

    // Creating and joining threads
    std::vector<std::thread> threads;
    for (int i = 0; i < N; i++)
        threads.emplace_back(&ResourceMgr::thread_func, &resource_mgr, i);
    for (std::thread& thread : threads)
        thread.join();

    file.close();

    return 0;
}