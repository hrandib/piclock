import qbs

Project {

    references: [
        "rpi-rgb-led-matrix/rpi-rgb-led-matrix.qbs"
    ]

    property string appPath: "/home/pi/wallclock"

Product { name: "cppOptions"

    Export {

        Depends { name: "cpp" }

        cpp.optimization: "fast"
        cpp.debugInformation: false
        cpp.includePaths: [
            "yaml-cpp/include"
        ]
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
        name: "app"
        fileTagsFilter: "application"
        qbs.installPrefix: project.appPath
        qbs.install: true
    }

    Group { name: "config"
        qbs.install: true
        qbs.installPrefix: project.appPath
        prefix: "prebuilt/"
        files: [
            "config.yml"
        ]
    }

    Group { name: "fonts"
        qbs.install: true
        qbs.installPrefix: project.appPath
        prefix: "fonts/"
        files: [
            "*.bdf"
        ]
    }


} //CppApplication

Product { name: "yaml-cpp"

    Depends { name: "cppOptions" }
    Export {
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
}

} //Project
