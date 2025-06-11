#include <Se/IO/MemoryBuffer.hpp>
#include <SeResource/YAMLFile.h>
#include <SeResource/JSONFile.h>

using namespace Se;

const String yaml0 = R"(
assetType: Texture
id: 8048001347822831655
testDouble: 3.14159265358979323846
testBool: true
testStr: long text with spaces and special characters like !@#$%^&*()_+{}|:"<>?`~ and more.
path: left.jpg
faces:
  left: 8048001347822831655
  right: 5141921474735944922
  top: 15306523284328498852
  bottom: 15566947957651972671
  front: 14188265351994701033
  back: 14057120215578723615
vector2: {x: 1.0, y: 2.0}
vector3: vector3(1.0, 2.0, 3.0)
  
)";



void TestYAMLFile() 
{
    std::cout << "-------------------------------------------------------" << std::endl 
              << "Test YAMLFile" << std::endl
              << "-------------------------------------------------------" << std::endl;

    MemoryBuffer buffer(yaml0.c_str(), yaml0.size());

    YAMLFile yamlFile;
    yamlFile.Load(buffer);

    auto object = yamlFile.GetRoot().GetObject();

    // std::vector<std::pair<String, Value>> pairs(object.begin(), object.end());
    // std::sort(pairs.begin(), pairs.end(), [](const auto& lval, const auto& rval){
    //     return lval.first < rval.first;
    // });

    // Check if the file loaded correctly
    if (yamlFile.GetRoot().Size() == 0) {
        SE_LOG_ERROR("Failed to load YAML file.");
        return;
    }

    // Access the root value
    JSONValue& root = yamlFile.GetRoot();
    std::cout << "Root value:\n" << yamlFile.ToString() << std::endl;

    JSONFile jsonFile;
    jsonFile.GetRoot() = root;

    std::cout << "JSON:\n" << jsonFile.ToString("  ") << std::endl;


    assert(root["assetType"].GetString() == "Texture");
//    assert(root["id"].GetUInt64() == 8048001347822831655);
    assert(root["path"].GetString() == "left.jpg");

    // Example of accessing a specific key
    if (root.Contains("assetType")) {
        std::cout << "Value for 'assetType': " << root["assetType"].GetString() << std::endl;
    } else {
        std::cout << "'assetType' not found in YAML file." << std::endl;
    }


}