if [[ -z "${KATANA_ROOT}" ]]; then
    echo "KATANA_ROOT env variable is not set."
else
    #assume script is sourced and located in installation root
    export OPENDCC_ROOT=`realpath $(/bin/dirname "$_")`
    if [[ ! -d "${OPENDCC_ROOT}/plugin" || \
            ! -d "${OPENDCC_ROOT}/bin" ||
            ! -d "${OPENDCC_ROOT}/qt-plugins"  ]]; then
        echo "ERROR: current directory is not a OpenDCC installation. Cannot continue."
    else
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENDCC_ROOT/lib:$KATANA_ROOT/bin:$KATANA_ROOT/bin/python3.9/lib:$OPENDCC_ROOT/plugin/opendcc:$OPENDCC_ROOT/plugin/usd:$LD_LIBRARY_PATH
        export PATH=$OPENDCC_ROOT/bin:$PATH
        export PYTHONPATH=${OPENDCC_ROOT}/lib/python3.9/site-packages:$KATANA_ROOT/bin/python3.9/site-packages:$KATANA_ROOT/bin/python
        export PYTHONHOME=$KATANA_ROOT/bin/python3.9
        export PXR_PLUGINPATH_NAME=$KATANA_ROOT/plugin/usd
        #used to fix OCIO parsing bug on non english-locales
        export LC_NUMERIC=C
    fi
fi
