set(LLVM_LINK_COMPONENTS
  # FrontendOpenMP
  # Support
  )

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_clang_executable(include-finder
  IncludeFinder.cpp
  )

clang_target_link_libraries(include-finder
  PRIVATE
  # clangAST
  # clangASTMatchers
  # clangBasic
  clangFrontend
  clangTooling
  # clangToolingRefactoring
  nlohmann_json::nlohmann_json
  )

install(TARGETS include-finder
  RUNTIME DESTINATION bin
  COMPONENT include-finder)
