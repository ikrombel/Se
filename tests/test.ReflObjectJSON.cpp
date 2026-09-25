#include "Se/String.hpp"
#include "SeTest.hpp"

#include <SeResource/JSONValue.h>
#include <SeResource/JSONFile.h>

#include <SeReflection/Resource/ReflObjectJSON.hpp>
#include <SeReflection/Reflected.hpp>
#include <SeMath/MathDefs.hpp>

using namespace Se;

class TestJSONNested
{
public:
    int nestedInt_ = 42;
    float nestedFloat_ = 3.14f;

    template<typename TAttributes>
    void RegisterAttributes(TAttributes& attr)
    {
        attr.Register("NestedInt", &nestedInt_, 0);
        attr.Register("NestedFloat", &nestedFloat_, 0.0f);
    }

    bool operator==(const TestJSONNested& rhs) const
    {
        return nestedInt_ == rhs.nestedInt_
            && Se::Equals(nestedFloat_, rhs.nestedFloat_);
    }
};

class TestJSONRoot
{
public:
    TestJSONRoot() = default;

    int   intVal_     = 10;
    float floatVal_   = 20.5f;
    bool  boolVal_    = true;
    long long llVal_  = 100000000LL;
    String strVal_    = "hello";
    TestJSONNested nested_;

    template<typename TAttributes>
    void RegisterAttributes(TAttributes& attr)
    {
        attr.Register("IntVal", &intVal_, 0);
        attr.Register("FloatVal", &floatVal_, 0.0f);
        attr.Register("BoolVal", &boolVal_, false);
        attr.Register("LLVal", &llVal_, 0LL);
        attr.Register("StrVal", &strVal_, String("default"));
        attr.Register("Nested", &nested_);
    }
};

// bool ReflectionRegisterAttributes(TestJSONRoot* refl, Se::Attributes* attr)
// {
//     attr->Register("IntVal", &refl->intVal_, 0);
//     attr->Register("FloatVal", &refl->floatVal_, 0.0f);
//     attr->Register("BoolVal", &refl->boolVal_, false);
//     attr->template Register<long long>("LLVal", &refl->llVal_, 0LL);
//     attr->Register("StrVal", &refl->strVal_, String()"default"));
//     attr->Register("Nested", &refl->nested_);
// }

void TestParameterJSONInt()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ParameterJSON<int>\n"
                 "-------------------------------------------------------");

    int value = 42;
    JSONValue json;

    bool result = ParameterJSON<int>("MyInt", value, json, true);
    assert(result);
    assert(json.Get("MyInt").GetInt() == 42);

    int deserialized = 0;
    result = ParameterJSON<int>("MyInt", deserialized, json, false);
    assert(result);
    assert(deserialized == 42);
}

void TestParameterJSONFloat()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ParameterJSON<float>\n"
                 "-------------------------------------------------------");

    float value = 3.14f;
    JSONValue json;

    bool result = ParameterJSON<float>("MyFloat", value, json, true);
    assert(result);
    assert(json.Get("MyFloat").GetFloat() == 3.14f);

    float deserialized = 0.0f;
    result = ParameterJSON<float>("MyFloat", deserialized, json, false);
    assert(result);
    assert(Se::Equals(deserialized, 3.14f));
}

void TestParameterJSONBool()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ParameterJSON<bool>\n"
                 "-------------------------------------------------------");

    bool value = true;
    JSONValue json;

    bool result = ParameterJSON<bool>("MyBool", value, json, true);
    assert(result);
    assert(json.Get("MyBool").GetBool() == true);

    bool deserialized = false;
    result = ParameterJSON<bool>("MyBool", deserialized, json, false);
    assert(result);
    assert(deserialized == true);
}

void TestParameterJSONLongLong()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ParameterJSON<long long>\n"
                 "-------------------------------------------------------");

    long long value = 9876543210LL;
    JSONValue json;

    bool result = ParameterJSON<long long>("MyLL", value, json, true);
    assert(result);
    assert(static_cast<long long>(json.Get("MyLL").GetDouble()) == 9876543210LL);

    long long deserialized = 0;
    result = ParameterJSON<long long>("MyLL", deserialized, json, false);
    assert(result);
    assert(deserialized == 9876543210LL);
}

