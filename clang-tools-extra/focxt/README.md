##### Focxt

该工具用于分析c++源代码每个函数的上下文信息。

##### 前提

使用该工具首先需要被分析项目具有类似compile_commands.json的编译数据库。

如果被分析项目本身使用CMake管理构建，可以通过在配置CMake时添加-DCMAKE_EXPORT_COMPILE_COMMANDS=ON来生成。

如果项目本身不使用CMake管理构建，可以考虑使用Bear来生成。

##### 使用

```shell
/usr/local/bin/focxt --help
USAGE: focxt [options]

OPTIONS:

Color Options:

  --color                                           - Use colors in output (default=autodetect)

General options:

  --disable-auto-upgrade-debug-info                 - Disable autoupgrade of debug info
  --disable-i2p-p2i-opt                             - Disables inttoptr/ptrtoint roundtrip optimization
  --dot-cfg-mssa=<file name for generated dot file> - file name for generated dot file
  --enable-name-compression                         - Enable name/filename string compression
  --generate-merged-base-profiles                   - When generating nested context-sensitive profiles, always generate extra base profile for function with all its context profiles merged into it.

Generic Options:

  --help                                            - Display available options (--help-hidden for more)
  --help-list                                       - Display list of available options (--help-list-hidden for more)
  --version                                         - Display the version of this program

focxt options:

  --build=<string>                                  - Specify the build path
  --class=<string>                                  - Specify the class to analyze
  --file=<string>                                   - Specify the file to analyze
  --function=<string>                               - Specify the function to analyze
  --may-test                                        - Specify get may test
  --must-test=<string>                              - Specify must test second parameters
  --project=<string>                                - Specify the projrct path
```

##### 用法

###### 分析整个项目

```
./focxt --project path/to/project --build path/to/build/directory {--may-test}
```

​	其中--may-test是一个开关，搜索可能的测试函数，由于TEST(TestSuiteName, TestName) { }会被展开为class TestSuiteName_TestName_Test，所以会搜索项目中TestName包含函数名的TEST宏和宏展开后的类。

###### 分析整个文件

```
./focxt --project path/to/project --build path/to/build/directory --file path/to/source_file {--may-test}
```

###### 分析某个函数或方法

```
./focxt --project path/to/project --build path/to/build/directory --file path/to/source_file --class class_name --function function_name {--may-test {--must-test TEST_second_parameter} {--must-test TEST_second_parameter} ..}
```

​	其中--must-test需要输入的是TEST宏的第二个参数，开启--may-test后，才能够通过--must-test手动指定测试函数。

最终结果以*_cxt.json存储在path/to/project目录中。

##### 构建

###### 安装依赖，包括：

C/C++编译工具，如gcc或clang
CMake
Ninja

###### 克隆并进入该项目

git clone https://github.com/cppbear/llvm-project.git && cd llvm-project

###### 配置CMake

cmake -S llvm -B build -G Ninja -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host

###### 构建并安装

cmake --build build --target focxt
cmake --install build --component focxt
cmake --install build --component clang-resource-headers
