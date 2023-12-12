CALL emsdk_env
CALL emcmake cmake . -Bbuild_emscripten -G "Ninja"
CALL cmake --build build_emscripten
