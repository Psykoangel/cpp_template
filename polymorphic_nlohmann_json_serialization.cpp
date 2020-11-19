#include <nlohmann/json.hpp>

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>

using json = nlohmann::json;

namespace nlohmann {
	template <typename T>
	struct adl_serializer<std::unique_ptr<T>> {
		static void to_json(json &j, const std::unique_ptr<T> &opt) {
			if (opt) {
				j = *opt.get();
			}
			else {
				j = nullptr;
			}
		}
	};
}

namespace PolymorphicJsonSerializer_impl {
	template <class Base>
	struct Serializer {
		void (*to_json)(json &j, Base const &o);
		void (*from_json)(json const &j, Base &o);
	};

	template <class Base, class Derived>
	Serializer<Base> serializerFor() {
		return {
			[](json &j, Base const &o) {
				return to_json(j, static_cast<Derived const &>(o));
			},
			[](json const &j, Base &o) {
				return from_json(j, static_cast<Derived &>(o));
			}
		};
	}
}

template <class Base>
struct PolymorphicJsonSerializer {

	// Maps typeid(x).name() to the from/to serialization functions
	static inline std::unordered_map<
		char const *,
		PolymorphicJsonSerializer_impl::Serializer<Base>
	> _serializers;

	template <class... Derived>
	static void register_types() {
		(_serializers.emplace(
			typeid(Derived).name(),
			PolymorphicJsonSerializer_impl::serializerFor<Base, Derived>()
		), ...);
	}

	static void to_json(json &j, Base const &o) {
		char const *typeName = typeid(o).name();
		_serializers.at(typeName).to_json(j, o);
		j["_type"] = typeName;
	}

	static void from_json(json const &j, Base &o) {
		_serializers.at(j.at("_type").get<std::string>().c_str()).from_json(j, o);
	}
};

////////////////////////////////////////////////////////////////

class Base {
public:
	Base() = default;
	virtual ~Base() = default;

	virtual void foo() const { std::cout << "Base::foo()" << std::endl; }
};

class Obj : public Base
{
public:
	Obj(int i) : _i(i) {}
	void foo() const override { std::cout << "Obj::foo()" << std::endl; }
	int _i = 0;
};

namespace nlohmann {
	template <>
	struct adl_serializer<Base>
		: PolymorphicJsonSerializer<Base> { };
}

void to_json(json &, const Base &)
{
	std::cout << "called to_json for Base" << std::endl;
}

void to_json(json &, const Obj &)
{
	std::cout << "called to_json for Obj" << std::endl;
}

void from_json(json const &, Base &)
{
	std::cout << "called from_json for Base" << std::endl;
}

void from_json(json const &, Obj &)
{
	std::cout << "called from_json for Obj" << std::endl;
}

int main()
{
	std::vector<std::unique_ptr<Base>> v;
	v.push_back(std::make_unique<Base>());
	v.push_back(std::make_unique<Obj>(5));
	v.push_back(std::make_unique<Base>());
	v.push_back(std::make_unique<Obj>(10));

	std::cout << v.size() << std::endl;

	PolymorphicJsonSerializer<Base>::register_types<Base, Obj>();

	json j = v;
}

// Output :
// 4
// called to_json for Base
// called to_json for Obj
// called to_json for Base
// called to_json for Obj
//
// Thanks to https://stackoverflow.com/a/64352583/7135482
