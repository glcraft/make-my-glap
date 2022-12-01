-- add_requires("glap")
add_requires("yaml-cpp")

target("make-my-glap")
    set_kind("binary")
    set_basename("mmg")
    set_languages("cxx20")
    add_files("src/*.cpp")
    add_includedirs("include")
    add_packages("yaml-cpp")
    add_options("use_tl_expected", "use_fmt")
    add_deps("glap")
