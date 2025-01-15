set windows-shell := ["pwsh", "-NoLogo", "-NoProfileLoadTime", "-Command"]

set root_dir=%cd%

git config --global http.postBuffer 500M
git config --global http.maxRequestBuffer 100M
git config --global core.compression 0
git clone --recursive https://github.com/quic/adreno-gpu-opengl-es-code-sample-framework

xcopy .\sgsr2 adreno-gpu-opengl-es-code-sample-framework\samples\sgsr2\ /s /e /h /i /y

xcopy .\sgsr2_fs adreno-gpu-opengl-es-code-sample-framework\samples\sgsr2_fs\ /s /e /h /i /y

cd adreno-gpu-opengl-es-code-sample-framework

call .\03_BuildTools.bat

