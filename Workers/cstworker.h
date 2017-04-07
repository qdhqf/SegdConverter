#ifndef CSTWORKER_H
#define CSTWORKER_H

#include <Workers/baseworker.h>
class CstFile;
class CstWorker : public BaseWorker
{
    Q_OBJECT
public:
    explicit CstWorker(volatile bool *running, CountedAttributes *attr) : BaseWorker(running,attr) {}

protected:
    void countAttributes(CstFile *cst);
    bool convertOneFile(const QString &filePath);
public slots:

    virtual void Converting();
};

#endif // CSTWORKER_H
