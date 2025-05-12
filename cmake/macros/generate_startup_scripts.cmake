
if(WIN32)
    set(BIN_NAME dcc_base.exe)
elseif(NOT WIN32)
    set(BIN_NAME dcc_base)
    set(SCRIPT_NAME opendcc)
endif()


function(gen_startup_script RENDER_DELEGATE_NAME ADDITIONAL_ENV_CONF)
    if(WIN32)
        install(
            CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/opendcc.${RENDER_DELEGATE_NAME}.bat\"
\"@echo off
setlocal
${ADDITIONAL_ENV_CONF}
set PXR_PLUGINPATH_NAME=%~dp0/plugin/${RENDER_DELEGATE_NAME};%PXR_PLUGINPATH_NAME%
call \\\"%~dp0bin\\\\${BIN_NAME}\\\" %*\"
            )")
    elseif(NOT WIN32)
        install(
            CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/${SCRIPT_NAME}.${RENDER_DELEGATE_NAME}.sh\"
\"${ADDITIONAL_ENV_CONF}
export PXR_PLUGINPATH_NAME=$PXR_PLUGINPATH_NAME:$(dirname $(realpath $0))/plugin/${RENDER_DELEGATE_NAME}
cd $(dirname $(realpath $0))/bin/
./${BIN_NAME}\"
)")
    endif()
endfunction()

if(WIN32)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/opendcc.bat\"
    \"@echo off
setlocal
call \\\"%~dp0bin\\\\${BIN_NAME}\\\" %*\"
                )")
endif()

if(DCC_BUILD_ARNOLD_SUPPORT
   OR DCC_USD_FALLBACK_PROXY_BUILD_ARNOLD_USD)
    if(WIN32)
        set(ADDITIONAL_ENV_CONF
            "REM Uncomment the lines below and insert a path where Arnold is installed
REM set ARNOLD_ROOT=C:\\\\Program Files\\\\arnold-${ARNOLD_VERSION}\\\\
REM set PATH=%ARNOLD_ROOT%\\\\bin;%PATH%")
        gen_startup_script(arnold "${ADDITIONAL_ENV_CONF}")
    elseif(NOT WIN32)
        set(ADDITIONAL_ENV_CONF
            "# Uncomment the lines below and insert a path where Arnold is installed
# export ARNOLD_ROOT=/path/to/arnold-${ARNOLD_VERSION}/
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ARNOLD_ROOT/bin")
        gen_startup_script(arnold "${ADDITIONAL_ENV_CONF}")
    endif()
endif()

if(DCC_USD_FALLBACK_PROXY_BUILD_CYCLES )
    if(WIN32)
        set(ADDITIONAL_ENV_CONF "set PATH=%~dp0\\\\plugin\\\\cycles;%PATH%")
        gen_startup_script(cycles "${ADDITIONAL_ENV_CONF}")
    elseif(NOT WIN32)
        gen_startup_script(cycles "")
    endif()
endif()

if(DCC_BUILD_RENDERMAN_SUPPORT)
    if(WIN32)
        set(ADDITIONAL_ENV_CONF
            "REM Uncomment the line below and insert a path to RenderMan root directory
rem set RMANTREE=C:\\\\Program Files\\\\Pixar\\\\RenderManProServer-${RENDERMAN_VERSION_MAJOR}.${RENDERMAN_VERSION_MINOR}
if \\\"%RMANTREE%\\\"==\\\"\\\" (
    echo \\\"ERROR: RMANTREE is not defined.\\\"
    exit /b -1
)
set PATH=%~dp0\\\\plugin\\\\renderman;%PATH%")
        gen_startup_script(renderman "${ADDITIONAL_ENV_CONF}")
    elseif(NOT WIN32)
        set(ADDITIONAL_ENV_CONF
            "# Uncomment the line below and insert a path to RenderMan root directory
# export RMANTREE=/path/to/RenderManProServer-${RENDERMAN_VERSION_MAJOR}.${RENDERMAN_VERSION_MINOR}
if [ -z \\\"$RMANTREE\\\" ]
then
     echo \\\"ERROR: RMANTREE is not defined.\\\"
     exit -1
fi
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(dirname $(realpath $0))/plugin/renderman")
        gen_startup_script(renderman "${ADDITIONAL_ENV_CONF}")
    endif()
endif()

if(DCC_HOUDINI_SUPPORT AND WIN32)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/opendcc.karma.bat\"
\"@echo off
setlocal
REM Uncomment the line below and insert a path to Houdini root directory
rem set HOUDINI_ROOT=c:\\\\Program Files\\\\Side Effects Software\\\\Houdini${HOUDINI_VERSION}
set PATH=%HOUDINI_ROOT%/bin;%PATH%
set PYTHONPATH=%~dp0/python/lib/site-packages;%HOUDINI_ROOT%/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}/lib/site-packages;%HOUDINI_ROOT%/houdini/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}libs
set PYTHONHOME=%HOUDINI_ROOT%/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}
set PXR_PLUGINPATH_NAME=%HOUDINI_ROOT%/houdini/dso/usd_plugins;%DCC_ROOT%/plugin/karma
call \\\"%~dp0bin\\\\${BIN_NAME}\\\" %*\"
        )")

    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/opendcc.houdini.bat\"
