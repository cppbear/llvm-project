set(LLVM_LINK_COMPONENTS)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_clang_executable(brinfo
  BrInfo.cpp
  Analysis.cpp
  Condition.cpp
  CondChain.cpp
  )

clang_target_link_libraries(brinfo
  PRIVATE
  clangFrontend
  clangTooling
  nlohmann_json::nlohmann_json
  )

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(brinfo PRIVATE DEBUG)
endif()

install(TARGETS brinfo
  RUNTIME DESTINATION bin
  COMPONENT brinfo)
