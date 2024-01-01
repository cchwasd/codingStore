
#include <iostream>
#include <string>
#include <fstream>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// 1. 定义JSON数值类型
void func_json01()
{

    json j;                                                // 首先创建一个空的json对象
    j["pi"] = 3.141;                                       // 然后通过名称/值对的方式进行初始化，此时名称"pi"对应的数值就是3.141
    j["happy"] = true;                                     // 将名称"happy"赋值为true
    j["name"] = "Niels";                                   // 将字符串"Niels"存储到"name"
    j["nothing"] = nullptr;                                //"nothing"对应的是空指针
    j["answer"]["everything"] = 42;                        // 对对象中的对象进行初始化
    j["list"] = {1, 0, 2};                                 // 使用列表初始化的方法对"list"数组初始化
    j["object"] = {{"currency", "USD"}, {"value", 42.99}}; // 对对象进行初始化

    // 注意： 在进行如上方式构造JSON对象时，你并不需要告诉编译器你要使用哪种类型，nlohmann会自动进行隐式转换。
    // 格式化打印json
    std::cout << j.dump(4) << std::endl;

    json j2 = {
        {"pi", 3.141},
        {"happy", true},
        {"name", "Niels"},
        {"nothing", nullptr},
        {"answer", {{"everything", 42}}},
        {"list", {1, 0, 2}},
        {"object", {{"currency", "USD"}, {"value", 42.99}}}
    };

    std::cout << std::setw(4) << j2 << std::endl;

    // 显式定义或者表达一些情况
    json empty_array_explicit = json::array();                                    // 初始化一个JSON格式的空数组
    json empty_object_implicit = json({});                                        // 隐式定义一个空对象
    json empty_object_explicit = json::object();                                  // 显式定义一个空对象
    json array_not_object = json::array({{"currency", "USD"}, {"value", 42.99}}); // 显式定义并初始化一个JSON数组
}

// 2.从STL容器转换到json
void func_json02()
{
    std::vector<int> c_vector{1, 2, 3, 4};
    json j_vec(c_vector);
    // [1, 2, 3, 4]

    std::deque<double> c_deque{1.2, 2.3, 3.4, 5.6};
    json j_deque(c_deque);
    // [1.2, 2.3, 3.4, 5.6]

    std::list<bool> c_list{true, true, false, true};
    json j_list(c_list);
    // [true, true, false, true]

    std::forward_list<int64_t> c_flist{12345678909876, 23456789098765, 34567890987654, 45678909876543};
    json j_flist(c_flist);
    // [12345678909876, 23456789098765, 34567890987654, 45678909876543]

    std::array<unsigned long, 4> c_array{{1, 2, 3, 4}};
    json j_array(c_array);
    // [1, 2, 3, 4]

    std::set<std::string> c_set{"one", "two", "three", "four", "one"};
    json j_set(c_set); // only one entry for "one" is used
    // ["four", "one", "three", "two"]

    std::unordered_set<std::string> c_uset{"one", "two", "three", "four", "one"};
    json j_uset(c_uset); // only one entry for "one" is used
    // maybe ["two", "three", "four", "one"]

    std::multiset<std::string> c_mset{"one", "two", "one", "four"};
    json j_mset(c_mset); // both entries for "one" are used
    // maybe ["one", "two", "one", "four"]

    std::unordered_multiset<std::string> c_umset{"one", "two", "one", "four"};
    json j_umset(c_umset); // both entries for "one" are used
    // maybe ["one", "two", "one", "four"]

    std::map<std::string, int> c_map{{"one", 1}, {"two", 2}, {"three", 3}};
    json j_map(c_map);
    // {"one": 1, "three": 3, "two": 2 }

    std::unordered_map<const char *, double> c_umap{{"one", 1.2}, {"two", 2.3}, {"three", 3.4}};
    json j_umap(c_umap);
    // {"one": 1.2, "two": 2.3, "three": 3.4}

    std::multimap<std::string, bool> c_mmap{{"one", true}, {"two", true}, {"three", false}, {"three", true}};
    json j_mmap(c_mmap); // only one entry for key "three" is used
    // maybe {"one": true, "two": true, "three": true}

    std::unordered_multimap<std::string, bool> c_ummap{{"one", true}, {"two", true}, {"three", false}, {"three", true}};
    json j_ummap(c_ummap); // only one entry for key "three" is used
    // maybe {"one": true, "two": true, "three": true}
}

// 3.string序列化和反序列化
void func_json03()
{
    // 反序列化：从字节序列恢复JSON对象。
    json j = "{ \"happy\": true, \"pi\": 3.141, \"list\": [1,2,3] }"_json;
    auto j2 = R"({"happy": true, "pi": 3.141, "list": [1,2,3] })"_json;
    auto j3 = json::parse(R"({"happy": true, "pi": 3.141, "list": [1,2,3]})");
    // 序列化：从JSON对象转化为字节序列。
    std::string s = j2.dump(); // {"happy":true,"pi":3.141}
    std::cout << j.dump(4) << std::endl;
    std::cout << s << std::endl;
}

