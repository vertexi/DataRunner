if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
call cmake . -Bbuild  -DCMAKE_MAKE_PROGRAM="C:/Program Files/CMake/bin/ninja.exe" -DCMAKE_BUILD_TYPE=Debug -DIMGUI_BUNDLE_WITH_IMMVISION=OFF -DIMGUI_BUNDLE_WITH_TEST_ENGINE=OFF -DCMAKE_TOOLCHAIN_FILE=%userprofile%/Documents/My_Files/Git_repos/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Ninja"
call cmake --build build
