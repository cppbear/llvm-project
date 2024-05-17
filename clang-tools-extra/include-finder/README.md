# IncludeFinder

该工具能够对于指定C++源码文件分析其中包含的位于项目内的头文件。

## 前提

使用该工具首先需要被分析项目具有类似`compile_commands.json`的编译数据库。

如果被分析项目本身使用CMake管理构建，可以通过在配置CMake时添加`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`来生成。

如果项目本身不使用CMake管理构建，可以考虑使用[Bear](https://github.com/rizsotto/Bear)来生成。

## 使用

```
USAGE: include-finder [options] <source0> [... <sourceN>]

OPTIONS:

Generic Options:

  --help                      - Display available options (--help-hidden for more)
  --help-list                 - Display list of available options (--help-list-hidden for more)
  --version                   - Display the version of this program

brinfo options:

  -p <string>                 - Build path
  --project=<string>          - Specify the projrct path

-p <build-path> is used to read a compile command database.

        For example, it can be a CMake build directory in which a file named
        compile_commands.json exists (use -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        CMake option to get this output). When no build path is specified,
        a search for compile_commands.json will be attempted through all
        parent paths of the first input file . See:
        https://clang.llvm.org/docs/HowToSetupToolingForLLVM.html for an
        example of setting up Clang Tooling on a source tree.

<source0> ... specify the paths of source files. These paths are
        looked up in the compile command database. If the path of a file is
        absolute, it needs to point into CMake's source tree. If the path is
        relative, the current working directory needs to be in the CMake
        source tree and the file must be in a subdirectory of the current
        working directory. "./" prefixes in the relative files will be
        automatically removed, but the rest of a relative path must be a
        suffix of a path in the compile command database.
```

`--project`选项指定项目所在路径，`-p`选项指定`compile_commands.json`的文件夹路径。

### 示例

1. 分析某个项目中某个/些源文件中所包含的项目内的所有头文件

   ```
   include-finder --project path/to/project -p path/to/build/directory path/to/source/file0 [path/to/source/file1 ...]
   ```

该工具分析得到的头文件路径将保存为以`includes.json`为文件名的json文件，该文件位于被分析的项目目录下。

## 构建

1. 安装依赖，包括：

   1. C/C++编译工具，如`gcc`或`clang`
   2. `CMake`
   3. `Ninja`
   
2. 克隆并进入该项目

   ```
   git clone https://github.com/cppbear/llvm-project.git && cd llvm-project
   ```

2. 配置CMake

   ```
   cmake -S llvm -B build -G Ninja -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=host
   ```

3. 构建并安装

   ```
   cmake --build build --target include-finder
   cmake --install build --component include-finder
   cmake --install build --component clang-resource-headers
   ```
