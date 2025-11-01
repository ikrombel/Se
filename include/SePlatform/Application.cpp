#include "Application.h"

#include <Se/Console.hpp>
#include <Se/Debug.h>
#include <Se/IO/FileSystem.h>

#include <SeArc/ArchiveSerialization.hpp>
#include <SeResource/JSONArchive.h>

#include <SeVFS/VirtualFileSystem.h>
#include <SeVFS/MountedDirectory.h>

#include <SeResource/JSONArchive.h>

namespace Se {

bool FileIsChanged(String filePath)
{
    auto fs = FileSystem::Get();    

    if (fs.FileExists(filePath))
        return true;

    auto fileT = std::make_shared<JSONFile>();
    fileT->LoadFile(filePath);
    fileT->ToString("");

    return true;

    //SE_LOG_DEBUG("Path: \n{}", path);
    //SE_LOG_DEBUG("FileIsChanged: {}", FullFilePath({"cfg", "test.txt"}));
    // Se::File checkFile;
    // if (!checkFile.Open(file, Se::FileMode::FILE_READ))
    //     return true;

    return false;

}


bool Config::Save()
{
#ifdef SE_DESKTOP
    String fileConfig = Application::Get()->GetProgramPrefDir() + "Application.json";
    
    auto fs = FileSystem::Get();

    JSONValue objToCompare;

    if (fs.FileExists(fileConfig))
    {
        File checkFile(fileConfig, FileMode::FILE_READ);
        auto fileToCheck = std::make_shared<JSONFile>();
        fileToCheck->Load(checkFile);
        objToCompare = fileToCheck->GetRoot();
    }

    auto fileT = std::make_shared<JSONFile>();
    auto arcT = JSONOutputArchive(fileT.get());
    SerializeValue(arcT, "Settings", *app_);

    if (JSONValue::Compare(fileT->GetRoot(), objToCompare))
        return true;

    File outputFile(fileConfig, FileMode::FILE_WRITE);
    return fileT->Save(outputFile);
    //return fileT->SaveFile( Application::Get()->GetProgramPrefDir() + "Application.json");
#else
    return true;
#endif

    
}

bool Config::Load()
{
#ifdef SE_DESKTOP
    String fileConfig = Application::Get()->GetProgramPrefDir() + "Application.json";
    auto fs = FileSystem::Get();
    if (!fs.FileExists(fileConfig))
        return false;

    File inputFile(fileConfig, FileMode::FILE_READ);

    auto fileT = std::make_shared<JSONFile>();
    auto arcT = JSONInputArchive(fileT.get());
    //if (!fileT->LoadFile(Application::Get()->GetProgramPrefDir() + "Application.json"))
    if (!fileT->Load(inputFile))
        return false;
    SerializeValue(arcT, "Settings", *app_);
#endif
    return true;
}

Application::~Application()
{

}

void Application::SerializeInBlock(Archive& archive)
{
    SerializeOptionalValue(archive, "Window", window_);
    int testVal = 150;
    SerializeOptionalValue(archive, "testVal", testVal);
}

void Application::Init()
{
    auto debugLog = gDebug().onLog();
    Console::setOutputLog(Console::DefaultColored, debugLog);

    auto fs = FileSystem::Get();
    auto vfs = VirtualFileSystem::Get();

    // cfg_ = std::make_shared<Config>(this);

    // String xdg = fs.GetENV("HOME");
    // SE_LOG_WARNING(xdg);

#ifndef ANDROID 
    pathProgramPref = fs.GetAppPreferencesDir("SE", appName_);
    if (!fs.DirExists(pathProgramPref))
            fs.CreateDirsRecursive(pathProgramPref);
#endif


    // vfs->MountDir("cfg", pathProgramPref);

    //pathProgramPref = String(path.c_str(), path.length());
    //mDirGfg_ = std::dynamic_pointer_cast<MountedDirectory>(vfs->MountDir("cfg", path));

    SE_LOG_INFO("AppPreferencesDir: {}", pathProgramPref);

//    InitRenderer();

    // Window::onWindowClose.connect([this](bool& close){
    //     cfg_->Save();
    // });


    window_ = Window::Get();

    //cfg_->Load();

    window_->ApplyConfigs();
}

void Application::Run() {
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    //io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!window_->WindowShouldClose())
#endif
    {
        window_->Render();
#ifndef ANDROID //TODO remove this for more framerate
        window_->Sleep(25);
#endif
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif
}

}