#include <iostream>

#include <cassert>
#include <cstring>
#include <unistd.h>

int main() {
    // 由系统调用分配内存，需要手动释放
    char* pwd = getcwd(nullptr, 0);
    std::cout << pwd << std::endl;
    delete []pwd;


    // 用户自己分配内存，不需要手动释放
    char mypwd[1024];
    assert(getcwd(mypwd, 1024) != nullptr);
    std::cout << mypwd << std::endl;

    assert(chdir("/") == 0);

    memset(mypwd, '\0', 1024);
    assert(getcwd(mypwd, 1024) != nullptr);
    std::cout << mypwd << std::endl;
    return 0;
}