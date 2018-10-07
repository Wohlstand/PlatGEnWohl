# Major
set(MUSPLAY_VERSION_1 1)
# Minor
set(MUSPLAY_VERSION_2 4)
# Revision
set(MUSPLAY_VERSION_3 0)
# Patch
set(MUSPLAY_VERSION_4 0)
# Type of version: "-alpha","-beta","-dev", or "" aka "release"
set(MUSPLAY_VERSION_REL "")

add_definitions(-DMUSPLAY_VERSION_1=${MUSPLAY_VERSION_1})
add_definitions(-DMUSPLAY_VERSION_2=${MUSPLAY_VERSION_2})
add_definitions(-DMUSPLAY_VERSION_3=${MUSPLAY_VERSION_3})
add_definitions(-DMUSPLAY_VERSION_4=${MUSPLAY_VERSION_4})
add_definitions(-DMUSPLAY_VERSION_REL=${MUSPLAY_VERSION_REL})

set(MUSPLAY_VERSION_STRING "${MUSPLAY_VERSION_1}.${MUSPLAY_VERSION_2}")

if(NOT ${MUSPLAY_VERSION_3} EQUAL 0 OR NOT ${MUSPLAY_VERSION_4} EQUAL 0)
    string(CONCAT MUSPLAY_VERSION_STRING "${MUSPLAY_VERSION_STRING}" ".${MUSPLAY_VERSION_3}")
endif()

if(NOT ${MUSPLAY_VERSION_4} EQUAL 0)
    string(CONCAT MUSPLAY_VERSION_STRING "${MUSPLAY_VERSION_STRING}" ".${MUSPLAY_VERSION_4}")
endif()

if(NOT "${MUSPLAY_VERSION_REL}" STREQUAL "")
    string(CONCAT MUSPLAY_VERSION_STRING "${MUSPLAY_VERSION_STRING}" "${MUSPLAY_VERSION_REL}")
endif()

message("== Music Player version ${MUSPLAY_VERSION_STRING} ==")

