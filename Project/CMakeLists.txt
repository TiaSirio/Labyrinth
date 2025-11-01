### Adapted from blurrypiano Vulkan tutorial https://github.com/blurrypiano/littleVulkanEngine

# CMake version and required policies
cmake_minimum_required(VERSION 3.26)

# Project setup
set(NAME Labyrinth)
project(${NAME} VERSION 0.1.0)

################################ OPTIONS ################################

option(USE_7Z "Use 7z for packaging" OFF)
option(USE_IFW "Use IFW for packaging" ON)
option(USE_ADDITIONAL_COMPILE_OPTIONS "Use additional compile options" OFF)
option(USE_COMPILE_OPTIMIZATIONS "Use general compile optimizations" OFF)
option(ENABLE_PRECOMPILED_HEADERS "Enable precompiled headers" OFF)
option(ENABLE_UNITY_BUILD "Enable unity build" OFF)
option(ENABLE_CACHE_OF_VULKAN_PIPELINE "Enable cache of Vulkan pipeline" OFF)
option(ENABLE_USE_OF_PROFILING "Enable use of profiling" OFF)
option(ENABLE_BUILD_DOC "Enable build documentation" ON)
option(USE_VOLK "Use volk" OFF)

# Print all options at the start of the configuration
message(STATUS "==================== Selected Options ====================")
message(STATUS "USE_7Z: ${USE_7Z}")
message(STATUS "USE_IFW: ${USE_IFW}")
message(STATUS "USE_ADDITIONAL_COMPILE_OPTIONS: ${USE_ADDITIONAL_COMPILE_OPTIONS}")
message(STATUS "USE_COMPILE_OPTIMIZATIONS: ${USE_COMPILE_OPTIMIZATIONS}")
message(STATUS "ENABLE_PRECOMPILED_HEADERS: ${ENABLE_PRECOMPILED_HEADERS}")
message(STATUS "ENABLE_UNITY_BUILD: ${ENABLE_UNITY_BUILD}")
message(STATUS "ENABLE_CACHE_OF_VULKAN_PIPELINE: ${ENABLE_CACHE_OF_VULKAN_PIPELINE}")
message(STATUS "ENABLE_USE_OF_PROFILING: ${ENABLE_USE_OF_PROFILING}")
message(STATUS "ENABLE_BUILD_DOC: ${ENABLE_BUILD_DOC}")
message(STATUS "USE_VOLK: ${USE_VOLK}")
message(STATUS "==========================================================")

################################ PROJECT SETUP ###############################

# Display project information
message(STATUS "Project: ${NAME} - Version: ${PROJECT_VERSION}")
message(STATUS "Using generator: ${CMAKE_GENERATOR}")

# Load environment variables from .env.cmake
include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

################################ COMPILER SETUP ###############################

# Compiler setup for MinGW (Windows-specific setup)
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW TRUE)
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER ${MINGW_PATH}/bin/g++.exe)
endif()

if (MSVC)
    # Compiler flags configuration based on the build type
    # Force /MD (runtime multithreaded DLL) for Release ad Debug build
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif(MSVC)

if(USE_ADDITIONAL_COMPILE_OPTIONS)
    # Setup flags for different build types
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Definitions and sanitizers for debug
        add_compile_definitions(
            VK_ENABLE_BETA_EXTENSIONS
            VK_LAYER_KHRONOS_validation
            VK_EXT_debug_utils
            VK_EXT_debug_marker
            ENABLE_VULKAN_LOGGING
        )
        add_compile_options(-fsanitize=address)
        if (MSVC)
            # Warning levels
            add_compile_options(/W4 /w44365 /external:W3)
        endif(MSVC)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        # CPU optimization
        add_compile_options(-mfpmath=sse -msse4.2)
        # Linker optimization
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO (link-time optimization)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-O2)
endif()

################################ COMPILER FLAGS ##############################

if(USE_COMPILE_OPTIMIZATIONS)
    # Apply general compiler optimizations depending on the toolchain
    if (MSVC)
        # /GL enables whole program optimization
        # /fp:fast enables fast floating-point model (less strict IEEE compliance)
        # /arch:AVX2 enables AVX2 instruction set optimizations
        add_compile_options(/GL /fp:fast /arch:AVX2)
        # /LTCG enables link-time code generation (pairs with /GL)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
        # Uncomment to get vectorization reports
        # add_compile_options(/Qvec-report:2)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # -ffast-math enables aggressive FP optimizations (not IEEE compliant)
        # -ftree-loop-vectorize enables loop vectorization
        add_compile_options(-ffast-math -ftree-loop-vectorize)
        # --gc-sections removes unused code and data
        # -fuse-ld=gold selects the "gold" linker (faster than ld.bfd)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -fuse-ld=gold")
    endif()
