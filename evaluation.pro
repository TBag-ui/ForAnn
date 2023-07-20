TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH +=googletest-main/googletest/include
INCLUDEPATH +=googletest-main/googletest/

QMAKE_CXXFLAGS_RELEASE += /O2			# Max optimization

SOURCES += \
    BasicStatsTests.cpp \
googletest-main/googletest/src/gtest-all.cc \
googletest-main/googletest/src/gtest-assertion-result.cc \
googletest-main/googletest/src/gtest-death-test.cc \
googletest-main/googletest/src/gtest-filepath.cc \
googletest-main/googletest/src/gtest-matchers.cc \
googletest-main/googletest/src/gtest-port.cc \
googletest-main/googletest/src/gtest-printers.cc \
googletest-main/googletest/src/gtest-test-part.cc \
googletest-main/googletest/src/gtest-typed-test.cc \
googletest-main/googletest/src/gtest.cc \
googletest-main/googletest/src/gtest_main.cc

HEADERS += \
googletest-main/googletest/include/gtest/gtest-assertion-result.h \
googletest-main/googletest/include/gtest/gtest-death-test.h \
googletest-main/googletest/include/gtest/gtest-matchers.h \
googletest-main/googletest/include/gtest/gtest-message.h \
googletest-main/googletest/include/gtest/gtest-param-test.h \
googletest-main/googletest/include/gtest/gtest-printers.h \
googletest-main/googletest/include/gtest/gtest-spi.h \
googletest-main/googletest/include/gtest/gtest-test-part.h \
googletest-main/googletest/include/gtest/gtest-typed-test.h \
googletest-main/googletest/include/gtest/gtest.h \
googletest-main/googletest/include/gtest/gtest_pred_impl.h \
googletest-main/googletest/include/gtest/gtest_prod.h \
googletest-main/googletest/include/gtest/internal/custom/gtest-port.h \
googletest-main/googletest/include/gtest/internal/custom/gtest-printers.h \
googletest-main/googletest/include/gtest/internal/custom/gtest.h \
googletest-main/googletest/include/gtest/internal/gtest-death-test-internal.h \
googletest-main/googletest/include/gtest/internal/gtest-filepath.h \
googletest-main/googletest/include/gtest/internal/gtest-internal.h \
googletest-main/googletest/include/gtest/internal/gtest-param-util.h \
googletest-main/googletest/include/gtest/internal/gtest-port-arch.h \
googletest-main/googletest/include/gtest/internal/gtest-port.h \
googletest-main/googletest/include/gtest/internal/gtest-string.h \
googletest-main/googletest/include/gtest/internal/gtest-type-util.h \
googletest-main/googletest/src/gtest-internal-inl.h

SOURCES += \
        BasicStats.cpp \
        main.cpp

HEADERS += \
    BasicStats.h \
    ForceInline.h
