#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <array>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <span>
#include <valarray>
#include <tuple>
#include <string>

#include "pretty_view\pretty_view.hpp"

using namespace rmsn::pv;

template<typename T>
void test(const char *const name, const T& value) {
    std::cout << std::string(10, '-') << "\n";
    std::cout << "TEST " << name << "\n";
    std::cout << "1st way:\n" << pretty_view{value} << "\n";
    std::cout << "2st way:\n" <<  value << "\n";
    std::cout << std::string(10, '-') << "\n";
}

namespace {
    enum class gender {
        MALE,
        FEMALE
    };

    std::string to_string(const gender& gender) {
        switch (gender) {
            case gender::MALE:
                return "male";
            case gender::FEMALE:
                return "female";
            default:
                return "";
        }
    }

    class student {
    private:
        std::uint8_t age_;
        std::string name_;
        gender gender_;
        std::vector<std::uint16_t> marks_;

    public:
        template<typename String, typename Vector>
        requires std::is_convertible_v<String, std::string> && std::is_same_v<Vector, std::vector<std::uint16_t>>
        explicit student(const std::uint8_t age, String&& name, const gender gender, Vector&& marks) noexcept :
                age_{age}, name_{std::forward<String>(name)}, gender_{gender}, marks_{std::forward<Vector>(marks)}
        {}

        friend std::ostream& operator<<(std::ostream& os, const student& student) {
            os << "student[\n"
               << "\tage=" << +student.age_ << ",\n"
               << "\tname=" << student.name_ << ",\n"
               << "\tgender=" << to_string(student.gender_) << ",\n"
               << "\tmarks=" << student.marks_ << "\n"
               << "]";

            return os;
        }
    };
};

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    test("int arr[]", arr);

    test("vector<int>", std::vector{1, 2, 3});
    test("list<int>", std::list{1, 2, 3});
    test("forward_list<int>", std::forward_list<int>{1, 2, 3});
    test("deque<int>", std::deque{4, 5, 6});
    test("array<int,3>", std::array<int, 3>{7, 8, 9});
    test("valarray<int>", std::valarray<int>{10, 11, 12});

    test("set<int>", std::set{1, 3, 2});
    test("unordered_set<int>", std::unordered_set{4, 5, 6});

    test("map<int,string>", std::map<int, std::string>{{1, "hello"}, {2, "world"}});
    test("unordered_map<string,int>", std::unordered_map<std::string, int>{{"x", 1}, {"y", 2}});

    test("vector<string>", std::vector<std::string>{"hello", "world"});
    test("span<char>", std::span<const char>("abc", 3));

    test("tuple<int,double,const char*>", std::make_tuple(1, 2.5, "hi"));
    test("pair<int,string>", std::pair{10, "xx"});

    auto nested_tup = std::tuple{
            10,
            std::make_tuple(20, 30),
            std::pair{40, 50}
    };
    test("nested_tuple", nested_tup);

    std::vector<std::tuple<int, double, std::string>> vt = {
            {1, 2.5, "a"},
            {3, 4.5, "b"}
    };
    test("vector of tuples", vt);

    std::vector<std::vector<std::tuple<int, int>>> crazy = {
            {{1, 2}, {3, 4}},
            {{5, 6}, {7, 8}}
    };
    test("vector<vector<tuple>>", crazy);

    auto mega = std::make_tuple(
            std::vector{1, 2, 3},
            std::set<std::string>{"x", "y"},
            std::tuple<std::vector<int>, std::vector<std::vector<int>>>{
                    {5, 6},
                    {{7, 8}, {9, 10}}
            }
    );
    test("super nested structure", mega);

    std::map<int, std::tuple<int, std::vector<int>, std::string>> mega_map{
            {1, {10, {1, 2, 3}, "aaa"}},
            {2, {20, {4, 5},    "bbb"}}
    };
    test("map<int, tuple<...>>", mega_map);

    std::vector<std::tuple<
            int,
            std::vector<std::string>,
            std::tuple<std::vector<int>, std::string>
    >> insane = {
            {1, {"x", "y"}, {{1, 2, 3}, "lol"}}
    };
    test("insane nested tuple", insane);

    test("empty vector<int>", std::vector<int>{});
    test("empty list<string>", std::list<std::string>{});
    test("empty set<int>", std::set<int>{});
    test("empty tuple", std::tuple{});
    test("vector<tuple{}>", std::vector<std::tuple<>>{std::tuple{}, std::tuple{}});

    int raw[4] = {1, 2, 3, 4};
    std::span s(raw);
    test("std::span", s);

    std::vector<student> students;
    students.reserve(4);
    students.emplace_back(22, "Roman", gender::MALE, std::vector<std::uint16_t>{4, 5, 3, 4});
    students.emplace_back(22, "Vadim", gender::MALE, std::vector<std::uint16_t>{3, 5, 4});
    students.emplace_back(21, "Anna", gender::FEMALE, std::vector<std::uint16_t>{3, 4, 3, 2});
    students.emplace_back(18, "Vanrye", gender::MALE, std::vector<std::uint16_t>{5, 5, 5, 5, 5});
    test("vector<custom_type>", students);

    return 0;
}
