if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON)
    # disable USD warnings truncation from 'double' to 'float' due to matrix and vector classes in `Gf`
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4305")

    # conversion from size_t to int. While we don't want this enabled it's in the Python headers. So all the Python wrap
    # code is affected.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4715") # treat warning "not all control paths return a value" as an error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4172") # treat warning "returning address of local variable or temporary"
                                                      # as an error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4717") # treat warning "recursive on all control paths, function will
                                                      # cause runtime stack overflow" as an error

    # The /Zc:inline option strips out the "arch_ctor_<name>" symbols used for library initialization by
    # ARCH_CONSTRUCTOR starting in Visual Studio 2019, causing release builds to fail. Disable the option for this and
    # later versions.
    #
    # For more details, see:
    # https://developercommunity.visualstudio.com/content/problem/914943/zcinline-removes-extern-symbols-inside-anonymous-n.html
    if(MSVC_VERSION GREATER_EQUAL 1920)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline- /bigobj")
        # Since v143 toolset use new MSVC more standard-conforming preprocessor
        if(MSVC_VERSION GREATER_EQUAL 1930)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor")
        endif()
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type") # treat warning "not all control paths return a value"
                                                                  # as an error
    if(CMAKE_COMPILER_IS_GNUCXX) # treat warning "returning address of local variable or temporary" as an error
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-local-addr")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-stack-address")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-deprecated -Wno-deprecated-declarations -fdiagnostics-color")
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-deprecated -Wno-deprecated-declarations -fdiagnostics-color")
endif()

if(MSVC)
    add_definitions(
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
        -D__TBB_NO_IMPLICIT_LINKAGE
        -DTBB_USE_CAPTURED_EXCEPTION
        -DBOOST_ALL_DYN_LINK
        -DHAVE_SNPRINTF
        -DTBB_SUPPRESS_DEPRECATED_MESSAGES # disable pragma message when USD uses deprecated tbb API
        -DBOOST_BIND_GLOBAL_PLACEHOLDERS # disable pragma message when Boost.Python includes boost/bind.hpp (bad
                                         # practice declaring placeholders in the global space)
    )
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -DTBB_SUPPRESS_DEPRECATED_MESSAGES -DBOOST_BIND_GLOBAL_PLACEHOLDERS")
endif()
if(DCC_DEBUG_BUILD)
    add_compile_definitions(OPENDCC_DEBUG_BUILD)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    if(WITH_LINKER_GOLD)
        execute_process(
            COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version
            ERROR_QUIET
            OUTPUT_VARIABLE LD_VERSION)
        if("${LD_VERSION}" MATCHES "GNU gold")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=gold")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
        else()
            message(STATUS "GNU gold linker isn't available, using the default system linker.")
        endif()
        unset(LD_VERSION)
    endif()
endif()

if(WITH_WINDOWS_SCCACHE AND CMAKE_VS_MSBUILD_COMMAND)
    message(WARNING "Disabling sccache, sccache is not supported with msbuild")
    set(WITH_WINDOWS_SCCACHE Off)
endif()

if(WITH_WINDOWS_SCCACHE)
    set(CMAKE_C_COMPILER_LAUNCHER sccache)
    set(CMAKE_CXX_COMPILER_LAUNCHER sccache)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
        string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    endif()
else()
    unset(CMAKE_C_COMPILER_LAUNCHER)
    unset(CMAKE_CXX_COMPILER_LAUNCHER)
endif()
