cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("VortexConfig")

# Add source to this project's executable.
add_executable (VortexConfig "src/main.cpp" "include/vcfg/VortexConfig.h" "include/vcfg/macros.h" "include/vcfg/parser.h" "include/vcfg/compatibility.h" "include/vcfg/strconv.h" )
target_include_directories(VortexConfig PRIVATE "include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VortexConfig PROPERTY CXX_STANDARD 20)
endif()
