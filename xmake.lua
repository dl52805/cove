add_rules(
  "plugin.compile_commands.autoupdate",
  {
    outputdir = ".",
    lsp = "clangd"
  }
)

add_rules("mode.debug", "mode.release")

target("meta")
  set_kind("binary")
  add_files("meta/*.cpp")
  add_includedirs("common")
  add_includedirs("lib")

  add_includedirs("lib/tree_sitter/include")
  add_files(
    "lib/tree_sitter/src/lib.c",
    "lib/tree_sitter/c_parser/parser.c",
    "lib/tree_sitter/c_parser/scanner.c"
  )

  -- set_toolset("cxx", "/Users/danluo/compilers/c_cpp/clang/instance01/build/bin/clang")
  add_cxxflags("-std=c++2c")
  set_rundir("$(projectdir)")

target("codegen")
  set_kind("phony")
  add_deps("meta")
  on_build(function(target)
    if xmake.argv()[1] == "build" then
      local meta = target:dep("meta"):targetfile()
      os.execv(meta)
    end
  end)

target("cove")
  set_kind("binary")
  add_files("src/*.cpp")
  add_includedirs("common")
  add_includedirs("src")
  add_includedirs("generated")
  -- set_toolset("cxx", "/Users/danluo/compilers/c_cpp/clang/instance01/build/bin/clang")
  add_cxxflags("-std=c++2c")
  add_cxxflags("-Wno-unknown-attributes")
  set_rundir("$(projectdir)")
  add_deps("codegen")

  -- add_cxxflags("-fsanitize=address,undefined")
  -- add_ldflags("-fsanitize=address,undefined")
