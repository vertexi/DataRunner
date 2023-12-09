CALL emsdk_env
CALL emcmake cmake . -Bbuild_emscripten -DIMGUI_BUNDLE_WITH_IMMVISION=OFF -DIMGUI_BUNDLE_WITH_TEST_ENGINE=OFF -G "Ninja"
CALL ninja -C ./build_emscripten
