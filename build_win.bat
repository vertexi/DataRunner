cmake . -Bbuild -DCMAKE_BUILD_TYPE=Debug -DIMGUI_BUNDLE_WITH_IMMVISION=OFF -DIMGUI_BUNDLE_WITH_TEST_ENGINE=OFF -DCMAKE_TOOLCHAIN_FILE=%userprofile%/Documents/My_Files/Git_repos/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Ninja"
cmake --build build --clean-first
