#include <iostream>
#include <utility>

template <typename... Args>
class RecevierBase
{
public:
    virtual void Notify(Args...) = 0;
};


template <typename Parent, typename... Args>
class Recevier : public RecevierBase<Args...>
{
    typedef void (Parent::* NOTIFY_METHOD)(Args...);
public:
    Recevier(Parent *parent, NOTIFY_METHOD notifyMethod) :
        _parent(parent), _notifyMethod(notifyMethod)
    {  }

    virtual void Notify(Args... args)
    {
        (_parent->*_notifyMethod)(args...);
    }

    template<typename... Ts>
    void Notify(Ts&&... args)
    {
        (_parent->*_notifyMethod)(std::forward<Args>(args)...);
    }

private:
    Parent* _parent;
    NOTIFY_METHOD _notifyMethod;
};

struct Parent
{
    void CallMe(int n)
    {
        std::cout << "Called with " << n << std::endl;
    }
};

int main()
{
    Parent p;
    Recevier<Parent, int> r(&p, &Parent::CallMe);
    r.Notify(4);
}
