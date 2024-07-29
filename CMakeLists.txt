project(Se)



set (CMAKE_CXX_STANDARD 20)

option(HAVE_LZ4 "LZ4 Compression" ON)

set(SE_DEFINED
        PUBLIC SE_FILEWATCHER
        PUBLIC SE_THREADING
)

set(SE_INCLUDE_DIRS )

set(SE_SOURCE
        src/Se/Debug.cpp
        src/Se/IO/File.cpp
        src/Se/IO/FileSystem.cpp
        src/Se/IO/PackageFile.cpp
        src/Se/Thread.cpp
        src/Se/Timer.cpp
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

target_compile_definitions(Se ${SE_DEFINED})
target_compile_features(Se PRIVATE cxx_std_20)


add_library(SeVFS STATIC
        include/SeVFS/FileIdentifier.cpp 
        include/SeVFS/FileWatcher.cpp 
        include/SeVFS/MountedAliasRoot.cpp 
        include/SeVFS/MountedDirectory.cpp 
        include/SeVFS/MountedExternalMemory.cpp 
        include/SeVFS/MountedRoot.cpp 
        include/SeVFS/MountPoint.cpp 
        include/SeVFS/VirtualFileSystem.cpp 
)
target_link_libraries(SeVFS PUBLIC
        Se
)

add_library(SeArcJson STATIC
        src/SeArcJson/JSONValue.cpp
        include/SeArcJson/JSONArchive.cpp
        src/SeArcJson/JSONFile.cpp
)

target_include_directories(SeArcJson PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/SeArcJson
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SeArcJson/rapidjson/include
)

target_link_libraries(SeArcJson PUBLIC
        Se
)

add_executable(test.SeArc tests/main.cpp
        # include/SeVFS/PackageFile.hpp        
)

target_link_libraries(test.SeArc PUBLIC
        SeArcJson
        SeVFS
)