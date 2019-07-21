import qbs

Project {
    references: [
        "rpi-rgb-led-matrix/rpi-rgb-led-matrix.qbs"
    ]
CppApplication {
    name: "piclock"

    Depends {
        name: "rpi-rgb-led-matrix"
    }

    cpp.optimization: "none"
    cpp.debugInformation: true
    cpp.cFlags: [
        "-std=gnu11"
    ]
    cpp.cxxFlags: [
        "-std=gnu++17"
    ]
    cpp.commonCompilerFlags: [
        "-Wall", "-Wextra"
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
