set(PLUGIN_NAME csjson)

# Dependencies
    # None

# Source files
set(CPPFILES src/opcodes.cpp)
set(INCLUDES ${CSOUND_INCLUDE_DIRS} "include")
make_plugin(${PLUGIN_NAME} "${CPPFILES}")
target_include_directories(${PLUGIN_NAME} PRIVATE ${INCLUDES})