void TestParameterJSONString()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ParameterJSON<String>\n"
                 "-------------------------------------------------------");

    String value = "test string";
    JSONValue json;

    bool result = ParameterJSON<String>("MyStr", value, json, true);
    assert(result);
    assert(json.Get("MyStr").GetString() == "test string");

    String deserialized;
    result = ParameterJSON<String>("MyStr", deserialized, json, false);
    assert(result);
    assert(deserialized == "test string");
}

void TestParameterJSONUnknownType()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ParameterJSON<unknown type>\n"
                 "-------------------------------------------------------");

    struct UnknownType {};
    UnknownType value{};
    JSONValue json;

    bool result = ParameterJSON<UnknownType>("Unknown", value, json, true);
    assert(!result);
}

void TestReflObjectJSONRegister()
{
    // SE_LOG_PRINT("-------------------------------------------------------\n"
    //              "Test ReflObjectJSON::Register\n"
    //              "-------------------------------------------------------");

    // ReflObjectJSON::Register<int, float, bool, long long, String, TestJSONNested>(true);
    // reflJSON.Register<int, float, bool, long long, String, TestJSONNested>(true);

    // auto& reg = ReflObjectJSON::GetRegisterMap();
    // assert(reg.find(typeid(int).hash_code()) != reg.end());
    // assert(reg.find(typeid(float).hash_code()) != reg.end());
    // assert(reg.find(typeid(bool).hash_code()) != reg.end());
    // assert(reg.find(typeid(long long).hash_code()) != reg.end());
    // assert(reg.find(typeid(String).hash_code()) != reg.end());
    // assert(reg.find(typeid(TestJSONNested).hash_code()) != reg.end());
}

void TestReflObjectJSONToJSON()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ReflObjectJSON::ToJSON\n"
                 "-------------------------------------------------------");


    TestJSONRoot rootObj;
    rootObj.intVal_ = 99;
    rootObj.floatVal_ = 42.5f;
    rootObj.boolVal_ = false;
    rootObj.llVal_ = 1234567890123LL;
    rootObj.strVal_ = "ReflObjectJSON";
    rootObj.nested_.nestedInt_ = 777;
    rootObj.nested_.nestedFloat_ = 1.5f;

    ReflObjectJSON reflJSON;
    reflJSON.Register<int, float, bool, long long, String, TestJSONNested>(true);
    reflJSON.Link(&rootObj);

    JSONValue json = reflJSON.ToJSON();

    assert(json.Get("IntVal").GetInt() == 99);
    assert(Se::Equals(json.Get("FloatVal").GetFloat(), 42.5f));
    assert(json.Get("BoolVal").GetBool() == false);
    assert(static_cast<long long>(json.Get("LLVal").GetDouble()) == 1234567890123LL);
    assert(json.Get("StrVal").GetString() == "ReflObjectJSON");

    assert(json.Get("Nested").Get("NestedInt").GetInt() == 777);
    assert(Se::Equals(json.Get("Nested").Get("NestedFloat").GetFloat(), 1.5f));
}

void TestReflObjectJSONFromJSON()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ReflObjectJSON::FromJSON\n"
                 "-------------------------------------------------------");

    JSONValue json;
    json["IntVal"] = 55;
    json["FloatVal"] = 99.9f;
    json["BoolVal"] = true;
    json["LLVal"] = static_cast<double>(9876543210LL);
    json["StrVal"] = "deserialized";
    json["Nested"]["NestedInt"] = 321;
    json["Nested"]["NestedFloat"] = 6.28f;

    ReflObjectJSON reflJSON;
    reflJSON.Register<int, float, bool, long long, String, TestJSONNested>(true);
    auto obj = reflJSON.FromJSON<TestJSONRoot>(json);

    assert(obj != nullptr);
    assert(obj->intVal_ == 55);
    assert(Se::Equals(obj->floatVal_, 99.9f));
    assert(obj->boolVal_ == true);
    assert(obj->llVal_ == 9876543210LL);
    assert(obj->strVal_ == "deserialized");
    assert(obj->nested_.nestedInt_ == 321);
    assert(Se::Equals(obj->nested_.nestedFloat_, 6.28f));
}

