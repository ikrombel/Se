project(Se)

include(cmake/SeOutputDirectoryStyle.cmake)

set (CMAKE_CXX_STANDARD 17)

option(HAVE_LZ4 "LZ4 Compression" ON)
option(SE_THREADING "" ON)
option(SE_FILEWATCHER "" ON)

if (SE_FILEWATCHER)
        set(SE_DEFINED ${SE_DEFINED}
                PUBLIC SE_FILEWATCHER
        )
endif()

if (SE_THREADING)
        set(SE_DEFINED ${SE_DEFINED}
                PUBLIC SE_THREADING
        )
endif()

set(SE_INCLUDE_DIRS )

set(SE_SOURCE
        src/Se/Debug.cpp
        src/Se/IO/File.cpp
        src/Se/IO/FileSystem.cpp
        src/Se/IO/PackageFile.cpp
        src/Se/IO/PackageFile.Tool.cpp
        src/Se/Thread.cpp
        src/Se/Timer.cpp
        src/Se/ProcessTaskManager.cpp
        src/Se/VectorBuffer.cpp
        src/Se/WorkQueue.cpp
        )

if (HAVE_LZ4) 
        set(SE_DEFINED ${SE_DEFINED}
                PRIVATE HAVE_LZ4
        )
        set(SE_SOURCE ${SE_SOURCE}
                src/deps/LZ4/lz4hc.c
                src/deps/LZ4/lz4.c
        )

        set(SE_INCLUDE_DIRS ${SE_INCLUDE_DIRS}
                PRIVATE src/deps/LZ4/include
                PRIVATE src/deps/LZ4/include/LZ4
        )
        
endif()

add_library(Se STATIC ${SE_SOURCE})

target_include_directories(Se PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE include/Se/
        PRIVATE include/Se/IO/
        ${SE_INCLUDE_DIRS}
)

target_compile_definitions(Se 
        ${SE_DEFINED}
        PUBLIC NOMINMAX
        PUBLIC SE_STATIC_LIB
        )
# target_compile_features(Se PRIVATE cxx_std_20)

if (WIN32)
        target_link_libraries(Se PRIVATE user32 winmm)
endif()

add_library(SeResource STATIC
        src/SeResource/ETCPACK/source/etcdec.cxx
        src/SeResource/ETCPACK/source/image.cxx

        src/SeResource/Decompress.cpp
        src/SeResource/Decompress.h
        src/SeResource/Image.cpp
        src/SeResource/ImageCube.cpp
        src/SeResource/Resource.cpp

        src/SeResource/JSONValue.cpp
        src/SeResource/JSONArchive.cpp
        src/SeResource/JSONFile.cpp

        src/SeResource/BinaryArchive.cpp 
        src/SeResource/Base64Archive.cpp

        src/SeResource/YAMLFile.cpp


        src/SeResource/BackgroundLoader.cpp
        src/SeResource/ResourceCache.cpp
        src/SeResource/ResourceCache.reg.cpp

        # src/SeResource/PugiXml/pugixml.cpp
        # src/SeResource/XMLElement.cpp
        # src/SeResource/XMLFile.cpp
)

target_include_directories(SeResource 
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/SeResource
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/SeResource/ETCPACK/source

        ${CMAKE_CURRENT_SOURCE_DIR}/src/SeResource/rapidjson/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/SeResource
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/SeResource/rapidjson/include   
)

# target_compile_definitions(SeResource 
#         PUBLIC SE_STATIC_LIB
# )

target_link_libraries(SeResource PUBLIC
        Se
)


add_library(SeVFS STATIC
        include/SeVFS/FileIdentifier.cpp
        include/SeVFS/MountedAliasRoot.cpp 
        include/SeVFS/MountedDirectory.cpp 
        include/SeVFS/MountedExternalMemory.cpp 
        include/SeVFS/MountedRoot.cpp 
        include/SeVFS/MountPoint.cpp
        include/SeVFS/VirtualFileSystem.cpp 
        include/SeVFS/FileWatcher.cpp 
)
target_link_libraries(SeVFS PUBLIC
        Se
)

add_executable(test.SeResource 
        tests/main.cpp
        tests/test.Reflection.cpp
        # include/SeVFS/PackageFile.hpp        
)

target_link_libraries(test.SeResource PUBLIC
        SeResource
        SeVFS
)