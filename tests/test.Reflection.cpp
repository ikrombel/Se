
#include "SeTest.hpp"

#include <SeResource/JSONFile.h>
#include <SeResource/JSONArchive.h>

#include <SeArc/Archive.hpp>
#include <SeArc/ArchiveSerialization.hpp>

#include <SeReflection/Reflected.hpp>

class ReflObjectInvalid : public Se::Reflected<ReflObjectInvalid>
{

};

class ReflObject0 //: public Se::Reflected<ReflObject0>
{
    Se::String name_;
    
public:
    ReflObject0() = default;

    int id_ = 0;
    float param2_{};

    float getParam1() const { return param1_; }
    void setParam1(const float& value) {
        param1_ = value;
    }

    bool isEnabled() const { return enabled_; }
    void setEnabled(const bool& value) {
        enabled_ = value;
    }

    void RegisterAttributes(Se::Attributes& attr) //override
    {
        attr.Register<int>("Id", &id_);
        attr.Register<Se::String>("Name", &name_, "Unnamed");
        // param1 wrapper by setter and getter methods
        attr.Register<float>("Param1", this,  &ReflObject0::getParam1, &ReflObject0::setParam1, 80.f);
        attr.Register<float>("Param2", &param2_, 2.f);
        attr.Register<bool>("Enabled", this, &ReflObject0::isEnabled, &ReflObject0::setEnabled, true);
    }

protected:
    

private:

    float param1_{};
    bool enabled_{};
};

class ReflObject3 : public ReflObject0
{
public:
    ReflObject3() : ReflObject0() {};

    void RegisterAttributes(Se::Attributes& attr); //final;
protected:
    

    Se::String param00_;
    int valInt0{};

};

void ReflObject3::RegisterAttributes(Se::Attributes& attr)
{
    //RegisterAttributesParrent<ReflObject0>("ReflObject0");
    ReflObject0::RegisterAttributes(attr);
    attr.Register<Se::String>("Param003", &param00_, "ReflObject3::Param003");
    attr.Register<int>("valInt0", &valInt0, 11);
    int yy = 0;
}





class ReflObject1 //: public Se::Reflected<ReflObject1>
{
    ReflObject0 object1_;
    
public:
    //ReflObject1() : Reflected() {}

    int id = 1;

    void SerializeInBlock(Se::Archive& archive) //override
    {
        auto refObj = std::make_shared<Se::Reflected<ReflObject0>>(&object1_);

        Se::SerializeValue(archive, "ReflObject_1", refObj);
        Se::SerializeValue(archive, "Id", id);
    }
};

namespace Se
{
    REGISTER_OBJECT_REFLECTED(ReflObject0);
    REGISTER_OBJECT_REFLECTED(ReflObject1);
    REGISTER_OBJECT_REFLECTED(ReflObject3);
} // namespace Se


