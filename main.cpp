///////////////////////////////////////////////////////////////////////////////
/// (C) kindid ltd 2018+ //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <QCoreApplication>

#include <here.h>
#include <mld2.h>

// so what i really want to do is, say,
// crash in the middle of a nested loop
// over different iterator types.



int main(int argc, char *argv[])
{
    // should be able to remove this too...
//    mld::init();


    QList<QList<QString *>> bug;
    for(int y = 0; y < 5; y++) {
        QList<QString *> inner_bug;
        for (int x = 0; x < 5; x++) {
            if (x == 2 && y == 3) {
                inner_bug.append(nullptr);
            } else {
                inner_bug.append(new QString(QString("%0 %1").arg(x).arg(y)));
            }
        }
        bug.append(inner_bug);
    }

// I really want to something to fail here - not sure how.
    foreach(auto yy, bug) {
        foreach(auto xx, yy) {
            here << *xx;
        }
    }

    QCoreApplication a(argc, argv);

//    int * bob = nullptr;
    int * bob = &argc;

    int q = 1 / 0;

    return * bob;

//    return a.exec();
}
