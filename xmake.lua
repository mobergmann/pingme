add_rules("mode.debug", "mode.release")

add_requires("dpp")
add_requires("fmt")
add_requires("yaml-cpp")

target("pingme")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("dpp")
    add_packages("fmt")
    add_packages("yaml-cpp")
    set_languages("c++20")