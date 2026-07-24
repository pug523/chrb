-- Copyright 2026 pugur
-- This source code is licensed under the Apache License, Version 2.0
-- which can be found in the LICENSE file.

local project_name = "chrb"
local project_version = "0.4.2"

set_project(project_name)
set_version(project_version)

option("coverage", { default = false, description = "use llvm-cov for analyzing test coverage" })
option("xray", { default = false, description = "use llvm-xray for determining performance bottleneck" })
option("optreport", { default = false, description = "report optimization result" })
option("sanitizers", { default = false, description = "enable address/undefined behaviour/leak sanitizer" })
option("timetrace", { default = false, description = "generate timetrace json that can be see with perfetto ui" })
option("native", { default = false, description = "native architecture optimization" })
option("unitybuild", { default = false, description = "enable unity build to shorten build time" })
option("lto", { default = false, description = "use link time optimization on release builds" })
option("tests", { default = false, description = "build unit tests" })
option("lib", { default = false, description = "build as static library"})
option("stdlib", { default = "libstdc++", description = "stl to use" })

includes("src/build/xmake/fpag.lua")

set_policy("build.ccache", true)
set_policy("check.auto_ignore_flags", false)
set_policy("build.progress_style", "multirow")
set_policy("build.c++.msvc.runtime", "MD")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.compile_commands.autoupdate")

-- set_targetdir("out/$(plat)-$(arch)-$(mode)")

local is_gcc = is_config("toolchain", "gcc")
local is_clang = is_config("toolchain", "clang", "llvm")

local chrb_modules = {
  "chrb_core",
  "chrb_region",
}

local build_kind
if has_config("lib") then
  build_kind = "static"
else
  build_kind = "binary"
end

-- Helper functions
local function stdlib_config()
    if is_clang and not is_plat("windows") and has_config("stdlib") then
        local std = get_config("stdlib")
        return { cxxflags = "-stdlib=" .. std, ldflags = "-stdlib=" .. std, exceptions = "none" }
    end
    return { exceptions = "none" }
end

local subdirs = "src tests"
local function source_files()
    local files = os.files("src/**.cc")
    table.join2(files, os.files("src/**.h"))

    if has_config("tests") then
        table.join2(files, os.files("tests/**.cc"))
        table.join2(files, os.files("tests/**.h"))
    end
    return files
end

add_requires("fpag 419c9bee39f133e7d0bf0adb3928a172dd561f93", {
  system = false,
  external = true,
  configs = {
    stdlib = get_config("stdlib"),
    libunwind = get_config("libunwind"),
  },
})
add_requires("toml++ v3.4.0", { system = false, configs = table.join(stdlib_config(), { header_only = true }) })

if has_config("tests") then
  add_requires("catch2 v3.13.0", { system = false, configs = stdlib_config() })
end

-- Tasks
task("format")
set_menu({ usage = "xmake format", description = "format source code" })
on_run(function()
  local files = source_files()
  if #files > 0 then
    os.execv(
      "clang-format",
      table.join({
        "--fail-on-incomplete-format",
        "--ferror-limit=1",
        "--sort-includes",
        "-i",
      }, files)
    )
  end
  os.exec("uv sync")
  os.exec("uv run scripts/header_license.py")
end)
task_end()

task("tidy")
set_menu({ usage = "xmake tidy", description = "Run clang-tidy --fix" })
on_run(function()
  local files = source_files()
  if #files > 0 then
    os.execv(
      "clang-tidy",
      table.join(
        { "--use-color", "--fix", "--config-file=./.clang-tidy" },
        files
      )
    )
  end
end)
task_end()

task("lint")
set_menu({
  usage = "xmake lint",
  description = "lint using cpplint & clang-format",
})
on_run(function()
  os.exec("uv sync")
  os.execv("uv", table.join({ "run", "cpplint", "--recursive", "src/" }))
  local files = source_files()
  if #files > 0 then
    os.execv(
      "clang-format",
      table.join({ "--dry-run", "--fail-on-incomplete-format", "-i" }, files)
    )
  end
end)
task_end()

-- Events
after_build(function(target)
  if has_config("timetrace") then
    local trace_dir = path.join(os.projectdir(), "build/timetrace")
    os.mkdir(trace_dir)
    for _, objfile in ipairs(target:objectfiles()) do
      local base = path.directory(objfile) .. "/" .. path.basename(objfile)
      local json = base .. ".json"
      if os.exists(json) then
        os.cp(json, path.join(trace_dir, path.basename(json) .. ".json"))
      end
    end
  end

  if has_config("optreport") and is_mode("release") then
    local remark_dir = "build/remarks"
    os.mkdir(remark_dir)
    for _, yaml in ipairs(os.files(path.join(target:targetdir(), "**.opt.yaml"))) do
      os.cp(yaml, remark_dir)
    end
  end
end)

