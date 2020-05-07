
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <chrono>

class null_monitoring
{
    public:
        void report(const std::string& msg)
        {}
};

class monitoring
{
    public:
        void report(const std::string& msg)
        {
            std::cout << "report : " << msg << '\n';
        }
};

template <typename T>
class Obj : public T
{
    public:
        void compute()
        {
            T::report("important reporting");
            std::cout << "Do an important computation" << '\n';
        }
};

int main()
{
    auto start = std::chrono::steady_clock::now();

    //Obj<null_monitoring> o;
    Obj<monitoring> o;
    o.compute();

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";

    start = std::chrono::steady_clock::now();

    Obj<null_monitoring> o1;
    o1.compute();

    end = std::chrono::steady_clock::now();
    elapsed_seconds = end-start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
}

/*
Possible output:
  Program returned: 0
  report : important reporting
  Do an important computation
  elapsed time: 3.6538e-05s
  Do an important computation
  elapsed time: 8.81e-07s
*/
