### Setup and Build Instructions

If you don't already have Conan on your system, find instructions here: https://conan.io/downloads.html

To configure and build a debug build, use:
```conan build . -s build_type=Debug --build=missing```

To configure and build a release build, use:
```conan build . -s build_type=Release -s compiler.runtime=static --build=missing```

Generated files will go into the **build/Release/bin** or **build/Debug/bin** folder.

### License Information

See [LICENSE](LICENSE) for license information about the code (which is under an MIT license).
