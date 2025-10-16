set_project("Sparkland")
set_version("0.0.1", { build = "%Y%m%d%H%M" })
set_languages("cxxlatest")
set_warnings("all", "error")

set_configvar("PROJECT_DIRECTORY", os.projectdir())
set_configvar("PROJECT_VERSION", "0.0.1")

add_rules("mode.debug",
    "mode.release",
    "mode.releasedbg",
    "mode.coverage",
    "mode.profile",
    "mode.check",
    "mode.valgrind",
    "mode.asan",
    "mode.tsan",
    "mode.ubsan",
    "mode.lsan")

if is_mode("releasedbg") then
    set_symbols("debug") 
    set_optimize("fastest")  
    set_strip("none")          
end

-- gRPC & Protobuf
add_requires("protobuf-cpp")
add_requires("grpc", {system = false})


-- Testing & Benchmarking Packages
add_requires("conan::gtest/1.13.0", { alias = "gtest" })
add_requires("conan::benchmark/1.8.3", { alias = "benchmark" })

-- Core & System Packages
add_requires("conan::cli11/2.3.2", { alias = "cli11" })
add_requires("conan::boost/1.86.0", { alias = "boost", configs = { header_only = true }})
add_requires("conan::magic_enum/0.9.6", { alias = "magic_enum" })

-- Data Structures & Algorithms
add_requires("conan::xxhash/0.8.2", { alias = "xxhash" })
add_requires("conan::frozen/1.2.0", { alias = "frozen" })

-- Secutiry & Cryptography
add_requires("conan::openssl/3.5.2", { alias = "openssl" })

-- -- Encoding & Serialization
add_requires("conan::daw_json_link/3.24.1", { alias = "daw_json_link" })
add_requires("conan::libcurl/8.15.0", { alias = "libcurl" })

includes("codec")
includes("core")
includes("concepts")
includes("logger")
includes("meta")
includes("network")
includes("reflect")
includes("result")
includes("types")
includes("protocol")
includes("components")
includes("exchange")
includes("math")
includes("container")
includes("candlestick")
