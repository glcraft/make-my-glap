includes("xmake/**/xmake.lua")
-- add_requires("glap")
add_requires("yaml-cpp")
add_requires("tl_expected")
add_requires("fmt")

if not constellation then
    add_requires("glap")
end

target("mmg")
    set_kind("binary")
    set_basename("mmg")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_includedirs("include")
    add_packages("yaml-cpp", "fmt", "tl_expected", "glap")
    if constellation then
        add_deps("glap")
    else 
        add_packages("glap")
    end
    -- add_rules("glap.mmg")
    -- add_files("mmg/args.glap.yml")
    add_installfiles("xmake/rules/(**/*.lua)", {prefixdir = "rules"})
