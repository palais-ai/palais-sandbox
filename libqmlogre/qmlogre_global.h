#ifndef QMLOGRE_GLOBAL_H
#define QMLOGRE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QMLOGRE_LIBRARY)
#  define DLL_EXPORT Q_DECL_EXPORT
#else
#  define DLL_EXPORT Q_DECL_IMPORT
#endif

#endif // QMLOGRE_GLOBAL_H
