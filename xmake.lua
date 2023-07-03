add_rules("mode.debug", "mode.release")

add_requires("dpp")
add_requires("fmt")

target("pingme")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("dpp")
    add_packages("fmt")
    set_languages("c++20")

