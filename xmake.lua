add_rules("mode.debug", "mode.release")

add_requires("dpp")
add_requires("fmt")
add_requires("nlohmann_json")

target("pingme")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("dpp")
    add_packages("fmt")
    add_packages("nlohmann_json")
    set_languages("c++20")

