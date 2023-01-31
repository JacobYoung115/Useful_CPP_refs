#include <algorithm>
#include <iostream>
#include <vector>
//This folows: https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp

/*
    //C++ defines 6 types of iterators. These are heirarchical with #6 being at the highest level of heirarchy.
    (meaning a 'Contiguous Iterator' is also a 'Random Access Iterator', ...)

    1. Input iterator (read-only)
        forward scan of the container one time. Doesn't modify values.

    2. Output Iterator (write-only)
        forward scan of container once, can't read values it points to.

    3. Forward Iterator
        Can scan the container forward multiple times, able to read and write the values it points to.

    4. Bidirectional Iterator
        same as 3. but is back and forth

    5. Random Access Iterator
        same as 4, but can access non-sequentially

    6. Contiguous Iterator
        same as 5, but logically adjacent elements are physically adjacent in memory.


    By default, all iterators are (1) Input Iterators [read-only] aka 'constant iterators'
    Iterators which support *read* and *write* operators are (2) Output iterators aka 'mutable iterators'.

    ---------------------------------------------------------------------------

    Iterators (1) & (2) are often used for input and output streams [single-pass algorithms]
    We will use a (3) Forward Iterator to work with the custom container.

    1. Define iterator inside the class
    2. Prepare custom iterator with tags
    3. Define iterator constructors
    4. Implement operators for the iterator (*iterator, iterator->x, iterator++, ==, !=, ...)
    5. Prepare the container

    ---------------------------------------------------------------------------
*/

//Define some custom container, with which we will iterate over.
//Integers class is a 'wrapper' around a raw aray of ints.              (toy example to explain concept)
class Integers {
    public:
        //1. define iterator
        struct Iterator { 
            /* 
            2. You must define properties for an iterator. 
            note: wrong tags mean sub-optimal perforamnce, since STL uses these tags to decide which algs to use. 
            */ 
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = int;                          //int, since our data is an int, using other types makes the compiler skip over more or less memory.
            using pointer           = int*;     //or also value_type*
            using reference         = int&;     //or also value_type&
            //3. all iterators must be constructible, copy-constructible, copy-assignable, destructible and swappable.
            //a pointer to the container satisfies constructible, while the rest are implicitly declared.
            Iterator(pointer ptr) : m_ptr(ptr) {}

            //4. implement operators for iterator
            reference operator*() const { return *m_ptr; }
            pointer operator->() { return m_ptr; }

            //prefix & postfix operators
            Iterator& operator++() { m_ptr++; return *this; }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

            //equality operators
            //'friend' keyword defines operators as non-member functions which can access private members of Iterator.
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

            private:
                pointer m_ptr;
        };  //end struct iterator
        //5. Add begin() and end() for creating Iterator objects.
        Iterator begin() {return Iterator(&m_data[0]);}
        Iterator end() { return Iterator(&m_data[4]);}        //note the iterator must return the position AFTER the last index value (invalid memory).
    private:
        int m_data[4];
};


//Custom wrapper with an explicitly defined iterator and type (toy example)
class IntVec {
    public:
        //1. define iterator
        IntVec() {}
        IntVec(std::vector<int>& data): m_data(data) {}
        struct Iterator { 
            /* 
            2. You must define properties for an iterator. 
            note: wrong tags mean sub-optimal perforamnce, since STL uses these tags to decide which algs to use. 
            */ 
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = int;                          //int, since our data is an int, using other types makes the compiler skip over more or less memory.
            using pointer           = int*;     //or also value_type*
            using reference         = int&;     //or also value_type&
            //3. all iterators must be constructible, copy-constructible, copy-assignable, destructible and swappable.
            //a pointer to the container satisfies constructible, while the rest are implicitly declared.
            Iterator(pointer ptr) : m_ptr(ptr) {}

            //4. implement operators for iterator
            reference operator*() const { return *m_ptr; }
            pointer operator->() { return m_ptr; }

            //prefix & postfix operators
            Iterator& operator++() { m_ptr++; return *this; }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

            //equality operators
            //'friend' keyword defines operators as non-member functions which can access private members of Iterator.
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

            private:
                pointer m_ptr;
        };

        //5. Add begin() and end() for creating Iterator objects.
        //convert references to pointers:
        //https://stackoverflow.com/questions/19032461/convert-reference-to-pointer-representation-in-c
        Iterator begin() {return Iterator( std::addressof(m_data.front()) );}
        Iterator end() {
             Iterator::pointer back = std::addressof(m_data.back());
             ++back;
            return Iterator(back); //more general than choosing an index, but needs to be incremented.
        }         
        std::vector<int>& vector() { return m_data; }
    private:
        std::vector<int> m_data = {};        
};


//If your class is a wrapper of a std lib datastruct, you can just pass the iterator (toy example)
class Doubles {
    using DoublesType = std::vector<double>;
    public:
        Doubles() {}
        Doubles(DoublesType data) : m_data(data) {}
        DoublesType::iterator begin() { return m_data.begin(); }
        DoublesType::iterator end() { return m_data.end(); }
    private:
        std::vector<double> m_data = std::vector<double>(0, 0); //length, default value
};


//now, try it with template classes
/*
    Template parameters on a function either have to be explicitly provided OR
    have to be deductible from the function parameters
    iterators have a value_type member
*/


