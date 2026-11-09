# cmake/GetGitHash.cmake
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT GIT_HASH)
    set(GIT_HASH "local-dev")
endif()

# Check dirty status restricted strictly to source/build paths
execute_process(
    COMMAND git status --porcelain engine/ main.cpp CMakeLists.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    OUTPUT_VARIABLE GIT_DIFF
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(GIT_DIFF STREQUAL "")
    set(DIRTY_SUFFIX "")
else()
    set(DIRTY_SUFFIX "-dirty")
endif()

set(FINAL_COMMIT "${GIT_HASH}${DIRTY_SUFFIX}")

# Generate a small header file
file(WRITE "${OUTPUT_FILE}" "#define COMMIT_HASH \"${FINAL_COMMIT}\"\n")
