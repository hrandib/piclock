import qbs

Project {

    references: [
        "rpi-rgb-led-matrix/rpi-rgb-led-matrix.qbs"
    ]

Product { name: "cppOptions"

    Export {

        Depends { name: "cpp" }

        cpp.optimization: "fast"
        cpp.debugInformation: false
        cpp.cFlags: [
            "-std=gnu11"
        ]
        cpp.cxxFlags: [
            "-std=gnu++17"
        ]
        cpp.linkerFlags: [
            "-lpthread"
        ]
        cpp.commonCompilerFlags: [
            "-Wall", "-Wextra", "-Wno-unused-parameter"
        ]
    }
} //cppOptions

CppApplication { name: "piclock"

    Depends { name: "cppOptions" }
    Depends { name: "rpi-rgb-led-matrix" }
    Depends { name: "yaml-cpp" }

    Group {
        name: "source"
        prefix: "src/"
        files: [
            "piclock.cpp"
        ]
    }

    Group {
        name: "The app"
        fileTagsFilter: "application"
        qbs.installPrefix:"/home/pi"
        qbs.install: true
        qbs.installDir: "bin"
    }

} //CppApplication

StaticLibrary { name: "yaml-cpp"

    Depends { name: "cppOptions" }

    cpp.includePaths: [
        "yaml-cpp/include"
    ]
    Group { name: "yaml_inc"
        prefix: "yaml-cpp/"
        files: [
            "include/yaml-cpp/**/*.h",
            "src/**/*.h"
        ]
    }
    Group { name: "yaml_src"
        prefix: "yaml-cpp/src/"
        files: [
            "**/*.cpp"
        ]
    }
}

} //Project
