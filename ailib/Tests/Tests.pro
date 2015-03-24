TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()




win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../AICore/release/ -lailib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../AICore/debug/ -lailib
else:unix: LIBS += -L$$OUT_PWD/../AICore/ -lailib

INCLUDEPATH += $$PWD/../AICore
DEPENDPATH += $$PWD/../AICore

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../AICore/release/libailib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../AICore/debug/libailib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../AICore/release/ailib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../AICore/debug/ailib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../AICore/libailib.a



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../LinearMath/release/ -lLinearMath
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../LinearMath/debug/ -lLinearMath
else:unix: LIBS += -L$$OUT_PWD/../LinearMath/ -lLinearMath

INCLUDEPATH += $$PWD/../LinearMath
DEPENDPATH += $$PWD/../LinearMath

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/release/libLinearMath.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/debug/libLinearMath.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/release/LinearMath.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/debug/LinearMath.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/libLinearMath.a
