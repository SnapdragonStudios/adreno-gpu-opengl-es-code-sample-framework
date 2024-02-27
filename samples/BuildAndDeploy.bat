cd %~dp0..

call 03_BuildTools.bat

cd %~dp0motion_estimation\build\android
call gradlew assembleRelease
adb install -r %~dp0motion_estimation\build\android\app\build\outputs\apk\release\motion_estimation-release.apk

cd %~dp0hello_gltf\build\android
call gradlew assembleRelease
adb install -r %~dp0hello_gltf\build\android\app\build\outputs\apk\release\hello_gltf-release.apk

cd %~dp0amfe_power_saving\build\android
call gradlew assembleRelease
adb install -r %~dp0amfe_power_saving\build\android\app\build\outputs\apk\release\amfe_power_saving-release.apk

cd %~dp0hello_fw\build\android
call gradlew assembleDebug
adb install -r %~dp0hello_fw\build\android\app\build\outputs\apk\debug\hello_fw-debug.apk

cd %~dp0avoid_gmem_loads\build\android
call gradlew assembleDebug
adb install -r %~dp0avoid_gmem_loads\build\android\app\build\outputs\apk\debug\avoid_gmem_loads-debug.apk

cd %~dp0msaa\build\android
call gradlew assembleDebug
adb install -r %~dp0msaa\build\android\app\build\outputs\apk\debug\msaa-debug.apk

cd %~dp0reduce_gmem_stores\build\android
call gradlew assembleDebug
adb install -r %~dp0reduce_gmem_stores\build\android\app\build\outputs\apk\debug\reduce_gmem_stores-debug.apk

cd %~dp0shading_rate\build\android
call gradlew assembleDebug
adb install -r %~dp0shading_rate\build\android\app\build\outputs\apk\debug\shading_rate-debug.apk