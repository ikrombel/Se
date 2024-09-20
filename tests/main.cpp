#include <iostream>
#include <vector>

#include <unordered_map>

#include <Se/Algorithms.hpp>
#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>
#include <Se/Signal.hpp>
#include <functional>

#include <SeResource/JSONArchive.h>
#include <SeArc/Archive.hpp>
#include <SeArc/ArchiveSerialization.hpp>
#include <SeResource/JSONFile.h>

#include <SeMath/Vector4.hpp>
#include <SeMath/Quaternion.hpp>
#include <SeMath/Matrix4.hpp>
#include <SeMath/BoundingBox.hpp>
#include <SeMath/Frustum.hpp>

#include <SeReflection/Reflected.hpp>



#include <Se/Debug.h>

template<class T>
T reverseBuff(T value) {
    T buf;
    const unsigned char *s = (const unsigned char*)&value;
    unsigned char *d = (unsigned char*)&buf + sizeof(T) - 1;
    for (size_t i = 0; i < sizeof(T); i++) {
        *d-- = *s++;
    }
    return buf;
}

template<class T>
bool readArrayReverse(T& dest, std::size_t size) {
    for (std::size_t i = 0; i < size; i++) {
        dest[i] = reverseBuff(dest[i]);
    }
    return true;
}

struct Vector3_Test {
    int x_ = 0;
    int y_ = 2;
    int z_ = 3;
};

//wrapper
namespace Se {
inline void SerializeValue(Archive& archive, const char* name, Vector3_Test& value)
{
    //LibSTD::SerializeVectorAsObjects(archive, name, value);
    //Detail::SerializeAsString<Vector2>(archive, name, value);
    ArchiveBlock block = archive.OpenUnorderedBlock(name);
    SerializeValue(archive, "x", value.x_);
    SerializeValue(archive, "y", value.y_);
    SerializeValue(archive, "z", value.z_);
}
}

struct TestSerialization {

    Se::String name_= "TextName";
    int id_ = 10;
    float float_ = 20.3f;

    //std::unordered_map<Se::String, Se::String> variables_;

    TestSerialization() {};
 
    std::vector<int> stdVectorInt_ = {0, 2, 4, 6};

    std::map<Se::String, int> stdStringInt_ = {{"test0", 1}, {"test2", 3}, {}};

    Vector3_Test position_;

    void SerializeInBlock(Se::Archive& archive) {
        SerializeValue(archive, "Name", name_);
        SerializeValue(archive, "ValueInt", id_);
        SerializeValue(archive, "ValueFloat", float_);
        SerializeValue(archive, "Position", position_);
        // SerializeValue(archive, "VariantOptionalValue", variables_);
        // SerializeValue(archive, "stdVectorInt", stdVectorInt_);
        // SerializeValue(archive, "stdStringInt", stdStringInt_);
        
    }
};



//TEST_CASE(TestArchive, "IO/Arhive", {
void TestArch() {
    auto file = std::make_shared<Se::JSONFile>();
    auto arc = Se::JSONOutputArchive(file.get());

    TestSerialization test1;
    

    // test1.variables_.insert("val1", 10);
    // test1.variables_.insert("val2", 9.9);
    // test1.variables_.insert("val3", "Test");
    //test1.variables_.insert("val4", Vector3{0.1, 0.2, 0.3});
    SerializeValue(arc, "Flavor", test1);
    


    auto file2 = std::make_shared<Se::JSONFile>();
    auto arc2 = Se::JSONInputArchive(file.get());

    file2->GetRoot() = file->GetRoot();

    TestSerialization test2;
    Se::SerializeValue(arc2, "Flavor", test2);

    

    // Vector3 var0;
    // var0 = test2.variables_["val4"].GetVector3();
    // PrintLine("-- " + var0.ToString());

    auto output = file->ToString("  ");
    SE_LOG_PRINT("{}", output.c_str());

    SE_LOG_DEBUG("{}", "Debug Test");
    SE_LOG_WARNING("{}", "Warning Test");
    SE_LOG_WARNING("{}", "Warning Test2");
    SE_LOG_INFO("{}", "Info Test");
    SE_LOG_ERROR("{}", "Error Test");
    SE_LOG_ERROR("{}", "Error Test 2");


    {
        //std::vector<int> testErase = { 0, 1, 2, 3, 2, 4, 5, 2};
        std::vector<int> testErase = {  };

        Se::String printStr = "----------------------\n";
        for (auto a : testErase)
            printStr += cformat(" %i", a);
        SE_LOG_INFO(printStr);


        Se::EraseIf(testErase, [](int item){
            return item ==2;
        });

        printStr = "----------------------\n";
        for (auto a : testErase)
            printStr += cformat(" %i", a);
        SE_LOG_INFO(printStr);
    }
//});
}



