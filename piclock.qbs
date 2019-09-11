import qbs

Project {

    references: [
        "rpi-rgb-led-matrix/rpi-rgb-led-matrix.qbs"
    ]

    property string appPath: "/home/pi/wallclock"

Product { name: "cppOptions"

    Export {

        Depends { name: "cpp" }

        cpp.optimization: "small"
        cpp.debugInformation: false
        cpp.includePaths: [
            "inc",
            "yaml-cpp/include"
        ]
        cpp.cFlags: [
            "-std=c11"
        ]
        cpp.cxxFlags: [
            "-std=c++17"
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

    Group { name: "source"
        prefix: "src/"
        files: [
            "clock_impl.cpp",
            "ledwidget.cpp",
            "piclock.cpp",
        ]
    }

    Group { name: "include"
        prefix: "inc/"
        files: [
            "clock_impl.h",
            "common.h",
            "ledwidget.h",
            "options.h",
            "sensors.h",
        ]
    }

    Group { name: "app"
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
        qbs.installDir: "fonts"
        prefix: "rpi-rgb-led-matrix/fonts/"
        files: [
            "*.bdf"
        ]
    }

    Group { name: "fonts-aux"
        qbs.install: true
        qbs.installPrefix: project.appPath
        qbs.installDir: "fonts-aux"
        prefix: "fonts-aux/"
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
