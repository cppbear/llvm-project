//===---- tools/extra/BrInfo.cpp ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Usage:
//  brinfo <cmake-output-dir> <file1> <file2> ...
//
//  Where <cmake-output-dir> is a CMake build directory in which a file named
//  compile_commands.json exists (enable -DCMAKE_EXPORT_COMPILE_COMMANDS in
//  CMake to get this output).
//
//  <file1> ... specify the paths of files in the CMake source tree. This path
//  is looked up in the compile command database. If the path of a file is
//  absolute, it needs to point into CMake's source tree. If the path is
//  relative, the current working directory needs to be in the CMake source
//  tree and the file must be in a subdirectory of the current working
//  directory. "./" prefixes in the relative files will be automatically
//  removed, but the rest of a relative path must be a suffix of a path in
//  the compile command line database.
//
//  For example, to use brinfo on all files in a subtree of the
//  source tree, use:
//
//    /path/in/subtree $ find . -name '*.cpp'|
//        xargs brinfo /path/to/build
//
//===----------------------------------------------------------------------===//

#include "Matcher.h"

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory BrInfoCategory("brinfo options");
cl::opt<std::string> FunctionName("f",
                                  cl::desc("Specify the function to analyze"),
                                  cl::value_desc("string"),
                                  cl::cat(BrInfoCategory));
cl::opt<std::string> ClassName("c",
                               cl::desc("Specify the class of the function"),
                               cl::value_desc("string"),
                               cl::cat(BrInfoCategory));
cl::opt<std::string> ProjectPath("project", cl::Required,
                                 cl::desc("Specify the projrct path"),
                                 cl::value_desc("string"),
                                 cl::cat(BrInfoCategory));
cl::opt<bool> DumpCFG("cfg", cl::desc("Dump CFG to .dot file"),
                      cl::cat(BrInfoCategory));

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, BrInfoCategory);
  if (!ExpectedParser) {
    errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionParser = ExpectedParser.get();
  if (OptionParser.getSourcePathList().size() != 1) {
    errs() << "Just specify one source file\n";
    return 1;
  }
  ClangTool Tool(OptionParser.getCompilations(),
                 OptionParser.getSourcePathList());

  return BrInfo::run(Tool);
}