void TestReflection() {

    // {
    //     auto file = std::make_shared<Se::JSONFile>();
    //     auto arc = Se::JSONOutputArchive(file.get());

    //     ReflObjectInvalid obj;
    //     Se::SerializeValue(arc, "ReflObject", obj);
    // }

    {
        auto file = std::make_shared<Se::JSONFile>();
        auto arc = Se::JSONOutputArchive(file.get());

        // Se::ReflectedManager::Register<ReflObject0>();
        // Se::ReflectedManager::Register<ReflObject3>();

        // Se::ReflectedManager::Register<ReflObject1>();

        auto obj0 = Se::ReflectedManager::Create("ReflObject0");
        SE_LOG_ERROR("+++ " + obj0->GetStaticType() );
        assert(obj0->GetType() == "ReflObject0"
            && obj0->GetStaticType() == "Se::ReflectedObject"
            );
        Se::SerializeValue(arc, "obj0", obj0);

        auto obj1 = Se::ReflectedManager::Create("ReflObject0");
        assert(obj1->GetType() == "ReflObject0"
            && obj1->GetStaticType() == "Se::ReflectedObject"
            );
        auto output = file->ToString("  ");
        // assert(output ==
        //     "{\n"
        //     "  \"Id\": 0,\n"
        //     "  \"Name\": \"test name\"\n"
        //     "}");

        // obj0->SetAttribute("Id", 0);
        int obj0_id = obj0->GetAttribute<int>("Id");
        assert(obj0_id == 0);

        obj0->SetAttribute("Id", 10);
        obj0_id = obj0->GetAttribute<int>("Id");
        assert(obj0_id == 10);
        
        //obj0->SetAttribute("Param2", 2.0f);
        assert(Se::Equals(obj0->GetAttribute<float>("Param2"), 2.0f)); 

        {   //Enabled
            bool obj0_enabled = obj0->GetAttribute<bool>("Enabled");
            assert(obj0_enabled == true);

            obj0->SetAttribute("Enabled", false);
            obj0_enabled = obj0->GetAttribute<bool>("Enabled");
            assert(obj0_enabled == false);

            Se::AttributeEmpty* attrEnabled =  obj0->FindAttribute("Enabled").get();
            assert(attrEnabled->GetTypeName() == "bool");

            assert(obj0->FindAttribute("Enabled0").get() == nullptr); //object has no attribute 'Enabled0'

        }
    }    

    {  
        auto fileT = std::make_shared<Se::JSONFile>();
        auto arcT = Se::JSONOutputArchive(fileT.get());
        
            //Param1
        auto objTest = Se::ReflectedManager::Create("ReflObject0");

        float objTest_param1;

        objTest_param1 = objTest->GetAttribute<float>("Param1");
        assert(Se::Equals(objTest_param1, 80.0f));

        // objTest->SetAttribute("Param1", 10.7);
        // auto valParam1 = objTest->GetAttribute<float>("Param1");
        // assert(Se::Equals(valParam1, 10.7f)); 

        objTest->SetAttribute("Param1", 10.5f);
        assert(Se::Equals(objTest->GetAttribute<float>("Param1"), 10.5f));

        Se::AttributeAccessor<float>* attrParam1 = reinterpret_cast<Se::AttributeAccessor<float>*>(objTest->FindAttribute("Param1").get());
        assert(Se::Equals(attrParam1->GetDefaultValue(), 80.0f));

        Se::AttributeEmpty* attrParam1_Info = objTest->FindAttribute("Param1").get();
        assert(!attrParam1_Info->IsDefault());

        objTest->SetAttribute("Param1", 80.f);
        assert(attrParam1_Info->IsDefault());

        objTest->SetAttribute("Id", 2);
        objTest->SetAttribute("Param1", 81.f);

        Se::SerializeValue(arcT, "obj0", objTest);
        SE_LOG_PRINT("-- {}", fileT->ToString("  "));

        SE_LOG_INFO("-- parrent:{} type:{}", objTest->GetTypeOrig(), objTest->GetType());     
    }

    {
        auto fileT = std::make_shared<Se::JSONFile>();
        auto arcT = Se::JSONOutputArchive(fileT.get());

        auto obj3 = Se::ReflectedManager::Create("ReflObject3");
        SE_LOG_INFO("-- parrent:{} type:{}", obj3->GetTypeOrig(), obj3->GetType());
        // SE_LOG_INFO("-- {}\n-- {}", typeid(decltype(obj00)).name(), Se::ToStringTypeId<decltype(obj00)>());
        // SE_LOG_INFO(".. {}", obj00->IsClassBase<ReflObject0>());


        // Se::String testType = typeid(std::map<int, std::vector<decltype(obj3)>>).name();
        // int status;
        // std::string demangledName = abi::__cxa_demangle(testType.c_str(), 0, 0, &status);
        // SE_LOG_INFO("00 {}", demangledName);

        Se::SerializeValue(arcT, "ReflObject3", obj3);
        SE_LOG_PRINT("{}", fileT->ToString("  "));

    }

    {
        auto fileT = std::make_shared<Se::JSONFile>();
        auto arcT = Se::JSONOutputArchive(fileT.get());

        auto obj3 = Se::Reflected<ReflObject3>(); 
        //std::make_shared<Se::Reflected<ReflObject3>>(); // Se::ReflectedManager::Create<ReflObject3>();
        //obj3->initAttributes();
        // SE_LOG_INFO("-- parrent:{} type:{}", obj3->GetTypeOrig(), obj3->GetType());
        // Se::SerializeValue(arcT, "ReflObject3", obj3);
        SE_LOG_PRINT("{}", fileT->ToString("  "));

    }

    {
        auto file = std::make_shared<Se::JSONFile>();
        auto arc = Se::JSONOutputArchive(file.get());

        

        auto obj0 = Se::Reflected<ReflObject1>();
        assert(obj0.GetType() == "ReflObject1"
            && obj0.GetStaticType() == "Se::ReflectedObject"
            );
        Se::SerializeValue(arc, "obj1", obj0);

        auto output = file->ToString("  ");
        // assert(output ==
        //     "{\n"
        //     "  \"Id\": 1,\n"
        //     "  \"ReflObject_1\": {\n"
        //     "    \"Id\": 0,\n"
        //     "    \"Name\": \"test name\"\n"
        //     "  }\n"
        //     "}");
    }
}