\"@echo off
setlocal
REM Uncomment the line below and insert a path to Houdini root directory
rem set HOUDINI_ROOT=c:\\\\Program Files\\\\Side Effects Software\\\\Houdini${HOUDINI_VERSION}
set DCC_ROOT=%~dp0
set PATH=%HOUDINI_ROOT%/bin;%DCC_ROOT%/bin;%DCC_ROOT%/plugin/usd;%DCC_ROOT%/plugin/opendcc;%PATH%
set PYTHONPATH=%DCC_ROOT%/python/lib/site-packages;%HOUDINI_ROOT%/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}/lib/site-packages;%HOUDINI_ROOT%/houdini/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}libs
set PYTHONHOME=%HOUDINI_ROOT%/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}
set PXR_PLUGINPATH_NAME=%HOUDINI_ROOT%/houdini/dso/usd_plugins;%DCC_ROOT%/plugin/usd;%DCC_ROOT%/plugin/opendcc;%DCC_ROOT%/plugin/karma
call \\\"%HOUDINI_ROOT%/bin/houdinifx.exe\\\" %*\"
            )")

    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/opendcc.husk.bat\"
\"@echo off
setlocal
REM Uncomment the line below and insert a path to Houdini root directory
rem set HOUDINI_ROOT=c:\\\\Program Files\\\\Side Effects Software\\\\Houdini${HOUDINI_VERSION}
set DCC_ROOT=%~dp0
set PATH=%HOUDINI_ROOT%/bin;%DCC_ROOT%/bin;%DCC_ROOT%/plugin/usd;%DCC_ROOT%/plugin/opendcc;%PATH%
set PYTHONPATH=%DCC_ROOT%/python/lib/site-packages;%HOUDINI_ROOT%/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}/lib/site-packages;%HOUDINI_ROOT%/houdini/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}libs
set PYTHONHOME=%HOUDINI_ROOT%/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}
set PXR_PLUGINPATH_NAME=%HOUDINI_ROOT%/houdini/dso/usd_plugins;%DCC_ROOT%/plugin/usd;%DCC_ROOT%/plugin/opendcc;%DCC_ROOT%/plugin/karma
call \\\"%HOUDINI_ROOT%/bin/husk.exe\\\" %*\"
            )")
endif()

if(DCC_KATANA_SUPPORT AND WIN32)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/opendcc.katana.bat\"
\"@echo off
setlocal
REM Uncomment the line below and insert a path to Katana root directory
rem set KATANA_ROOT=c:\\\\Program Files\\\\Katana6.0v1
set PATH=%KATANA_ROOT%/bin;%PATH%
set PYTHONPATH=%~dp0/python/lib/site-packages;%KATANA_ROOT%/bin/lib/site-packages;%KATANA_ROOT%/bin/python
set PYTHONHOME=%KATANA_ROOT%/bin
set PXR_PLUGINPATH_NAME=%KATANA_ROOT%/bin/usd
call \\\"%~dp0bin\\\\${BIN_NAME}\\\" %*\"
        )")
endif()
