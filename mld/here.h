////////////////////////////////////////////////////////////////
/// (C) 2011-2018 kuiash.com
////////////////////////////////////////////////////////////////

#ifndef _here_h_
#define _here_h_

#include <QDebug>

#define here (qDebug().nospace().noquote() << "file:///" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << ":").space()
#define hrep(X) "" #X "[" << (X) << "]"

#endif