endif(USE_COMPILE_OPTIMIZATIONS)

################################ DEPENDENCIES ###############################

# Volk setup (Vulkan loader library)
if(USE_VOLK)
    # Store Volk in project-local libraries folder
    set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/libraries/Volk)
    include(FetchContent)
    FetchContent_Declare(
        volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(volk)
else()
    # Check if Vulkan SDK is installed
    find_package(Vulkan REQUIRED)
endif()

# GLFW setup
# Check if GLFW path is defined in environment variables, otherwise use find_package
if (DEFINED GLFW_PATH)
    message(STATUS "Using GLFW path specified in .env")
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
    if (MSVC)
        set(GLFW_LIB "${GLFW_PATH}/lib-vc2022")
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
    endif()
else()
    # Fallback: search system-installed GLFW
    find_package(glfw3 3.3 REQUIRED)
    set(GLFW_LIB glfw)
    message(STATUS "Found GLFW")
endif()

# Error if GLFW isn't found
if (NOT GLFW_LIB)
    message(FATAL_ERROR "Could not find GLFW library!")
else()
    message(STATUS "Using GLFW lib at: ${GLFW_LIB}")
endif()

# Gather all sources and headers
# CONFIGURE_DEPENDS allows automatic rebuilds if new files are added
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.hpp)

################################ BUILD CONFIGURATION ###############################

# Resource file for Windows builds (e.g., application icon, version info)
set(RESOURCE_FILE_PATH "${PROJECT_SOURCE_DIR}/project_resources/application_files/resources/${PROJECT_NAME}.rc")

# Create executable from source files
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${RESOURCE_FILE_PATH})

if(USE_VOLK)
    # Define a preprocessor macro for C++ code
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_VOLK)
endif()

if(ENABLE_CACHE_OF_VULKAN_PIPELINE)
    # Enable Vulkan pipeline cache
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_PIPELINE_CACHE)
endif()

if(ENABLE_USE_OF_PROFILING)
    # Enable profiling
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_PROFILING)
endif()

# Precompiled headers for Vulkan and GLFW
if(ENABLE_PRECOMPILED_HEADERS)
    target_precompile_headers(${PROJECT_NAME} PRIVATE <vulkan/vulkan.h> <GLFW/glfw3.h>)
endif()

# Set C++ standard for the project
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Visual Studio-specific settings (working directory and startup project)
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}/build" PROPERTY VS_STARTUP_PROJECT ${NAME})
set_target_properties(${NAME} PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/build"
)

# Output directories configuration based on build type
set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/build/Release/executable"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/build/Debug/executable"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/build/RelWithDebInfo/executable"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_SOURCE_DIR}/build/MinSizeRel/executable"
)

# Folder organization in Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable Unity build for faster compilation (can be disabled if necessary)
if(ENABLE_UNITY_BUILD)
    set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
endif()

# Include and link directories for different platforms
if (WIN32)
    message(STATUS "Creating build for Windows")

    if (USE_MINGW)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include)
        target_link_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/lib)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${TINYOBJLOADER_PATH}
        ${STB_PATH}
        ${GLFW_INCLUDE_DIRS}
        ${GLM_PATH}
        ${PROJECT_SOURCE_DIR}/headers
    )

    target_link_directories(${PROJECT_NAME} PUBLIC
        ${GLFW_LIB}
    )

    # Link libraries
    if(USE_VOLK)
        target_link_libraries(${PROJECT_NAME} glfw3 volk_headers)
    else()
        target_link_libraries(${PROJECT_NAME} glfw3 Vulkan::Vulkan)
    endif()

elseif (UNIX)
    message(STATUS "Creating build for Unix")

    find_package(glm REQUIRED)

    # Include directories
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${TINYOBJLOADER_PATH}
        ${STB_PATH}
    )

    # Link libraries
    if(USE_VOLK)
        target_link_libraries(${PROJECT_NAME} glfw3 volk_headers)
    else()
        target_link_libraries(${PROJECT_NAME} glfw3 Vulkan::Vulkan)
    endif()

endif()

################################ SHADERS ################################

# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
# Shader compilation setup
find_program(GLSL_VALIDATOR glslangValidator HINTS 
    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
    /usr/bin 
    /usr/local/bin 
    ${VULKAN_SDK_PATH}/Bin
    ${VULKAN_SDK_PATH}/Bin32
    $ENV{VULKAN_SDK}/Bin/ 
    $ENV{VULKAN_SDK}/Bin32/
)

