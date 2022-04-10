ACLOCAL_AMFLAGS = -I m4

warningflags = \
        -Wall \
        -Wextra \
        -Wshadow \
        -Wunused \
        -Wnull-dereference \
        -Wvla \
        -Wstrict-aliasing \
        -Wuninitialized \
        -Wunused-parameter \
        -Wreorder \
        -Wsign-compare \
        -Wunreachable-code \
        -Wconversion \
        -Wno-sign-conversion \
         $(COMPWARNFLAGS)

if CLG

# Clang
warningflags += \
        -Wshadow-all \
        -Wshorten-64-to-32 \
        -Wint-conversion \
        -Wconditional-uninitialized \
        -Wconstant-conversion \
        -Wunused-private-field \
        -Wbool-conversion \
        -Wextra-semi \
        -Wnullable-to-nonnull-conversion \
        -Wno-unused-private-field \
        -Wno-unused-command-line-argument
# -Wzero-as-null-pointer-constant

else

# GCC
warningflags += \
        -Wredundant-decls \
        -Wno-ignored-attributes \
        -Wno-expansion-to-defined

endif

warnflagscpp =
includeflags = -I$(srcdir)/../../src
commoncflags = $(DEBUGCFLAGS) $(MFLAGS) $(warningflags) $(includeflags) $(STACKREALIGN)
AM_CXXFLAGS  = -std=$(CXXSTD) $(commoncflags) $(warnflagscpp) $(EXTRA_CXXFLAGS)
AM_LDFLAGS   = $(PLUGINLDFLAGS)

lib_LTLIBRARIES = libchickendream.la
check_PROGRAMS = chickendreamtest
chickendreamtest_CXXFLAGS = $(AM_CXXFLAGS)

commonsrc = \
        ../../src/fgrn/Cell.h \
        ../../src/fgrn/Cell.hpp \
        ../../src/fgrn/CellCache.cpp \
        ../../src/fgrn/CellCache.h \
        ../../src/fgrn/GenGrain.cpp \
        ../../src/fgrn/GenGrain.h \
        ../../src/fgrn/GenGrain.hpp \
        ../../src/fgrn/GrainDensity.h \
        ../../src/fgrn/GrainDensity.cpp \
        ../../src/fgrn/PointList.h \
        ../../src/fgrn/PointList.hpp \
        ../../src/fgrn/UtilPrng.h \
        ../../src/fgrn/UtilPrng.hpp \
        ../../src/fgrn/VisionFilter.cpp \
        ../../src/fgrn/VisionFilter.h \
        ../../src/fgrn/VisionFilter.hpp \
        ../../src/fstb/AllocAlign.h \
        ../../src/fstb/AllocAlign.hpp \
        ../../src/fstb/Approx.h \
        ../../src/fstb/Approx.hpp \
        ../../src/fstb/ArrayAlign.h \
        ../../src/fstb/ArrayAlign.hpp \
        ../../src/fstb/CpuId.cpp \
        ../../src/fstb/CpuId.h \
        ../../src/fstb/def.h \
        ../../src/fstb/fnc_fstb.cpp \
        ../../src/fstb/fnc.h \
        ../../src/fstb/fnc.hpp \
        ../../src/fstb/Hash.h \
        ../../src/fstb/Hash.hpp \
        ../../src/fstb/Poly.h \
        ../../src/fstb/Poly.hpp \
        ../../src/fstb/SingleObj.h \
        ../../src/fstb/SingleObj.hpp \
        ../../src/fstb/ToolsSimd.cpp \
        ../../src/fstb/ToolsSimd.h \
        ../../src/fstb/ToolsSimd.hpp \
        ../../src/fstb/VecAlign.h \
        ../../src/fstb/Vf32.h \
        ../../src/fstb/Vf32.hpp \
        ../../src/fstb/Vs32.h \
        ../../src/fstb/Vs32.hpp \
        ../../src/fstb/Vu32.h \
        ../../src/fstb/Vu32.hpp \
        ../../src/chkdr/AvstpScopedDispatcher.cpp \
        ../../src/chkdr/AvstpScopedDispatcher.h \
        ../../src/chkdr/CpuOptBase.cpp \
        ../../src/chkdr/CpuOptBase.h \
        ../../src/chkdr/GrainProc.cpp \
        ../../src/chkdr/GrainProc.h \
        ../../src/avstp.h \
        ../../src/AvstpWrapper.cpp \
        ../../src/AvstpWrapper.h

libchickendream_la_SOURCES = $(commonsrc) \
        ../../src/chkdrvs/Grain_vs.cpp \
        ../../src/chkdrvs/Grain.h \
        ../../src/chkdrvs/version.h \
        ../../src/vsutl/FilterBase.cpp \
        ../../src/vsutl/FilterBase.h \
        ../../src/vsutl/fnc_vsutl.cpp \
        ../../src/vsutl/fnc.h \
        ../../src/vsutl/FrameRefSPtr.h \
        ../../src/vsutl/FuncRefSPtr.h \
        ../../src/vsutl/NodeRefSPtr.h \
        ../../src/vsutl/ObjRefSPtr.h \
        ../../src/vsutl/ObjRefSPtr.hpp \
        ../../src/vsutl/PlaneProcCbInterface_vs.cpp \
        ../../src/vsutl/PlaneProcCbInterface.h \
        ../../src/vsutl/PlaneProcessor_vs.cpp \
        ../../src/vsutl/PlaneProcessor.h \
        ../../src/vsutl/PlaneProcMode.h \
        ../../src/vsutl/Redirect.h \
        ../../src/vsutl/Redirect.hpp \
        ../../src/main-vs.cpp \
        ../../src/types.h \
        ../../src/VapourSynth4.h

libchickendream_la_LDFLAGS = -no-undefined -avoid-version $(PLUGINLDFLAGS)
libchickendream_la_LIBADD =
chickendreamtest_LDADD =
noinst_LTLIBRARIES =

chickendreamtest_SOURCES =  $(commonsrc) \
        ../../src/test/main.cpp


if X86

libsse2_la_SOURCES = \
        ../../src/fstb/ToolsSse2.cpp \
        ../../src/fstb/ToolsSse2.h \
        ../../src/fstb/ToolsSse2.hpp

libsse2_la_CXXFLAGS = $(AM_CXXFLAGS) -msse2
libchickendream_la_LIBADD += libsse2.la
chickendreamtest_LDADD += libsse2.la
noinst_LTLIBRARIES += libsse2.la

commonsrcavx = \
        ../../src/fgrn/GenGrain_avx.cpp


libavx_la_SOURCES = $(commonsrcavx)

libavx_la_CXXFLAGS = $(AM_CXXFLAGS) -mavx
libchickendream_la_LIBADD += libavx.la
chickendreamtest_LDADD += libavx.la
noinst_LTLIBRARIES += libavx.la

commonsrcavx2 =


libavx2_la_SOURCES = $(commonsrcavx2) \
        ../../src/fstb/ToolsAvx2.cpp \
        ../../src/fstb/ToolsAvx2.h \
        ../../src/fstb/ToolsAvx2.hpp

libavx2_la_CXXFLAGS = $(AM_CXXFLAGS) -mavx2
libchickendream_la_LIBADD += libavx2.la
chickendreamtest_LDADD += libavx2.la
noinst_LTLIBRARIES += libavx2.la

endif
