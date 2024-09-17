set(BUILD_DIR "Bin.${CMAKE_SYSTEM_NAME}${CMAKE_CXX_COMPILER_ID}.${CMAKE_BUILD_TYPE}")

if (NOT ANDROID)
    #    set(BUILD_DIR "Bin.Android${CMAKE_CXX_COMPILER_ID}.${CMAKE_BUILD_TYPE}/${ANDROID_ABI}")
    #    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${BUILD_DIR}) # *.dll
    if (UNIX)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${BUILD_DIR}/lib/) # *.so
    elseif (WIN32)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${BUILD_DIR}) # *.dll
    endif()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${BUILD_DIR}/lib/) # *.a, *.lib
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${BUILD_DIR})

    if (MINGW AND NOT EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libwinpthread-1.dll")
        find_file(DLL_FILE_PATH_1 "libstdc++-6.dll")
        find_file(DLL_FILE_PATH_2 "libgcc_s_seh-1.dll")
        find_file(DLL_FILE_PATH_3 "libwinpthread-1.dll")

        foreach (DLL_FILE_PATH ${DLL_FILE_PATH_1} ${DLL_FILE_PATH_2} ${DLL_FILE_PATH_3})
            if (DLL_FILE_PATH)
                # Copies dlls to bin or tools.
                file (COPY ${DLL_FILE_PATH} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
                if (NOT SE_STATIC_RUNTIME)
                    file (COPY ${DLL_FILE_PATH} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
                endif ()
            endif ()
        endforeach ()
    endif ()
endif()