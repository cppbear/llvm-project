set(LLVM_LINK_COMPONENTS
  # FrontendOpenMP
  # Support
  )

add_clang_executable(brinfo
  BrInfo.cpp
  )

clang_target_link_libraries(brinfo
  PRIVATE
  # clangAST
  # clangASTMatchers
  # clangBasic
  clangFrontend
  clangTooling
  # clangToolingRefactoring
  )
