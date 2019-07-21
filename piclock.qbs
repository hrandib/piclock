import qbs

CppApplication {

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
    name: "HelloWorld"
    Group {
        name: "source"
        prefix: "src/"
        files: [
            "multiclock.cpp"
        ]
    }
    Group {
        name: "The app"
        fileTagsFilter: "application"
        qbs.installPrefix:"/home/pi"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
