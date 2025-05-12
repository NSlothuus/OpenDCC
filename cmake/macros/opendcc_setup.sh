#assume script is sourced and located in installation root
export OPENDCC_ROOT="`realpath "$(dirname "$_")"`"
if [[ ! -d "${OPENDCC_ROOT}/plugin" || \
          ! -d "${OPENDCC_ROOT}/bin" ||
          ! -d "${OPENDCC_ROOT}/qt-plugins" ]]; then
    echo "ERROR: current directory is not a OpenDCC installation. Cannot continue."
else
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$OPENDCC_ROOT/lib:$OPENDCC_ROOT/plugin/opendcc:$OPENDCC_ROOT/plugin/usd:$OPENDCC_ROOT"
    export PATH="$OPENDCC_ROOT/bin:$PATH"
    #used to fix OCIO parsing bug on non english-locales
    export LC_NUMERIC=C
fi
