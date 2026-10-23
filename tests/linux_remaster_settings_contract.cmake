if(NOT DEFINED LAUNCHER_CONFIG OR NOT EXISTS "${LAUNCHER_CONFIG}")
    message(FATAL_ERROR "launcher/config.cpp was not provided")
endif()
if(NOT DEFINED LINUX_TOOL OR NOT EXISTS "${LINUX_TOOL}")
    message(FATAL_ERROR "linux/w3vr-linux.py was not provided")
endif()

file(READ "${LAUNCHER_CONFIG}" launcher)
file(READ "${LINUX_TOOL}" linux_tool)

# The 5.x settings table of "Configure Settings for VR".
string(FIND "${launcher}" "static constexpr Setting kSettings[] = {" table_start)
if(table_start EQUAL -1)
    message(FATAL_ERROR "kSettings table not found in launcher/config.cpp")
endif()
string(SUBSTRING "${launcher}" ${table_start} -1 table)
string(FIND "${table}" "};" table_end)
string(SUBSTRING "${table}" 0 ${table_end} table)
string(REGEX MATCHALL "\\{\"[^\"]+\", \"[^\"]+\", \"[^\"]+\"\\}" entries "${table}")
list(LENGTH entries entry_count)
if(entry_count LESS 20)
    message(FATAL_ERROR "kSettings looks incomplete (${entry_count} entries)")
endif()

# configure in the Linux tool writes the same keys on 5.01 (REMASTER_VR_SETTINGS); AAMode and AllowDLSS
# follow from its mode table there.
foreach(entry IN LISTS entries)
    string(REGEX REPLACE "^\\{\"([^\"]+)\", \"([^\"]+)\", \"([^\"]+)\"\\}$" "\\1;\\2;\\3" parts "${entry}")
    list(GET parts 0 section)
    list(GET parts 1 key)
    list(GET parts 2 value)
    if((section STREQUAL "PostProcess" AND key STREQUAL "AAMode") OR
       (section STREQUAL "Rendering" AND key STREQUAL "AllowDLSS"))
        continue()
    endif()
    string(FIND "${linux_tool}" "('${section}', '${key}', '${value}')," position)
    if(position EQUAL -1)
        message(FATAL_ERROR "linux/w3vr-linux.py REMASTER_VR_SETTINGS lacks [${section}] ${key}=${value}")
    endif()
endforeach()

string(FIND "${linux_tool}" "for section, key, value in REMASTER_VR_SETTINGS:" applied)
if(applied EQUAL -1)
    message(FATAL_ERROR "configure does not apply REMASTER_VR_SETTINGS")
endif()

message(STATUS "Linux configure writes the launcher's ${entry_count} 5.x settings")