// 4.stream的序列化和反序列化
void func_json04()
{
    // json j;         // {"happy": true, "pi": 3.141, "list": [1,2,3]}
    // std::cin >> j; // 从标准输入中反序列化json对象

    // std::cout << j; // 将json对象序列化到标准输出中
    //  {"happy":true,"list":[1,2,3],"pi":3.141}

    //读取一个json文件，nlohmann会自动解析其中数据
    std::ifstream ifs("resources/file_read.json");
    json j;
    ifs >> j;

    //以易于查看的方式将json对象写入到本地文件
    std::ofstream ofs("resources/file_write.json");
    ofs << std::setw(4) << j << std::endl;

}

namespace ns {
    //首先定义一个结构体
    struct Person {
        std::string name;
        std::string address;
        int age;

        //<<运算符重载
        friend std::ostream& operator<<(std::ostream &os, const Person &p){
            os << "Person(name:" << p.name<<", age:"<< p.age << ", address:" << p.address <<")"<< std::endl;
            return os;
        }
    };

    void to_json(json &j, const Person &p) {
        j = json{{"name",  p.name},
                 {"address", p.address},
                 {"age", p.age}};
    }

    void from_json(const json &j, Person &p) {
        j.at("name").get_to(p.name);
        j.at("address").get_to(p.address);
        j.at("age").get_to(p.age);
    }

}

// 5. 任意类型转换
void func_json05()
{
    // 对于某一种任意数据类型，可以使用如下方式转换：
    ns::Person p = {"Ned Flanders", "744 Evergreen Terrace", 60};//定义初始化p

    //从结构体转换到json对象
    json j;
    j["name"] = p.name;
    j["address"] = p.address;
    j["age"] = p.age;

    std::cout << std::setw(4) << j << std::endl;
    //从json对象转换到结构体
    ns::Person p2 {
        j["name"].get<std::string>(),
        j["address"].get<std::string>(),
        j["age"].get<int>()
    };
    std::cout << p2 << std::endl;

    // nlohmann提供了更为方便的方式：
    p = {"Mary Cui", "205 Evergreen Terrace", 27};//定义初始化p

    // ns::to_json(j, p);
    j=p;
    std::cout << std::setw(4) << j << std::endl;
    j["name"]="Birddle Cui";

    // ns::from_json(j,p);
    // p=j;
    p = j.template get<ns::Person>();
    std::cout << p << std::endl;
}

// 6.建议使用显式类型转换
void func_json06()
{
    // strings
    std::string s1 = "Hello, world!";
    json js = s1;
    auto s2 = js.template get<std::string>();

    // 隐式类型转换 NOT RECOMMENDED 
    std::string s3 = js;
    std::string s4;
    s4 = js;

    // Booleans
    bool b1 = true;
    json jb = b1;
    auto b2 = jb.template get<bool>();
    
    // NOT RECOMMENDED
    bool b3 = jb;
    bool b4;
    b4 = jb;

    // numbers
    int n1 = 42;
    json jn = n1;
    auto n2 = jn.template get<int>();
    // NOT RECOMMENDED
    int n3 = jn;
    int n4;
    n4 = jn;

    // double
    double d1 = 24.42;
    json jd = d1;
    auto d2 = jn.template get<double>();
    // NOT RECOMMENDED
    double d3 = jd;
    int d4;
    d4 = jd;

    // etc.
}

// 7.转换JSON到二进制格式
void func_json07()
{
   // create a JSON value
    json j = R"({"compact": true, "schema": 0})"_json;

    // serialize to BSON
    std::vector<std::uint8_t> v_bson = json::to_bson(j);

    // 0x1B, 0x00, 0x00, 0x00, 0x08, 0x63, 0x6F, 0x6D, 0x70, 0x61, 0x63, 0x74, 0x00, 0x01, 0x10, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    // roundtrip
    json j_from_bson = json::from_bson(v_bson);

    // serialize to CBOR
    std::vector<std::uint8_t> v_cbor = json::to_cbor(j);

    // 0xA2, 0x67, 0x63, 0x6F, 0x6D, 0x70, 0x61, 0x63, 0x74, 0xF5, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x00

    // roundtrip
    json j_from_cbor = json::from_cbor(v_cbor);

    // serialize to MessagePack
    std::vector<std::uint8_t> v_msgpack = json::to_msgpack(j);

    // 0x82, 0xA7, 0x63, 0x6F, 0x6D, 0x70, 0x61, 0x63, 0x74, 0xC3, 0xA6, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x00

    // roundtrip
    json j_from_msgpack = json::from_msgpack(v_msgpack);

    // serialize to UBJSON
    std::vector<std::uint8_t> v_ubjson = json::to_ubjson(j);

    // 0x7B, 0x69, 0x07, 0x63, 0x6F, 0x6D, 0x70, 0x61, 0x63, 0x74, 0x54, 0x69, 0x06, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x69, 0x00, 0x7D

    // roundtrip
    json j_from_ubjson = json::from_ubjson(v_ubjson);

    //如果需要写到本地，可以使用如下方式：
    std::string path = "resources/json_bin.txt";
    json jb = json::parse(R"({"happy": true, "pi": 3.141, "list": [1,2,3]})");
    std::ofstream ofs(path, std::ios::out | std::ios::binary);
    const auto msgpack = nlohmann::json::to_msgpack(jb);
    ofs.write(reinterpret_cast<const char*>(msgpack.data()), msgpack.size() * sizeof(uint8_t));
    ofs.close();

}

int main(int argc, char *argv[])
{
    // func_json01();
    // func_json02();
    // func_json03();
    // func_json04();
    // func_json05();
    // func_json06();
    func_json07();

    return 0;
}