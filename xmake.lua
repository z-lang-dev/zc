add_rules("mode.debug", "mode.release")

-- 测试用例列表
local case_list = {"hello", "simple_int", "single_int", "simple_add", "add_sub", "calc", "neg_group", "read_file", "write_file"}

target("z")
    set_kind("binary")
    add_files("src/*.c")
    add_deps("std")
    add_includedirs("lib")
    if is_mode("debug") then
        add_defines("LOG_TRACE")
    end
    -- 运行xmake clean时，顺便也把work和test目录下的临时文件清除
    on_clean(function (target)
        print("Cleaning temp files during test and run...")
        os.rm("work/*.obj")
        os.rm("work/*.exe")
        os.rm("work/*.lnk")
        os.rm("work/*.tmp")
        os.rm("work/a.out")
        for _, d in ipairs(case_list) do
            os.rm("test/"..d.."/app.*")
            os.rm("test/"..d.."/*.lnk")
            os.rm("test/"..d.."/*.tmp")
        end
    end)

target("std")
    set_kind("static")
    add_files("lib/*.c")
    add_files("src/util.c")
    add_includedirs("lib")
    add_includedirs("src")

target("test_std")
    set_kind("binary")
    add_files("test/test_std.c")
    add_deps("std")
    add_includedirs("lib")
    add_includedirs("src")
    set_rundir(os.projectdir())
    add_tests("hello", {rundir = os.projectdir(), trim_output=true, pass_outputs="hello world"})
    after_test(function (target, opt)
        os.rm("write_file_test.txt")
    end)

-- 解释器interp的测试用例
target("test_interp")
    set_kind("binary")
    set_default(false)
    add_includedirs("src")
    add_files("src/*.c")
    remove_files("src/main.c")
    add_deps("std")
    add_includedirs("lib")
    add_files("test/test_interp.c")
    add_tests("hello", {runargs="print(\"Hello, world!\")", trim_output=true, pass_outputs="Hello, world!"})
    add_tests("hello1", {runargs="print(\"Now!\")", trim_output=true, pass_outputs="Now!"})
    add_tests("simple_int", {runargs="print(41)", trim_output=true, pass_outputs="41"})
    add_tests("single_int", {runargs="42", trim_output=true, pass_outputs="42"})
    add_tests("simple_add", {runargs="37+4", trim_output=true, pass_outputs="41"})
    add_tests("add_sub", {runargs="1+5-3", trim_output=true, pass_outputs="3"})
    add_tests("calc", {runargs="2*3+4*5-1*7", trim_output=true, pass_outputs="19"})
    add_tests("neg_group", {runargs="-(3*5+-2-8)", trim_output=true, pass_outputs="-5"})


-- 编译器compiler的测试用例
target("test_compiler")
    set_kind("binary")
    set_default(false)
    add_includedirs("src")
    add_files("src/*.c")
    remove_files("src/main.c")
    add_deps("std")
    add_includedirs("lib")
    add_files("test/test_compiler.c")
    for _, d in ipairs(case_list) do
        local asm_ext = is_plat("windows") and "asm" or "s"
        add_tests(d, {rundir = os.projectdir().."/test/"..d, runargs = {d.."_case.z", d.."_expected."..asm_ext}})
    end

-- 转译器transpiler的测试用例
target("test_transpiler")
    set_kind("binary")
    set_default(false)
    add_includedirs("src")
    add_files("src/*.c")
    add_deps("std")
    add_includedirs("lib")
    remove_files("src/main.c")
    add_files("test/test_transpiler.c")

    for _, d in ipairs(case_list) do
        for _, lan in ipairs({"c", "py", "js"}) do
            add_tests(d.."_"..lan, {rundir = os.projectdir().."/test/"..d, runargs = {lan, d.."_case.z", d.."_expected."..lan}})
        end
    end


--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