void TestReflObjectJSONRoundTrip()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ReflObjectJSON Round Trip\n"
                 "-------------------------------------------------------");


    TestJSONRoot rootObj;
    rootObj.intVal_ = 123;
    rootObj.floatVal_ = 77.7f;
    rootObj.boolVal_ = true;
    rootObj.llVal_ = 555555555555LL;
    rootObj.strVal_ = "roundtrip";
    rootObj.nested_.nestedInt_ = 888;
    rootObj.nested_.nestedFloat_ = 9.99f;

    ReflObjectJSON reflJSON;
    reflJSON.Link(&rootObj);
    reflJSON.Register<int, float, bool, long long, String, TestJSONNested>(true);


    JSONValue json = reflJSON.ToJSON();
    auto obj = reflJSON.FromJSON<TestJSONRoot>(json);

    assert(obj != nullptr);
    assert(obj->intVal_ == 123);
    assert(Se::Equals(obj->floatVal_, 77.7f));
    assert(obj->boolVal_ == true);
    assert(obj->llVal_ == 555555555555LL);
    assert(obj->strVal_ == "roundtrip");
    assert(obj->nested_.nestedInt_ == 888);
    assert(Se::Equals(obj->nested_.nestedFloat_, 9.99f));
}

void TestReflObjectJSONDefaults()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ReflObjectJSON Defaults\n"
                 "-------------------------------------------------------");

    TestJSONRoot rootObj;
    rootObj.intVal_ = 7;
    rootObj.floatVal_ = 1.0f;
    rootObj.boolVal_ = true;
    rootObj.llVal_ = 42LL;
    rootObj.strVal_ = "test";
    rootObj.nested_.nestedInt_ = 100;
    rootObj.nested_.nestedFloat_ = 2.0f;

    ReflObjectJSON reflJSON;
    reflJSON.Register<int, float, bool, long long, String, TestJSONNested>(true);
    reflJSON.Link(&rootObj);

    JSONValue json = reflJSON.ToJSON();
    JSONFile file;

    file.GetRoot() = json;
    printf("%s", file.ToString("  ").c_str());
        

    TestJSONRoot defaults{};
    assert(defaults.intVal_ == 10);
    assert(Se::Equals(defaults.floatVal_, 20.5f));
    assert(defaults.boolVal_ == true);
    assert(defaults.llVal_ == 100000000LL);
    assert(defaults.strVal_ == "hello");
    assert(defaults.nested_.nestedInt_ == 42);

    JSONValue emptyJson;
    auto emptyObj = reflJSON.FromJSON<TestJSONRoot>(emptyJson);
    assert(emptyObj != nullptr);
    assert(emptyObj->intVal_ == 10);
    assert(Se::Equals(emptyObj->floatVal_, 20.5f));
    assert(emptyObj->boolVal_ == true);
    assert(emptyObj->llVal_ == 100000000LL);
    assert(emptyObj->strVal_ == "hello");
    assert(emptyObj->nested_.nestedInt_ == 42);
}

void TestReflObjectJSONEmpty()
{
    SE_LOG_PRINT("-------------------------------------------------------\n"
                 "Test ReflObjectJSON Empty\n"
                 "-------------------------------------------------------");

    ReflObjectJSON reflJSON;
    reflJSON.Register<int, float, bool, long long, String, TestJSONNested>(true);
    JSONValue json = reflJSON.ToJSON();
    assert(json.IsNull());

    JSONValue emptyJson;
    auto obj = reflJSON.FromJSON<TestJSONRoot>(emptyJson);
    assert(obj != nullptr);
    assert(obj->intVal_ == 10);
    assert(obj->strVal_ == "hello");
}

void TestReflObjectJSON()
{
    TestParameterJSONInt();
    TestParameterJSONFloat();
    TestParameterJSONBool();
    TestParameterJSONLongLong();
    TestParameterJSONString();
    TestParameterJSONUnknownType();

    TestReflObjectJSONRegister();
    TestReflObjectJSONToJSON();
    TestReflObjectJSONFromJSON();
    TestReflObjectJSONRoundTrip();
    TestReflObjectJSONDefaults();
    TestReflObjectJSONEmpty();

    SE_LOG_PRINT("All ReflObjectJSON tests passed!");
}
