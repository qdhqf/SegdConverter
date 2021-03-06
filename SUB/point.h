#ifndef POINT_H
#define POINT_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QPointF>
struct Template;
class Point
{
public:
    explicit Point();
    Point(QString &str);
    Point(const Point &other);
    void setLine(const int &l);
    void setPoint(const int &p);
    QString getLinePoint();
    uint getX() const;
    uint getY() const;
    uint getZ() const;
    QPointF getPointF() const;

protected:
    int line;
    int point;
    float x;
    float y;
    float z;
    QString linePoint;
};

class TestPoint: public Point
{
public:
    //TestPoint():Point() {testStatus =true};// {testStatus=true};
    TestPoint();
    TestPoint(const TestPoint &other);
    void setX(const float &f);
    void setY(const float &f);
    void setZ(const float &f);

    void setResistance(const float &f);
    void setTilt(const float &f);
    void setLeakage(const float &f);

    void setResistanceError(const bool &b);
    void setTiltError(const bool &b);
    void setLeakageError(const bool &b);

    void setTestStatus(const bool &b);

    void checkTestStatus();


    int getLine() const;
    int getPoint() const;
    float getX() const;
    float getY() const;
    float getZ() const;

    float getLeakage() const;
    float getTilt() const;
    float getResistance() const;

    bool getLeakageError() const;
    bool getTiltError() const;
    bool getResistanceError() const;

    bool getTestError() const;

private:
    bool testError;
    float resistanceValue;
    bool resistanceError;
    float tiltValue;
    bool tiltError;
    float leakageValue;
    bool leakageError;

};

class XFile
{

public:
    explicit XFile();
    explicit XFile(QQueue<QString> strList);
    explicit XFile(const uint &fileNum,const uint &line, const uint &point);

    uint getLine();
    uint getPoint();
    uint getFfid();
    QQueue<Template> getTemplates();
    bool checkTemplates();
    void addLineToTemplate(Template templ);
    void setFfid(const uint &f);
    void setSourceLine(const uint &line);
    void setSourcePoint (const uint &point);
    bool pointInTemplate(const uint &line, const uint &point) const;
private:
    uint ffid;
    uint sourceLine;
    uint sourcePoint;
    QQueue<Template> templates;

    //QVector<Template> templates;
};

struct Template
{
    Template();
    Template(const QString &str);
    Template(const Template &other);
    uint increment;
    uint firstChannel;
    uint lastChannel;
    uint receiverLine;
    uint firstReceiver;
    uint lastReceiver;

    uint badChannels;
    bool lineStatus;
};

struct TestLimits
{
    TestLimits(){
        maxInRow =0;
        maxInLine =0.0;
        maxAll =0.0;
    }
    uint maxInRow;
    float maxInLine;
    float maxAll;
};

#endif // POINT_H
