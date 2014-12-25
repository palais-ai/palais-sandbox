QT       -= core gui
TEMPLATE = lib
CONFIG += staticlib
TARGET = meshmagick

HEADERS += include/MeshMagick.h \
include/MeshMagickPrerequisites.h \
include/MmEditableBone.h \
include/MmEditableMesh.h \
include/MmEditableSkeleton.h \
include/MmInfoTool.h \
include/MmInfoToolFactory.h \
include/MmMeshMergeTool.h \
include/MmMeshMergeToolFactory.h \
include/MmMeshUtils.h \
include/MmOgreEnvironment.h \
include/MmOptimiseTool.h \
include/MmOptimiseToolFactory.h \
include/MmOptionsParser.h \
include/MmRenameTool.h \
include/MmRenameToolFactory.h \
include/MmStatefulMeshSerializer.h \
include/MmStatefulSkeletonSerializer.h \
include/MmTool.h \
include/MmToolFactory.h \
include/MmToolManager.h \
include/MmToolUtils.h \
include/MmTransformTool.h \
include/MmTransformToolFactory.h

SOURCES += src/MeshMagick.cpp \
src/MmEditableBone.cpp \
src/MmEditableMesh.cpp \
src/MmEditableSkeleton.cpp \
src/MmInfoTool.cpp \
src/MmInfoToolFactory.cpp \
src/MmMeshMergeTool.cpp \
src/MmMeshMergeToolFactory.cpp \
src/MmMeshUtils.cpp \
src/MmOgreEnvironment.cpp \
src/MmOptimiseTool.cpp \
src/MmOptimiseToolFactory.cpp \
src/MmOptionsParser.cpp \
src/MmRenameTool.cpp \
src/MmRenameToolFactory.cpp \
src/MmStatefulMeshSerializer.cpp \
src/MmStatefulSkeletonSerializer.cpp \
src/MmTool.cpp \
src/MmToolManager.cpp \
src/MmToolsUtils.cpp \
src/MmTransformTool.cpp \
src/MmTransformToolFactory.cpp

INCLUDEPATH += include/

# Copy all headers to build folder
Headers.path = $$OUT_PWD/include
Headers.files = $$files(*.h)
INSTALLS += Headers

include(../linkOgreSDK.pri)
