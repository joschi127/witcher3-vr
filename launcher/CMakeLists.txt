cmake_minimum_required(VERSION 3.24)
project(Witcher3VRLauncher VERSION 0.1579.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(w3vr_launcher_config STATIC
    config.cpp
    config.h
    managed_runtime.cpp
    managed_runtime.h
    ofxr_launch_environment.cpp
    ofxr_launch_environment.h
)
target_compile_definitions(w3vr_launcher_config PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_compile_options(w3vr_launcher_config PRIVATE /W4 /permissive-)
target_link_libraries(w3vr_launcher_config PUBLIC shell32 ole32 version)

add_library(w3vr_launcher_startup_checks STATIC
    startup_checks.cpp
    startup_checks.h
)
target_compile_definitions(w3vr_launcher_startup_checks PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_compile_options(w3vr_launcher_startup_checks PRIVATE /W4 /permissive-)
target_link_libraries(w3vr_launcher_startup_checks PUBLIC w3vr_launcher_config)

add_executable(Witcher3VRLauncher WIN32
    main.cpp
    openxr_resolution.cpp
    openxr_resolution.h
    launcher.rc
    resources.h
    witcher3vr.default.ini
    vr_dx12user.settings
)
target_include_directories(Witcher3VRLauncher PRIVATE
    "${W3VR_OPENXR_DIR}/include"
)
target_compile_definitions(Witcher3VRLauncher PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_compile_options(Witcher3VRLauncher PRIVATE /W4 /permissive-)
target_link_libraries(Witcher3VRLauncher PRIVATE
    w3vr_launcher_config
    w3vr_launcher_startup_checks
    comctl32
)

include(CTest)
if(BUILD_TESTING)
    add_executable(w3vr_launcher_tests tests.cpp)
    target_compile_definitions(w3vr_launcher_tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_compile_options(w3vr_launcher_tests PRIVATE /W4 /permissive-)
    target_link_libraries(w3vr_launcher_tests PRIVATE w3vr_launcher_config)
    add_test(NAME launcher_config COMMAND w3vr_launcher_tests)

    add_executable(w3vr_launcher_managed_runtime_tests
        managed_runtime_tests.cpp
    )
    target_compile_definitions(w3vr_launcher_managed_runtime_tests PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_compile_options(w3vr_launcher_managed_runtime_tests PRIVATE
        /W4 /permissive-)
    target_link_libraries(w3vr_launcher_managed_runtime_tests PRIVATE
        w3vr_launcher_config)
    add_test(NAME launcher_managed_runtime
        COMMAND w3vr_launcher_managed_runtime_tests)

    add_executable(w3vr_launcher_startup_checks_tests
        startup_checks_tests.cpp
    )
    target_compile_definitions(w3vr_launcher_startup_checks_tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_compile_options(w3vr_launcher_startup_checks_tests PRIVATE /W4 /permissive-)
    target_link_libraries(w3vr_launcher_startup_checks_tests PRIVATE
        w3vr_launcher_startup_checks
    )
    add_test(NAME launcher_startup_checks COMMAND w3vr_launcher_startup_checks_tests)

    # Hardware-dependent manual smoke test. It is intentionally not registered
    # with CTest because an active OpenXR runtime/headset is required.
    add_executable(w3vr_launcher_openxr_query_isolation_smoke
        openxr_query_isolation_smoke.cpp
        openxr_resolution.cpp
        openxr_resolution.h
    )
    target_include_directories(w3vr_launcher_openxr_query_isolation_smoke PRIVATE
        "${W3VR_OPENXR_DIR}/include"
    )
    target_compile_definitions(w3vr_launcher_openxr_query_isolation_smoke PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_compile_options(w3vr_launcher_openxr_query_isolation_smoke PRIVATE
        /W4 /permissive-)
endif()
