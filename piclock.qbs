import qbs

Project {
    references: [
        "rpi-rgb-led-matrix/rpi-rgb-led-matrix.qbs"
    ]
CppApplication { name: "piclock"

    Depends {
        name: "rpi-rgb-led-matrix"
    }

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
} //Project
