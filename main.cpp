#include <iostream>
#include "ring_buf.h"

int main()
{
    ring_buf<int, 4> buf;

    constexpr auto capacity = buf.capacity();

    std::cout << "\nCapacity: " << capacity << "\n";

    std::cout << "\n[ ] Printing empty:\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[+] Filling buffer";
    for (int i = 1; i <= capacity; ++i) buf.push_back(i); 

    std::cout << "\n[ ] Printing 1st time:\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[+] Adding 4 elements";
    for (int i = capacity + 1; i <= capacity + 4; ++i) buf.push_back(i); 	

    std::cout << "\n[ ] Printing 2nd time:\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[-] Deleting 2 elements";
    for (int i = 0; i < 2; ++i) buf.pop_front();

    std::cout << "\n[ ] Printing 3rd time:\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n[+] Adding 4 elements";
    for (int i = capacity + 5; i <= capacity + 8; ++i) buf.push_back(i);

    std::cout << "\n[ ] Printing 4th time:\t";
    for (auto &it : buf) std::cout << it << ' ';

    std::cout << "\n\n";
}