class ReflObjectInvalid : public Se::Reflected<ReflObjectInvalid>
{

};

class ReflObject0 : public Se::Reflected<ReflObject0>
{
    Se::String name = "test name";
    int id = 1;
public:
    void SerializeInBlock(Se::Archive& archive) override {
        Se::SerializeValue(archive, "Name", name);
        Se::SerializeValue(archive, "Id", id);
    }
};

void TestReflection() {

    {
        auto file = std::make_shared<Se::JSONFile>();
        auto arc = Se::JSONOutputArchive(file.get());

        ReflObjectInvalid obj;
        Se::SerializeValue(arc, "ReflObject", obj);
    }

    {
        auto file = std::make_shared<Se::JSONFile>();
        auto arc = Se::JSONOutputArchive(file.get());

        Se::ReflectedManager::Register<ReflObject0>();

        auto obj0 = Se::ReflectedManager::Create("ReflObject0");
        auto obj1 = Se::ReflectedManager::Create<ReflObject0>();

        SE_LOG_ERROR("obj0: {} {}", obj0->GetType(), obj0->GetStaticType());
        SE_LOG_ERROR("obj1: {} {}", obj1->GetType(), obj1->GetStaticType());
        //ReflObject0& obj = *(obj0.get());
        Se::SerializeValue(arc, "obj0", obj0);

        auto output = file->ToString("  ");
        SE_LOG_PRINT("{}", output.c_str());
    }
}

int main() {

    // Se::Debug debug;
    // auto onLog = debug.onLog();
    Console::setOutputLog(Console::DefaultColored);//, onLog);

    int64_t f = 0x000457dd;

    f = reverseBuff(f);
    printf("0x%016lx\n", f);

    f = reverseBuff(f);
    printf("0x%016lx\n", f);

    f = reverseBuff(0x7804dd00);
    printf("0x%016lx\n", f);

    //std::vector<int> ar0 = {0x000457dd , 0x7804dd00};
    unsigned ar0[] = {0x000457dd , 0x7804dd00};

    for (auto a : ar0)
        printf("  0x%08x\n", a);

    readArrayReverse(ar0, 2); 

    for (auto a : ar0)
        printf("r 0x%08x\n", a);

    // std::string str = "0123456789";
    // std::size_t str_size = str.length(); //strlen(str);
    // printf("  ");
    // for (auto i = 0; i < str_size; i++)
    //     printf("%02x.", str[i]);
    // printf("\n");

    // readArrayReverse(str, str_size);
    
    // printf("r ");
    // for (auto i = 0; i < str_size; i++)
    //     printf("%02x.", str[i]);
    // printf("\n");

    // Se::File file("Tests/render.scxx");
    // std::string fileData = file.ReadText();
    // printf("%s\n", fileData.c_str());

    TestArch();

    {
        Se::EventSystem eventSystem;

        // Register a slot for event type 1 with int and float arguments
        eventSystem.registerEvent<int, float>(1, [](int x, float y) {
            SE_LOG_INFO("Received event 1 with args: {}, {}", x, y);
        });

        // Register another slot for event type 1 with int and float arguments
        eventSystem.registerEvent<int, float>(1, [](int x, float y) {
            SE_LOG_INFO("Another slot received event 1 with args: {}, {}", x, y);
        });

        // Emit event type 1 with int and float arguments
        eventSystem.emitEvent<int, float>(1, 42, 3.14f);
    }

    Se::String output;
    Se::FileSystem::Get().SystemRun("echo \"test\"", {}, output);

    Se::FileSystem::Get().SystemRun("echo \"test 2\"", {}, output);
    //auto d = Debug::gDebug();

    Se::Vector4 vec = Se::Vector4::ONE;

    Se::Quaternion qua;
    Se::Matrix4 mat4;
    Se::BoundingBox box;
    Se::Frustum fFrustum;


    TestReflection();

    
}