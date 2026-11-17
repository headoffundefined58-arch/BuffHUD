cmake_minimum_required(VERSION 3.21)

# Runtime library must be decided before any target exists.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:DEBUG>:Debug>DLL")

project(
    BuffHUD
    VERSION 1.2.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# /utf-8: spdlog's bundled fmt refuses to compile without it.
# /EHsc:  CommonLibSSE does not set it and MSVC warns C4530 otherwise.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /EHsc /permissive-")

# Release keeps full optimisation but also emits a .pdb (see skill notes 13).
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /FS")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL")

# Header-only spdlog / rapidcsv assembled under D:/dev/deps (no vcpkg here).
list(APPEND CMAKE_PREFIX_PATH "D:/dev/deps")

set(SKSE_SUPPORT_XBYAK OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

# BOTH runtimes must be ON. This is not a size optimisation: with only AE
# enabled, REL::Module::GetRuntime() is hard-coded to return AE (see
# Module.h), so on a 1.5.97 install every REL::VariantID takes the AE branch,
# looks up an AE id in the SE address library, gets 0 back, and the plugin
# silently resolves every game address to null. That is what a 1.5.97 user
# sees as "address library does not match".
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)

# Six upstream headers (TES.h, ControlMap.h, InterfaceStrings.h, UserEvents.h,
# CombatController.h, BGSSaveLoadManager.h) branch on SKYRIM_SUPPORT_AE instead
# of ENABLE_SKYRIM_AE, so they need it defined explicitly or they assume the SE
# layout even in an AE build. They are compile-time-only for us — this plugin
# never dereferences those types — so pinning them to the AE layout is safe on
# both runtimes. Must precede add_subdirectory so the static library agrees.
add_compile_definitions(SKYRIM_SUPPORT_AE=1)

add_subdirectory("D:/dev/CommonLibSSE-NG" "${CMAKE_CURRENT_BINARY_DIR}/CommonLibSSE-NG")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# BuffHUD renders through the SkyUI SKSE Menu Framework, which statically links
# its own ImGui and re-exposes the entire ImGui API via the SMF header. We do
# NOT compile or link any ImGui of our own: every ImGui:: call is a thin wrapper
# that routes into SKSEMenuFramework.dll at runtime, so the plugin and the
# framework share one ImGui context without a second copy. Hud.cpp / Menu.cpp /
# Framework.cpp include that header (which pulls in <windows.h>) and must be kept
# out of the precompiled header so CommonLibSSE's REX::W32 declarations never
# clash with the Windows SDK.
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/external"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/smf")

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/PCH.h")

# These see the real Windows SDK headers; the PCH would drag CommonLib's
# competing REX::W32 declarations in ahead of them.
set(PLATFORM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Framework.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hud.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Menu.cpp")
set_source_files_properties(${PLATFORM_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    PREFIX ""
)
