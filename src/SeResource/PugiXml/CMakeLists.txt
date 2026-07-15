# Define target name
set (TARGET_NAME PugiXml)

# Define source files
set(PUGIXML_SOURCES
        pugiconfig.hpp
        pugixml.cpp
        pugixml.hpp
        )

# Setup target
add_library(${TARGET_NAME} STATIC ${PUGIXML_SOURCES})

target_compile_definitions(${TARGET_NAME} 
        PUBLIC PUGIXML_COMPACT)

#target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)