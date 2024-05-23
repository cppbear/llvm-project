set(LLVM_LINK_COMPONENTS support)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_clang_executable(focxt
Focxt.cpp
GetAllPath.cpp
GetSignature.cpp
GetAllContext.cpp
CoutContext.cpp
  )

target_link_libraries(focxt
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
  nlohmann_json
  clangLex
  )

  install(TARGETS brinfo
  RUNTIME DESTINATION bin
  COMPONENT brinfo)