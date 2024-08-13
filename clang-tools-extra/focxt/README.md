# Focxt

该工具用于分析 c++源代码每个函数的上下文信息。

## 前提

使用该工具首先需要被分析项目具有类似 compile_commands.json 的编译数据库。

如果被分析项目本身使用 CMake 管理构建，可以通过在配置 CMake 时添加-DCMAKE_EXPORT_COMPILE_COMMANDS=ON 来生成。

如果项目本身不使用 CMake 管理构建，可以考虑使用 Bear 来生成。

## 使用

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
  --project=<string>                                - Specify the projrct path
```

## 用法

### 分析整个项目

```
./focxt --project path/to/project --build path/to/build/directory
```

### 分析整个文件

```
./focxt --project path/to/project --build path/to/build/directory --file path/to/source_file
```

### 分析某个函数或方法

```
./focxt --project path/to/project --build path/to/build/directory --file path/to/source_file --class class_name --function function_name
```

分析类的方法时需要传入 class_name 和 function_name；分析函数时不用传入 class_name，只用传入 function_name 就可以。

## 构建

1. 安装依赖，包括：

   1. C/C++编译工具，如`gcc`或`clang`
   2. `CMake`
   3. `Ninja`

2. 配置 CMake

   ```
   cmake -S llvm -B build -G Ninja -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host -DLLVM_ENABLE_DUMP=ON
   ```

3. 构建并安装

   ```
   cmake --build build --target focxt
   cmake --install build --component focxt
   cmake --install build --component clang-resource-headers
   ```
