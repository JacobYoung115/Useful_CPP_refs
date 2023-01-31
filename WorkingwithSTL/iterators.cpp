#include <iostream>
#include <vector>
#include <array>
#include <list>

//printing without iterators..
void print(const std::array<int, 10>& arr) {
    for(const auto& value : arr) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

void print(const std::vector<int>& vec) {
    for(const auto& value : vec) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

//template function with an iterator.
template<typename Iterator>
void print_iter(Iterator begin, Iterator end) {
    for(Iterator it = begin; it != end; it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::array<int, 10> arr = {0,1,2,3,4,5,6,7,8,9};
    std::vector<int> vec = {0,1,2,3,4,5,6,7,8,9};
    std::list<int> l = {0,1,2,3,4,5,6,7,8,9};

    std::cout << "arr: ";
    print(arr);

    std::cout << "vec: ";
    print(vec);

    std::cout << "list: ";

    //The most effective way is to use an iterator, to reduce the amount of code to write.
    print_iter(l.begin(), l.end());
    print_iter(arr.begin(), arr.end());
    print_iter(vec.begin(), vec.end());
}