//Templated custom container using a templated datatype on stl datastructure with explicit iterator implementation 
template <typename X>
class TempVec {
    public:
        //1. define iterator
        TempVec() {}
        TempVec(std::vector<X>& data): m_data(data) {}
        struct Iterator { 
            /* 
            2. You must define properties for an iterator. 
            note: wrong tags mean sub-optimal perforamnce, since STL uses these tags to decide which algs to use. 
            */ 
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = X;                          //int, since our data is an int, using other types makes the compiler skip over more or less memory.
            using pointer           = X*;     //or also value_type*
            using reference         = X&;     //or also value_type&
            //3. all iterators must be constructible, copy-constructible, copy-assignable, destructible and swappable.
            //a pointer to the container satisfies constructible, while the rest are implicitly declared.
            Iterator(pointer ptr) : m_ptr(ptr) {}

            //4. implement operators for iterator
            reference operator*() const { return *m_ptr; }
            pointer operator->() { return m_ptr; }

            //prefix & postfix operators
            Iterator& operator++() { m_ptr++; return *this; }
            Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

            //equality operators
            //'friend' keyword defines operators as non-member functions which can access private members of Iterator.
            friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
            friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

            private:
                pointer m_ptr;
        };  //end struct iterator
        //5. Add begin() and end() for creating Iterator objects.
        //convert references to pointers:
        //https://stackoverflow.com/questions/19032461/convert-reference-to-pointer-representation-in-c
        Iterator begin() {return Iterator( std::addressof(m_data.front()) );}
        Iterator end() {
            //Rvalue incrementing is not valid in C++. It must be done on a lvalue variable.
            //https://stackoverflow.com/questions/53985228/why-is-rvalue-incrementing-in-c-illegal
            //use template type as a pointer.
            X* back = std::addressof(m_data.back());
            ++back;
            return Iterator(back);
        }         //more general than choosing an index, but needs to be incremented.
        std::vector<X>& vector() { return m_data; }
    private:
        std::vector<X> m_data = {};        
};


// ------------------------- A simplified iterable custom container with templated data type (preferred) -------------------------
template <typename T>
struct NamedTemplate {
    public:
        NamedTemplate() {}
        NamedTemplate(std::vector<T> data) : m_data(data) {}
        //Overriding the begin and end functions for the stl library doesn't require an iterator explicity. It can just be a pointer.
        T* begin() { return std::addressof(m_data.front()); }
        T* end() { 
            T* back = std::addressof(m_data.back());
            ++back;
            return back;
        }
     private:
        std::vector<T> m_data = std::vector<T>(0);
};


int main() {
    
    //------------------------- default iterator w/ std container -------------------------
    std::vector<int> some_vec = {1,2,3,4,5};
    int* start = &some_vec[0];               //this means we have a pointer (*), which points to the memory address (&) of some variable.
    int* end = &some_vec[5];
    std::cout << "Memory address start (int*): " << start << std::endl;
    std::cout << "Memory address end (int*): " << end << std::endl;

    while(start != end) {
        const auto i = *start;
        std::cout << "iteration with pointers: " << i << "\n";
        start++;
    }


    //------------------------- custom iterator on a wrapper class (toy example) -------------------------
    Integers integers;

    std::fill(integers.begin(), integers.end(), 3);

    //equivalent to a range based for loop
    for (auto iter = integers.begin(), end = integers.end(); iter != end; ++iter) {
        const auto i = *iter;
        std::cout << i << "\n";
    }


    //------------ wrapper of std container with custom iterator implementation. (toy example) ------------
    std::vector<int> v = {4,5,6,7,8};
    IntVec tempInt{v};
    tempInt.vector().emplace_back(9);
    
    
    auto tempstart = tempInt.begin();
    auto tempend = tempInt.end();
    //std::fill(tempInt.begin(), tempInt.end(), 10);

    while(tempstart != tempend) {
        const auto& i = *tempstart;
        std::cout << "iteration with custom iterator: " << i << "\n";
        tempstart++;
    }

    for (const auto& i : tempInt) {
        std::cout << "range-for loop with custom iterator: " << i << "\n";
    }

    //another wrapper w/ different type
    Doubles doubles = Doubles({5.0, 6.0, 7.0, 8.0});

    std::fill(doubles.begin(), doubles.end(), 1.999);
    for (auto i : doubles) {
        std::cout << i << "\n";
    }


    //------------------- Try with a template custom container iterator and std lib datastructure (custom implementation) ------------------
    std::vector<int> new_vec = {11,12,13,14,15};
    TempVec tempVector{new_vec};

    tempVector.vector().emplace_back(16);
    for (auto i : tempVector) {
        std::cout << "template range-for loop with custom iterator: " << i << "\n";
    }


    //trying with simplified
    std::vector<double> dv2 = {1.1,2.2,3.3,4.4};
    NamedTemplate<double> double_obj2(dv2);
    std::fill(double_obj2.begin(), double_obj2.end(), 1.234);
    for (auto i : double_obj2) {
        std::cout << i << "\n";
    }

    //Use more stl library algorithms.
    std::vector<int> intv2 = {1000, 10, 102, 100};
    NamedTemplate<int> int_obj2(intv2);

    std::sort(int_obj2.begin(), int_obj2.end());
    for (auto i : int_obj2) {
        std::cout << i << "\n";
    }
    
    std::fill(int_obj2.begin(), int_obj2.end(), 99);
    for (auto i : int_obj2) {
        std::cout << i << "\n";
    }
    
    return 0;
}

