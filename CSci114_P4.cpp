#include <iostream>
#include <vector>
#include <unistd.h>
#include <thread>

std::vector<int> requests[16];

void thread_func(int tid) {
    for (unsigned int i = 0; i < requests[tid].size(); i++) {
        for (int j = 0; j < requests[tid].at(i); j++) {
            request(tid);
        }
        avail += alloc[tid];
        alloc[tid] = 0;
        usleep(rand() % 500);
    }
}

int main() {
    while (not eof) {
        std::cin >> tid >> units;

        requests[tid].push_back(units);
    }

    for (i from 1 to N)
        pthread_create(i, thread_func);
    return 0;
}