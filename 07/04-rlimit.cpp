#include <iostream>
#include <cassert>
#include <csignal>

#include <unistd.h>
#include <sys/resource.h>

bool b = false;

void sigXCPUHandler(int sig) {
    std::cout << "recv signal number: " << sig << std::endl;
    b = true;
}

int main() {
    signal(SIGXCPU, sigXCPUHandler);
    struct rlimit limit;
    assert(getrlimit(RLIMIT_CPU, &limit) == 0);

    std::cout << "RLIMIT_CPU cur: " << limit.rlim_cur << std::endl;
    std::cout << "RLIMIT_CPU max: " << limit.rlim_max << std::endl;

    limit.rlim_cur = 1;
    limit.rlim_max = 1;
    assert(setrlimit(RLIMIT_CPU, &limit) == 0);

    assert(getrlimit(RLIMIT_CPU, &limit) == 0);
    std::cout << "RLIMIT_CPU cur: " << limit.rlim_cur << std::endl;
    std::cout << "RLIMIT_CPU max: " << limit.rlim_max << std::endl;

    while(!b) {

    }

    return 0;
}