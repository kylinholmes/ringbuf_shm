-- 设置项目名称和版本
set_project("ringbuf_shm")
set_version("1.0.0")


-- 设置全局编译选项
set_languages("cxx20") -- 使用 C++20 标准
add_rules("mode.debug", "mode.releasedbg") -- 添加调试和发布模式规则
set_defaultmode("releasedbg")
set_warnings("all", "error") -- 开启所有警告并将警告视为错误

-- 添加包含目录
add_includedirs("include")
add_requires(
    "benchmark", -- 添加 benchmark 依赖
    "unordered_dense",
    "robin-map",
    "fmt"
)

-- 定义目标
target("test_shm")
    set_kind("binary") -- 目标类型为可执行文件
    add_files("test/test_shm.cpp") -- 添加源文件
    add_links("pthread") -- 链接 pthread 库
    -- add_links("rt") -- 链接 rt 库

target("p1")
    set_kind("binary") -- 目标类型为可执行文件
    add_files("test/p1.cpp") -- 添加源文件
    add_links("pthread") -- 链接 pthread 库
    -- add_links("rt") -- 链接 rt 库
target("p2")
    set_kind("binary") -- 目标类型为可执行文件
    add_files("test/p2.cpp") -- 添加源文件
    add_links("pthread") -- 链接 pthread 库
    -- add_links("rt") -- 链接 rt 库

target("thp")
    set_kind("binary") -- 目标类型为可执行文件
    add_files("test/threadpool.cpp") -- 添加源文件
    add_links("pthread") -- 链接 pthread 库
    -- add_links("rt") -- 链接 rt 库



target("bench")
    set_kind("binary") -- 目标类型为可执行文件
    add_files("test/bench.cpp") -- 添加源文件
    add_links("pthread") -- 链接 pthread 库
    if is_plat("linux") then
        add_links("rt") -- 链接 rt 库   
    end
    add_packages(
        "benchmark", 
        "unordered_dense",
        "robin-map",
        "fmt"
    ) -- 添加 benchmark 包依赖