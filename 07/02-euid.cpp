#include <iostream>
#include <unistd.h>
#include <sys/types.h>

int main() {
    std::cout << "how to test: " << std::endl;
    std::cout << "sudo chown root:root a.out" << std::endl;
    std::cout << "sudo chown +s a.out" << std::endl;
    std::cout << "./a.out" << std::endl;
    uid_t uid = getuid();
    uid_t euid = geteuid();

    std::cout << "uid: " << uid << ", euid: " << euid <<std::endl;
}