//#include <GINI/INI.h>

#include <string>
#include <SeVFS/VirtualFileSystem.h>

#include <SeResource/LocalizationSAGE.hpp>
#include <SeResource/LocalizationMMDOC.hpp>

const char* testTextRu = R"(
Base_Locale
"Русский"
End

TestEMPTY
End

TestStrEMPTY
""
End

Victory_Test
"Поздравляю!\n
Вы победили!"
End
)";

const char* testTextEng = R"(
Base_Locale
"English"
End

TestEMPTY
End

TestStrEMPTY
""
End

Victory_Test
"Congratulations!\n
You won!"
End
)";

class LocalizationTest : public Se::LocalizationFile
{
public:
    // 
    using Se::LocalizationFile::LocalizationFile;


    /// Load resource from custom format.
    bool Load(Se::Deserializer& source, Se::LocalizationFile::LocData& data) override {
        data.clear();
        return true; }

     /// Save resource to custom format.
    bool Save(Se::Serializer& source, const Se::LocalizationFile::LocData& data) const override { 
        return true; }   
};

void TestLangFile()
{
    using namespace Se;

    auto vfs = Se::VirtualFileSystem::Get();
    auto mountTest = vfs->MountDir("alias", "../TestGINI/");


    //INI ini;

    int loadType = 0;
    //ini.load("config.ini", INI_LOAD_OVERWRITE);

    {
        Localization loc;
        File f;
        SE_ASSERT(loc.Load<LocalizationTest>(f, "Russian") == true, "");
        SE_ASSERT(loc.Save<LocalizationTest>(f, "Russian") == true, "");
    }

    {
        Localization locSAGE;
        Se::MemoryBuffer memBuf(testTextRu, strlen(testTextRu));
        locSAGE.Load<SAGE::LocalizationSTR>(memBuf, "Russian");
        locSAGE.SetLanguage("Russian");

        SE_ASSERT(locSAGE.Get("Base_Locale") == "Русский", "");
        SE_ASSERT(locSAGE.Get("TestEMPTY") == "EMPTY_KEY: TestEMPTY", "");
        SE_ASSERT(locSAGE.Get("TestStrEMPTY") == "EMPTY_KEY: TestStrEMPTY", "");
        SE_ASSERT(locSAGE.Get("Victory_Test") == "Поздравляю!\\nВы победили!", "");

        // Switch language to English
        Se::MemoryBuffer memBufEng(testTextEng, strlen(testTextEng));
        locSAGE.Load<SAGE::LocalizationSTR>(memBufEng, "English");
        locSAGE.SetLanguage("English");

        SE_ASSERT(locSAGE.Get("Base_Locale") == "English", "");

        locSAGE.SetLanguage("Russian");
        SE_ASSERT(locSAGE.Get("Base_Locale") == "Русский", "");
    }

    {
        Localization locSAGE;

        auto fileRus = vfs->OpenFile({"alias", "Lang/Reborn5.05.rus/Generals.csf"}, FileMode::FILE_READ);
        locSAGE.Load<SAGE::LocalizationCSV>(*fileRus, "Russian");

        //

        locSAGE.SetLanguage("Russian");

        auto text = locSAGE.Get("DIALOGEVENT:Taunts_Toxin010Subtitle");
        SE_LOG_INFO("DIALOGEVENT:Taunts_Toxin010Subtitle: {}", text);
        text = locSAGE.Get("CONTROLBAR:ToolTipGLABuildSuperGun");
        SE_LOG_INFO("CONTROLBAR:ToolTipGLABuildSuperGun: {}", text);

        auto fileEng = vfs->OpenFile({"alias", "Lang/Reborn5.05.eng/generals.csf"}, FileMode::FILE_READ);
        locSAGE.Load<SAGE::LocalizationCSV>(*fileEng, "English");

        locSAGE.SetLanguage("English");

        text = locSAGE.Get("DIALOGEVENT:Taunts_Toxin010Subtitle");
        SE_LOG_INFO("Text: {}", text);

        // File f;
        // locSAGE.Load<LocalizationTest>(f, "Russian");
        // locSAGE.Load<LocalizationTest>(f, "English");

        // fileEng.reset();
        // fileRus.reset();
    }

    //Test BOF
    {
        auto  locMMDOC = std::make_unique<Localization>();
        auto mountMMDOD = vfs->MountDir("mmdoc", "../TestMMDOC/");

        //English
        auto fileEng = vfs->OpenFile({"mmdoc", "Localization/English.bof"}, FileMode::FILE_READ);
        locMMDOC->Load<MMDOC::LocalizationBOF>(*fileEng, "English");

        locMMDOC->SetLanguage("English");
        SE_ASSERT(locMMDOC->Get("Spe_Fir_090_Name") == "Fire Splash", "");

        //Russian
        auto fileRu = vfs->OpenFile({"mmdoc", "Localization/Russian.bof"}, FileMode::FILE_READ);
        locMMDOC->Load<MMDOC::LocalizationBOF>(*fileRu, "Russian");

        locMMDOC->SetLanguage("Russian");
        SE_ASSERT(locMMDOC->Get("Spe_Fir_090_Name") == "Всплеск пламени", "");

        fileRu.reset();

        vfs->Unmount(mountMMDOD);
    }

    vfs->Unmount(mountTest);

}