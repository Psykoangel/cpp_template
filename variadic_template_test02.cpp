
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <any>
#include <functional>
#include <stdexcept>
#include <typeinfo>

template<typename Ret>
struct AnyCallable
{
    AnyCallable() = default;

    template<typename F>
    AnyCallable(F&& fun) : AnyCallable(std::function(fun))
    {}

    template<typename... Args>
    AnyCallable(std::function<Ret(Args...)> fun) : m_any(fun)
    {}

    template<typename... Args>
    Ret operator()(Args&&... args)
    {
        return std::invoke(
            std::any_cast<std::function<Ret(Args...)>>(m_any), std::forward<Args>(args)...);
    }

    template<typename... Args>
    Ret compute(Args&&... args)
    {
        return operator()(std::forward<Args>(args)...);
    }

    std::any m_any;
};

template<>
struct AnyCallable<void>
{
    AnyCallable() = default;

    template<typename Func>
    AnyCallable(Func&& fun) : AnyCallable(std::function(fun))
    {}

    template<typename... Args>
    AnyCallable(std::function<void(Args...)> fun) : m_any(fun)
    {}

    template<typename... Args>
    void operator()(Args&&... args)
    {
        using fType = std::tuple_element_t<0, std::tuple<Args...>>;
        std::cout << "AnyCallable operator() " << sizeof...(Args) << "    " << typeid(fType).name() << std::endl;
        std::invoke(
            std::any_cast<std::function<void(Args...)>>(m_any), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void compute(Args&&... args)
    {
        std::cout << "AnyCallable compute" << std::endl;
        operator()(std::forward<Args>(args)...);
    }

    std::any m_any;
};


class AP;
class BP;

class P : public AnyCallable<void>
{
    public:
        P() = default;

        template<typename Func>
        P(Func&& fun) : AnyCallable<void>(std::function(fun))
        {}
        template<typename... Args>
        P(std::function<void(Args...)> fun) : AnyCallable<void>(fun)
        {}

        void addSubs(const std::string& name, std::vector<std::unique_ptr<P>>&& ps)
        {
            for (auto& p : ps)
            {
                _subs.insert({name, std::move(p)});
            }
        }
    
    protected:
        template<typename... Ts>
        void applySub(const std::string& name, Ts&&... ts)
        {
            std::cout << "Apply Sub "  << name << std::endl;
            _subs[name]->compute<Ts...>(std::forward<Ts>(ts)...);
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<P>> _subs;
};

class AP : public P
{
    public:
        AP() : P([this](const std::string& s){ compute(s); }) { init(); }

        void compute(const std::string& str)
        {
            std::cout << "AP::compute str " << str << std::endl;
            std::string s("qwertyiop");
            int i = 3;
            applySub<int&, const std::string&>("two", i, s);
        }

        void init()
        {
            std::cout << "AP::init()" << std::endl;
            std::vector<std::unique_ptr<P>> v;
            v.emplace_back(std::move(std::make_unique<BP>()));
            addSubs("two", std::move(v));
            std::cout << "AP::init() ends" << std::endl;
        }
};

class BP : public P
{
    public:
        BP() : P([this](int& i, const std::string& s){ compute(i, s); }) {}
        void compute(int& i, const std::string& str) {std::cout << "BP::compute i " << i << " str " << str << std::endl;}
};

template<typename Base>
class Creator
{
    public:
        virtual ~Creator() = default;

        virtual std::unique_ptr<Base> create() = 0;
};

template<class Key, class T>
class Factory
{
    public:
        void store(Key key, std::unique_ptr<Creator<T>>&& creator)
        {
            _crs[key] = std::move(creator);
        }

        std::unique_ptr<T> create(Key key)
        {
            std::cout << "Factory::create() " << key << std::endl;
            return _crs[key]->create();
        }

    protected:
        size_t creatorsNb() { return _crs.size(); }

    private:
        std::map<Key, std::unique_ptr<Creator<T>>> _crs;
};

template<typename Derived>
class PCreator : public Creator<P>
{
    public:
        std::unique_ptr<P> create() override
        {
            std::cout << "PCreator::create() " << typeid(Derived).name() << std::endl;
            return std::make_unique<Derived>();
        }
};

class PFactory : public Factory<std::string, P>
{
    public:
        PFactory()
        {
            this->store("one", std::make_unique<PCreator<AP>>());
            this->store("two", std::make_unique<PCreator<BP>>());
            std::cout << "PFactory ctr ends with " << creatorsNb() << std::endl;
        }
};

class Thing
{
    const std::array<std::string, 2> a = {"one", "two"};
    public:
        Thing()
        {
            std::cout << "Thing()" << std::endl;
            PFactory f;
            for(const auto& e : a)
            {
                auto p = f.create(e);
                _ps.insert({e, std::move(p)});
            }

            std::cout << "Thing() ctr ends" << std::endl;
        }

        void compute()
        {
            std::cout << "Thing::compute()" << std::endl;
            int i = 100;
            std::string str = "qwerty";
            // additional computations...
            _ps["one"]->compute<const std::string&>(str);
            // additional computations...
            _ps["two"]->compute<int&, const std::string&>(i, str);
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<P>> _ps;
};

int main()
{
    /*
    std::cout << "compute AP" << std::endl;
    std::string str = "abc";
    std::unique_ptr<P> p = std::make_unique<AP>();
    p->compute<std::string&>(str);

    std::cout << "compute BP" << std::endl;
    int i = 15;
    std::unique_ptr<P> p1 = std::make_unique<BP>();
    p1->compute<int&, const std::string&>(i, str);
*/
//    std::cout << "compute Thing" << std::endl;
    Thing a;
    a.compute();
}
