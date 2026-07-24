package("fpag")
set_urls("https://github.com/pug523/fpag.git")

add_versions(
  "419c9bee39f133e7d0bf0adb3928a172dd561f93",
  "419c9bee39f133e7d0bf0adb3928a172dd561f93"
)

add_configs("libunwind", {
  description = "Use libunwind for stack tracing",
  default = false,
  type = "boolean",
})
add_configs(
  "stdlib",
  { description = "stl to use", default = "libstdc++", type = "string" }
)

local function is_clang()
  return is_config("toolchain", "clang", "llvm")
    or (
      not is_config("toolchain", "gcc")
      and (is_plat("macosx", "iphoneos") or is_host("macosx"))
    )
end

local function config()
  if is_clang() and not is_plat("windows") and has_config("stdlib") then
    local std = get_config("stdlib")
    return {
      cxxflags = "-stdlib=" .. std,
      ldflags = "-stdlib=" .. std,
      -- for releasedbg
      debug = not is_mode("release"),
    }
  end
  return {}
end

add_deps("xxhash v0.8.3", {
  external = true,
  system = false,
  configs = config(),
  -- configs = { cxxflags = "-stdlib=libc++", ldflags = "-stdlib=libc++" },
})
add_deps("fmt 12.1.0", {
  external = false,
  system = false,
  configs = config(),
  -- configs = { cxxflags = "-stdlib=libc++", ldflags = "-stdlib=libc++" },
})

on_load(function(package)
  package:add("deps", "fmt")
  if package:config("libunwind") and package:is_plat("linux") then
    package:add("deps", "libunwind v1.8.3", {
      external = true,
      system = false,
      configs = config(),
      -- configs = { cxxflags = "-stdlib=libc++", ldflags = "-stdlib=libc++" },
    })
  end

  if package:is_plat("windows") then
    package:add("syslinks", { "dbghelp", "onecore" })
  end
end)

on_install("linux", "macosx", "windows", function(package)
  local configs = {}
  configs.tests = false
  configs.benchmarks = false
  configs.libunwind = package:config("libunwind")
  configs.stdlib = package:config("stdlib")
  configs.kind = package:config("shared") and "shared" or "static"

  import("package.tools.xmake").install(package, configs)
end)

on_test(function(package)
  assert(
    package:check_cxxsnippets(
      {
        test = [[
                #include <fpag/base/math_util.h>
                #include <assert.h>
                void test() {
                  assert(base::next_power_of_two(31) == 32);
                  assert(base::next_power_of_two(4096) == 4096);
                }
            ]],
      },
      { configs = { languages = "c++23" }, includes = "fpag/base/math_util.h" }
    )
  )
end)
