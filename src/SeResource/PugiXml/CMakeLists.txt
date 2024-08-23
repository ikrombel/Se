# Define target name
set (TARGET_NAME PugiXml)

# Define source files
set(SOURCE_FILES
        pugiconfig.hpp
        pugixml.cpp
        pugixml.hpp
        )

# Setup target
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})

#target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)