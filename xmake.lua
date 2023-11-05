add_rules("mode.debug", "mode.release")

target("z")
    set_kind("binary")
    add_files("src/*.c")

target("test_interp")
    set_kind("binary")
    set_default(false)
    add_includedirs("src")
    add_files("src/*.c")
    remove_files("src/main.c")
    add_files("test/test_interp.c")
    add_tests("hello", {runargs="print(\"Hello, world!\")", trim_output=true, pass_outputs="Hello, world!"})
    add_tests("hello1", {runargs="print(\"Now!\")", trim_output=true, pass_outputs="Now!"})

target("test_compiler")
    set_kind("binary")
    set_default(false)
    add_includedirs("src")
    add_files("src/*.c")
    remove_files("src/main.c")
    add_files("test/test_compiler.c")
    if is_plat("windows") then
        add_tests("hello", {rundir = os.projectdir().."/test/hello", runargs = {"hello.z", "hello_expect.asm"}})
    else
        add_tests("hello", {rundir = os.projectdir().."/test/hello", runargs = {"hello.z", "hello_expect.s"}})
    end

target("test_transpiler")
    set_kind("binary")
    set_default(false)
    add_includedirs("src")
    add_files("src/*.c")
    remove_files("src/main.c")
    add_files("test/test_transpiler.c")
    add_tests("hello_c", {rundir = os.projectdir().."/test/hello", runargs = {"c", "hello.z", "hello_expect.c"}})
    add_tests("hello_py", {rundir = os.projectdir().."/test/hello", runargs = {"py", "hello.z", "hello_expect.py"}})
    add_tests("hello_js", {rundir = os.projectdir().."/test/hello", runargs = {"js", "hello.z", "hello_expect.js"}})


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