# Compile shaders in the project
file(GLOB_RECURSE UNCOMPILED_SHADERS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/resources/shaders/uncompiled/*.frag"
    "${PROJECT_SOURCE_DIR}/resources/shaders/uncompiled/*.vert"
)

foreach(GLSL ${UNCOMPILED_SHADERS})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/resources/shaders/compiled/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

# Custom target to build shaders
add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${PROJECT_NAME} Shaders)

################################ RESOURCES ################################

source_group("Resources\\Shaders" FILES ${UNCOMPILED_SHADERS})
target_sources(${PROJECT_NAME} PUBLIC ${UNCOMPILED_SHADERS})

# Function to organize resources (textures, models, etc.)
function(organize_resources)
    set(FOLDERS 
        textures models audio ai animations configs fonts inputs lighting 
        localizations materials networking physics post_processing scenes ui
    )
    foreach(FOLDER ${FOLDERS})
        # Manually capitalize first letter of the group name
        string(SUBSTRING ${FOLDER} 0 1 FIRST_LETTER)
        string(SUBSTRING ${FOLDER} 1 -1 REST)
        string(TOUPPER ${FIRST_LETTER} FIRST_LETTER_UPPER)
        set(GROUP_NAME "${FIRST_LETTER_UPPER}${REST}")

        file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/resources/${FOLDER}/*")
        source_group("Resources\\${GROUP_NAME}" FILES ${RESOURCES})
        #target_sources(${PROJECT_NAME} PUBLIC ${RESOURCES})
    endforeach()
endfunction()

# Organize project resources
organize_resources()

add_custom_target(resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/resources
)
add_dependencies(${PROJECT_NAME} resources)

################################ INSTALL ################################

# Installation configuration
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/Install" CACHE PATH "Install path" FORCE)
#cmake_policy(SET CMP0177 NEW)

install(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT main)

install(FILES ${HEADERS} DESTINATION include COMPONENT main)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION resources COMPONENT main)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/project_resources/" DESTINATION project_resources COMPONENT main)
install(FILES "${PROJECT_SOURCE_DIR}/project_resources/MANUAL_INSTALL_README.txt" DESTINATION project_resources COMPONENT main)

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION . COMPONENT main)

if(USE_VOLK)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/dxc" DESTINATION include/dxc COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/glm" DESTINATION include/glm COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/glslang" DESTINATION include/glslang COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/shaderc" DESTINATION include/shaderc COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/slang" DESTINATION include/slang COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/spirv_cross" DESTINATION include/spirv_cross COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/spirv-headers" DESTINATION include/spirv-headers COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/spirv-tools" DESTINATION include/spirv-tools COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/vk_video" DESTINATION include/vk_video COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/vma" DESTINATION include/vma COMPONENT lib_vulkan)
    install(DIRECTORY "${Vulkan_INCLUDE_DIR}/vulkan" DESTINATION include/vulkan COMPONENT lib_vulkan)
    install(FILES "${Vulkan_INCLUDE_DIR}/Volk/volk.h" DESTINATION include COMPONENT lib_vulkan)
endif()

################################ PACKAGING ################################

if(USE_IFW)
    set(CPACK_GENERATOR "IFW")
else()
    if(USE_7Z)
        set(CPACK_GENERATOR "7Z")
    else()
        if(UNIX)
            set(CPACK_GENERATOR "TGZ")
        else()
            set(CPACK_GENERATOR "ZIP")
        endif()
    endif()
endif()

include(InstallRequiredSystemLibraries)

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION "An Vulkan-based game of a labyrinth")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An Vulkan-based game of a labyrinth")
set(CPACK_PACKAGE_VENDOR "${PROJECT_NAME}")
set(CPACK_PACKAGE_CONTACT "${PROJECT_NAME} <develop.katrop@gmail.com>")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")

set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${PROJECT_NAME}")
set(CPACK_IGNORE_FILES "\\\\.psd$;/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;/CMakeLists.txt.user;\\\\.\\\\#;/\\\\#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\\\.qm$;/build/;\\\\.diff$;.DS_Store")

if(USE_IFW)
    set(CPACK_IFW_PACKAGE_TITLE "Labyrinth Installer")
    set(CPACK_IFW_PACKAGE_PUBLISHER "Labyrinth")
    set(CPACK_IFW_PRODUCT_URL "https://github.com/TiaSirio/Labyrinth")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The installer of the Labyrinth game")
    set(CPACK_PACKAGE_CONTACT "develop.katrop@gmail.com")

    set(CPACK_IFW_ROOT "${IFW_PATH}/Tools/QtInstallerFramework/4.7")

    set(CPACK_IFW_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/project_resources/install_files/icons/${PROJECT_NAME}_install.ico")
    set(CPACK_IFW_PACKAGE_LOGO "${PROJECT_SOURCE_DIR}/project_resources/install_files/icons/${PROJECT_NAME}_logo.png")
    set(CPACK_IFW_PACKAGE_WINDOW_ICON "${PROJECT_SOURCE_DIR}/project_resources/install_files/icons/${PROJECT_NAME}_install.png")

    set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")

    set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS ON)
    set(CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH ON)

    set(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDir@/${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_IFW_ADMIN_TARGET_DIRECTORY "@ApplicationsDir@/${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY "${PROJECT_NAME}")

    set(CPACK_IFW_VERBOSE ON)

    set(CPACK_IFW_RESOLVE_DUPLICATE_NAMES ON)
endif()

set(CPACK_VERBATIM_VARIABLES TRUE)

if(USE_VOLK)
    set(CPACK_COMPONENTS_ALL main lib_vulkan)
else()
    set(CPACK_COMPONENTS_ALL main)
endif()

include(CPack)
if(USE_IFW)
    include(CPackIFW)
endif()

set(LANGUAGE_INSTALLER "IT" CACHE STRING "Choose the language for the installer (EN/IT)")

# Include the appropriate translation file based on the language
if(LANGUAGE_INSTALLER STREQUAL "EN")
    include("${PROJECT_SOURCE_DIR}/project_resources/install_files/translations/translations_en.cmake")
elseif(LANGUAGE_INSTALLER STREQUAL "IT")
    include("${PROJECT_SOURCE_DIR}/project_resources/install_files/translations/translations_it.cmake")
else()
    message(FATAL_ERROR "Unsupported language: ${LANG}")
endif()

cpack_add_component(main
    DISPLAY_NAME ${MAIN_APPLICATION_LOC}
    DESCRIPTION ${MAIN_APPLICATION_DESC_LOC}
    REQUIRED
    #GROUP Application
)

if(USE_VOLK)
    cpack_add_component(lib_vulkan
        DISPLAY_NAME ${VULKAN_APPLICATION_LOC}
        DESCRIPTION ${VULKAN_APPLICATION_DESC_LOC}
        REQUIRED
        #GROUP Libraries
    )
endif()

if(USE_IFW)
    cpack_ifw_configure_component(main
        NAME "vruuum.main"
        VERSION "${PROJECT_VERSION}"
        LICENSES "Licenza" "${PROJECT_SOURCE_DIR}/LICENSE"
    )

    if(USE_VOLK)
        cpack_ifw_configure_component(lib_vulkan
            NAME "vruuum.lib_vulkan"
            VERSION "${PROJECT_VERSION}"
            LICENSES "Licenza" "${PROJECT_SOURCE_DIR}/LICENSE"
        )
    endif()
endif()

################################ DOCUMENTATION ################################

if(ENABLE_BUILD_DOC)
    # Check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # Set input and output files
        set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/project_resources/docs_files/Doxyfile.in")
        set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

        # Request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        add_custom_target(DOCUMENTATION #ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating VRuuum API documentation with Doxygen"
            VERBATIM
        )
        set_target_properties(DOCUMENTATION PROPERTIES FOLDER "Documentation")
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation!")
    endif (DOXYGEN_FOUND)
endif()

################################ DIRECTORY JUNCTION ###############################

# Check if the platform is Windows
if(WIN32)
    # Define the source and destination directories
    set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
    set(BUILD_SRC_DIR "${CMAKE_BINARY_DIR}/src")

    # Add a custom command to create a directory junction
    add_custom_command(
        OUTPUT ${BUILD_SRC_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Creating directory junction from ${BUILD_SRC_DIR} to ${SRC_DIR}"
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${BUILD_SRC_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BUILD_SRC_DIR}
        COMMAND powershell -Command "New-Item -ItemType Junction -Path '${BUILD_SRC_DIR}' -Target '${SRC_DIR}'"
        COMMENT "Creating directory junction for src"
    )

    # Mark the output as symbolic to prevent MSB8065 warning
    set_source_files_properties(${BUILD_SRC_DIR} PROPERTIES SYMBOLIC TRUE)

    # Ensure the custom command runs before the build
    add_custom_target(SHARED_DIRECTORY ALL DEPENDS ${BUILD_SRC_DIR})
    set_target_properties(SHARED_DIRECTORY PROPERTIES FOLDER "Shared Source Directory")
endif()