after_run(function(target)
  if has_config("coverage") and target:name() == "tests" and not is_plat("windows") then
    local profraw = path.join(target:targetdir(), "default.profraw")
    local profdata = path.join(target:targetdir(), "default.profdata")
    local coverage_dir = "build/coverage"

    os.runv("llvm-profdata", { "merge", "-sparse", profraw, "-o", profdata })
    os.runv(
      "llvm-cov", {
        "show",
        target:targetfile(),
        "-instr-profile=" .. profdata,
        "-format=html",
        "-output-dir=" .. coverage_dir,
      }
    )

    cprint("${green}coverage report generated at: " .. path.join(coverage_dir, "index.html"))
  end
end)

-- targets
target("chrb_root_config")
  set_kind("phony", {public = true})
  set_languages("c++23", { public = true })
  set_warnings("all", "extra", "error", "pedantic", {public = true})

  set_encodings("source:utf-8", "utf-8")

  add_defines('CHRB_PROJECT_NAME="' .. project_name .. '"', { public = true })
  add_defines('CHRB_PROJECT_VERSION="' .. project_version .. '"', { public = true })
  add_packages("fpag", { public = true })

  add_includedirs("src", {public = true})
  -- add_includedirs("src", "third_party", {public = true})

  set_exceptions("none", { public = true })
  add_cxxflags("-fno-exceptions", "-fno-rtti", { public = true })

  if is_clang or is_gcc then
    add_cxxflags("-Wconversion", "-Wsign-conversion", "-Wnull-dereference", "-Wformat=2", "-Wundef", { public = true })
    add_cxxflags("-Wnon-virtual-dtor", "-Woverloaded-virtual", { public = true })
    add_cxxflags("-fstack-protector-strong", { public = true })

    if is_mode("debug") and not is_plat("windows") then
      add_cxxflags("-rdynamic", { public = true })
      add_ldflags("-rdynamic", { public = true })
    end
  end

  if is_plat("linux") then
    if is_mode("debug") then
      add_ldflags("-Wl,--build-id", { public = true })
    end
  elseif is_plat("macosx") then
  elseif is_plat("windows") then
  end

  if is_mode("debug") then
    set_symbols("debug", { public = true })
    set_optimize("none", { public = true })
    add_cxxflags("-fno-omit-frame-pointer", "-g3", { public = true })
    add_defines("LLVM_ENABLE_STATS", "LLVM_ENABLE_DUMP", { public = true })
  elseif is_mode("release") then
    set_symbols("hidden", { public = true })

    -- set_optimize("smallest", { public = true })
    -- set_optimize("faster", { public = true })
    set_optimize("fastest", { public = true })

    set_strip("all", { public = true })
  end

  if is_clang and has_config("stdlib") then
    add_cxxflags("-stdlib=" .. get_config("stdlib"), { public = true })
    add_ldflags("-stdlib=" .. get_config("stdlib"), { public = true })
  end

  if has_config("sanitizers") and is_mode("debug") and not is_plat("windows") then
    set_policy("build.sanitizer.address", true)
    -- set_policy("build.sanitizer.memory", true)
    set_policy("build.sanitizer.undefined", true)
    set_policy("build.sanitizer.leak", true)
    -- add_cxflags("-fsanitize=thread")
  end
  if has_config("xray") and is_mode("debug") then
    add_cxxflags("-fxray-instrument", "-fxray-instruction-threshold=200", { public = true })
    add_ldflags("-fxray-instrument", { public = true })
  end
  if has_config("coverage") and not is_plat("windows") then
    add_cxxflags("-fprofile-instr-generate", "-fcoverage-mapping", { public = true })
    add_ldflags("-fprofile-instr-generate", "-fcoverage-mapping", { public = true })
  end
  if has_config("optreport") and is_mode("release") then
    add_cxxflags("-fsave-optimization-record", { public = true })
  end
  if has_config("timetrace") then
    add_cxxflags("-ftime-trace", { public = true })
  end
  if has_config("native") and not is_cross() and is_mode("release") then
    add_cxxflags("-march=native", { public = true })
  end
  if has_config("unitybuild") then
    add_rules("c++.unity_build", { batchsize = 12 })
  end
target_end()

target("chrb_core")
  add_deps("chrb_root_config")
  set_kind("object")
  add_files("src/core/**.cc")
  set_default(false)
target_end()

target("chrb_region")
  add_deps("chrb_root_config")
  set_kind("object")
  add_files("src/region/**.cc")
  set_default(false)
target_end()

target("chrb")
  add_deps("chrb_root_config")
  set_kind(build_kind)
  add_files("src/app/**.cc")
  add_deps(chrb_modules)
  add_packages("toml++")
  set_default(true)
target_end()

target("tests")
  set_enabled(has_config("tests"))
  add_deps("chrb_root_config")
  set_kind("binary")
  add_files("tests/**.cc")
  add_deps(chrb_modules)
  add_packages("catch2")
  add_includedirs("tests", { public = true })

  -- catch2 uses c2y extension in their macro
  if is_clang then add_cxxflags("-Wno-c2y-extensions") end

  set_default(false)
target_end()
