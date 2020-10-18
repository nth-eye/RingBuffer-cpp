#include <iostream>
#include "ring_buf.h"

int main()
{
    ring_buf<int, 8> buf;

    constexpr auto capacity = buf.capacity();

    std::cout << "\nCapacity: " << buf.capacity() << "\n";

    std::cout << "\n[ ] Printing empty...\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[+] Filling buffer... ";
    for (int i = 1; i <= capacity; ++i) buf.push_back(i); 

    std::cout << "\n[ ] Printing first time...\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[+] Adding 6 elements... ";
    for (int i = capacity + 1; i <= capacity + 6; ++i) buf.push_back(i); 	

    std::cout << "\n[ ] Printing second time...\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[-] Deleting 4 elements... ";
    for (int i = 0; i < 4; ++i) buf.pop_front();

    std::cout << "\n[ ] Printing third time...\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[+] Adding 6 elements... ";
    for (int i = capacity + 7; i <= capacity + 12; ++i) buf.push_back(i);

    std::cout << "\n[ ] Printing last time...\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n\n";
}