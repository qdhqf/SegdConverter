#include "testviewdialog.h"
#include "ui_testviewdialog.h"
#include "QValueAxis"
#include <QDebug>
QT_CHARTS_USE_NAMESPACE


TestViewDialog::TestViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestViewDialog)
{


    ui->setupUi(this);
    this->setWindowFlags(Qt::Window);
    p_TestModel = new TestModel(this);
    p_TestModel->setTestMap(&testMap);

    p_badFilterTestModel = new TestSortFilterModel(this);
    p_badFilterTestModel->setTestType(TestSortFilterModel::badTest);
    p_badFilterTestModel->setSourceModel(p_TestModel);
    //ui->badPointsTableView->setModel(p_badFilterTestModel);

    p_goodFilterTestModel = new TestSortFilterModel(this);
    p_goodFilterTestModel->setTestType(TestSortFilterModel::okTest);
    p_goodFilterTestModel->setSourceModel(p_TestModel);
    //ui->goodPointsTableView->setModel(p_goodFilterTestModel);




    QChart *chart = new QChart;
    chart->setAnimationOptions(QChart::NoAnimation);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setZValue(50);
    //qDebug()<<chart->margins();
    QMargins margins = chart->margins();
    margins.setTop(40);
    chart->setMargins(margins);
    ui->pointsChartView->setRubberBand(QChartView::RectangleRubberBand);

    p_goodScatterSeries = new QScatterSeries;
    p_goodScatterSeries->setMarkerSize(6.0);
    p_goodScatterSeries->setColor(Qt::green);
    p_goodScatterSeries->setBorderColor(Qt::green);
    p_goodScatterSeries->setName("Пикеты в допусках");
    p_badScatterSeries = new QScatterSeries;
    p_badScatterSeries->setMarkerSize(6.0);
    p_badScatterSeries->setColor(Qt::red);
    p_badScatterSeries->setBorderColor(Qt::red);
    p_badScatterSeries->setName("Пикеты вне допусков");


 /*   p_lastPointScatterSeries = new QScatterSeries;
    p_lastPointScatterSeries->setMarkerSize(4.0);
    p_lastPointScatterSeries->setColor(Qt::blue);
    p_lastPointScatterSeries->setBorderColor(Qt::blue);*/
    //p_badModelMappper = new QVXYModelMapper(this);
    //p_badModelMappper->setXColumn(2);
    //p_badModelMappper->setYColumn(3);
    //p_badModelMappper->setSeries(p_badScatterSeries);
    //p_badModelMappper->setModel(p_badFilterTestModel);



    //p_goodModelMapper = new QVXYModelMapper(this);
    //p_goodModelMapper->setXColumn(2);
    //p_goodModelMapper->setYColumn(3);
    //p_goodModelMapper->setSeries(p_goodScatterSeries);
    //p_goodModelMapper->setModel(p_goodFilterTestModel);
    chart->addSeries(p_goodScatterSeries);
    chart->addSeries(p_badScatterSeries);

 //   chart->addSeries(p_lastPointScatterSeries);

    chart->createDefaultAxes();

    ui->pointsChartView->setRenderHint(QPainter::Antialiasing);
    ui->pointsChartView->setChart(chart);

    ui->pointsChartView->addPointLabel();

    //connect(ui->viewBadPointsCheckBox,SIGNAL(toggled(bool)),this,SLOT(setAllScatterVisible(bool)));
    connect(ui->viewBadPointsCheckBox,SIGNAL(toggled(bool)),this,SLOT(setBadScatterVisible(bool)));
    //connect(ui->viewBadPointsRadioButton,SIGNAL(toggled(bool)),this,SLOT(setBadScatterVisible(bool)));
    connect(ui->viewGoodPointsCheckBox,SIGNAL(toggled(bool)),this,SLOT(setGoodScatterVisible(bool)));
    connect(ui->goodPointsSizeSpinBox,SIGNAL(valueChanged(int)),this,SLOT(setGoodScatterSize(int)));
    connect(ui->badPointsSizeSpinBox,SIGNAL(valueChanged(int)),this,SLOT(setBadScatterSize(int)));
    QValueAxis *axis = qobject_cast<QValueAxis*>(chart->axisX());
    connect(axis,SIGNAL(rangeChanged(qreal,qreal)),ui->pointsChartView,SLOT(chartChanged()));
    connect(this,SIGNAL(sendLineLabel(QPointF,QString)),ui->pointsChartView,SLOT(addLineLabel(QPointF,QString)));


    connect(p_badScatterSeries,SIGNAL(hovered(QPointF,bool)),this,SLOT(mouseUnderPoint(QPointF,bool)));
    connect(p_goodScatterSeries,SIGNAL(hovered(QPointF,bool)),this,SLOT(mouseUnderPoint(QPointF,bool)));
    connect(ui->pointsChartView,SIGNAL(mousePressedWithCtrl(QPointF)),this,SLOT(mousePressedOnChart(QPointF)));

}

void TestViewDialog::setSettings(QSettings *set)
{
    settings = set;
    readSettings();
}

void TestViewDialog::readSettings()
{
    settings->beginGroup("TestViewSettings");
    this->restoreGeometry(settings->value("Geometry").toByteArray());
    ui->viewBadPointsCheckBox->setChecked(settings->value("ViewBadPoints",true).toBool());
    ui->viewGoodPointsCheckBox->setChecked(settings->value("ViewGoodPoints",true).toBool());
    ui->badPointsSizeSpinBox->setValue(settings->value("BadPointsSize",5).toUInt());
    ui->goodPointsSizeSpinBox->setValue(settings->value("GoodPointsSize",5).toUInt());

    p_badScatterSeries->setVisible(ui->viewBadPointsCheckBox->isChecked());
    p_goodScatterSeries->setVisible(ui->viewGoodPointsCheckBox->isChecked());
    settings->endGroup();
}

void TestViewDialog::saveSettings()
{
    settings->beginGroup("TestViewSettings");
    settings->setValue("ViewBadPoints",ui->viewBadPointsCheckBox->isChecked());
    settings->setValue("ViewGoodPoints",ui->viewGoodPointsCheckBox->isChecked());
    settings->setValue("BadPointsSize",ui->badPointsSizeSpinBox->value());
    settings->setValue("GoodPointsSize",ui->goodPointsSizeSpinBox->value());
    settings->setValue("Geometry",this->saveGeometry());
    settings->endGroup();
}


TestViewDialog::~TestViewDialog()
{
    saveSettings();
    delete ui;
}

TestMap* TestViewDialog::getTestMap()
{
    return &testMap;
}

XFileMap* TestViewDialog::getXMap()
{
    return &xMap;
}



void TestViewDialog::newTestReceived()
{
    emit p_TestModel->layoutChanged();
   // p_filterTestModel->invalidate();
//   modelMappper->setFirstRow(0);
//   modelMappper->setRowCount(testMap.count());

    //ui->widget->chart()->axisX()->setRange(547870,551790);
    //ui->widget->chart()->axisY()->setRange(5912175,5914630);
}

//устанавливаем видимость для "хороших" пикетов
void TestViewDialog::setGoodScatterVisible(const bool &b)
{
        p_goodScatterSeries->setVisible(b);
}

void TestViewDialog::setBadScatterVisible(const bool &b)
{
    p_badScatterSeries->setVisible(b);
}
void TestViewDialog::setAllScatterVisible(const bool &b)
{
    if (b)
    {
        p_goodScatterSeries->setVisible(true);
        p_badScatterSeries->setVisible(true);
    }
}


void TestViewDialog::setBadScatterSize(const int &size)
{
    p_badScatterSeries->setMarkerSize(size);
}

void TestViewDialog::setGoodScatterSize(const int &size)
{
    p_goodScatterSeries->setMarkerSize(size);
}

void TestViewDialog::showAuxesByFfid(const uint &f)
{



    XFile xfile = xMap.value(f);

    pointsInTemplate.clear();
    uint counter;
    foreach (Template t, xfile.getTemplates()) {
        counter =0;
        for (uint i = t.firstChannel; i<=t.lastChannel;++i)
        {
            //pointsInTemplate.push_back(t.receiverLine*10000+t.firstReceiver+counter);
            pointsInTemplate.append(testMap.value(qMakePair(t.receiverLine,t.firstReceiver+counter)));
            counter++;
        }
    }
    //qDebug()<<pointsInTemplate.count();
    setBadScatterSeries();
 //   qDebug()<<ui->pointsChartView->chart()->contentsRect();
 //   setGoodScatterSeries();

 //   p_badFilterTestModel->setXFile(xMap.value(f));
 //   p_goodFilterTestModel->setXFile(xMap.value(f));

    /*p_badModelMappper->setFirstRow(0);
    p_badModelMappper->setRowCount(p_badFilterTestModel->rowCount());

    p_goodModelMapper->setFirstRow(0);
    p_goodModelMapper->setRowCount(p_goodFilterTestModel->rowCount());*/

 // qDebug()<<p_goodFilterTestModel->rows123;
  //qDebug()<<p_badFilterTestModel->rows123;
}

/*void TestViewDialog::mouseUnderPoint(cosnt QPointF &coordinates)
{
    qDebug()<<coordinates;
}*/

void TestViewDialog::mouseUnderPoint(const QPointF &coordinates, const bool &b)
{
    if (b)
    {
    qDebug()<<coordinates;
    QVector<TestPoint>::iterator vecIt = pointsInTemplate.begin();
    float minDistance = sqrt(powf(coordinates.x()-vecIt->getX(),2.0)+powf(coordinates.y()-vecIt->getY(),2.0));
    float distance;
    TestPoint tPoint = *vecIt;
    //QPointF point = QPointF(vecIt->getX(),vecIt->getY());
    while (vecIt!=pointsInTemplate.end())
    {
        vecIt++;
        distance = sqrt(powf(coordinates.x()-vecIt->getX(),2.0)+powf(coordinates.y()-vecIt->getY(),2.0));
        if (distance<minDistance)
        {
            minDistance = distance;
            tPoint = *vecIt;
        }
    }
    qDebug()<<tPoint.getLine()<<tPoint.getPoint();
    ui->pointsChartView->setLastPointLabel(tPoint);
    }
    else
    {
        ui->pointsChartView->hideLastPointLabel();
    }
}

void TestViewDialog::mousePressedOnChart(const QPointF &coordinates)
{
    float minDistance;
    float distance;
    QPointF nearestPoint;
    if (p_badScatterSeries->isVisible())
    {
        minDistance = sqrt(powf(coordinates.x()-p_badScatterSeries->at(0).x(),2.0)+powf(coordinates.y()-p_badScatterSeries->at(0).y(),2.0));
    }
    else if (p_goodScatterSeries->isVisible())
    {
        minDistance = sqrt(powf(coordinates.x()-p_goodScatterSeries->at(0).x(),2.0)+powf(coordinates.y()-p_goodScatterSeries->at(0).y(),2.0));
    }
    else
    {   qDebug()<<"No Visible Series";
        return;
    }
    if (p_badScatterSeries->isVisible())
    {
        for (int i=0; i<p_badScatterSeries->count();++i)
        {
            distance =  sqrt(powf(coordinates.x()-p_badScatterSeries->at(i).x(),2.0)+powf(coordinates.y()-p_badScatterSeries->at(i).y(),2.0));
            if (distance<minDistance)
            {
                minDistance = distance;
                nearestPoint = p_badScatterSeries->at(i);
            }
        }
    }
    if (p_goodScatterSeries->isVisible())
    {
        for (int i=0; i<p_goodScatterSeries->count();++i)
        {
            distance =  sqrt(powf(coordinates.x()-p_goodScatterSeries->at(i).x(),2.0)+powf(coordinates.y()-p_goodScatterSeries->at(i).y(),2.0));
            if (distance<minDistance)
            {
                minDistance = distance;
                nearestPoint = p_goodScatterSeries->at(i);
            }
        }
    }
    mouseUnderPoint(nearestPoint,true);
}
void TestViewDialog::setBadScatterSeries()
{

    //p_lastPointScatterSeries->clear();
    ui->pointsChartView->removeLineLabels();
    QVector<QPointF> badPoints;
    QVector<QPointF> goodPoints;

    TestPoint tPoint;
    uint currentLine;
    uint pointsInLine;
    uint badPointsInLine;

    QVector<TestPoint>::iterator vecIt = pointsInTemplate.begin();
    float minX = vecIt->getX();
    float maxX = 0.0;
    float minY = vecIt->getY();
    float maxY = 0.0;
    QPointF lineLabelPoint;


    while (vecIt!=pointsInTemplate.end())
    {
        //tPoint = testMap.value(*vecIt);
        //currentLine = tPoint.getLine();
        currentLine = vecIt->getLine();
        qDebug()<<"Line"<<currentLine;
        pointsInLine = 0;
        badPointsInLine = 0;
        while (currentLine == vecIt->getLine() && vecIt!=pointsInTemplate.end())
        {
            //pointF= QPointF(tPoint.getX(),tPoint.getY());
            if (vecIt->getTestError())
            {
                badPointsInLine++;
                badPoints.append(QPointF(vecIt->getX(),vecIt->getY()));
            }
            else
            {
                goodPoints.append(QPointF(vecIt->getX(),vecIt->getY()));
            }
            lineLabelPoint = QPointF(vecIt->getX(),vecIt->getY());
            minX = vecIt->getX() > minX ? minX : vecIt->getX();
            maxX = vecIt->getX() > maxX ? vecIt->getX() : maxX;
            minY = vecIt->getY() > minY ? minY : vecIt->getY();
            maxY = vecIt->getY() > maxY ? vecIt->getY() : maxY;
            vecIt++;
            pointsInLine++;
            //tPoint = testMap.value(*vecIt);
        }
        qDebug()<<"PointsInLine"<<pointsInLine;
        qDebug()<<"BadPointsInLine"<<badPointsInLine;
        //qDebug()<<pointF;
        //p_lastPointScatterSeries->append(pointF);
        qDebug()<<QPointF(vecIt->getX(),vecIt->getY());
        emit sendLineLabel(lineLabelPoint,QString("ЛП %1\n%2(%3%)").arg(currentLine).arg(badPointsInLine).arg(QString::number(badPointsInLine*100.0/pointsInLine,'g',2)));
    }
    QValueAxis *yAxis = qobject_cast<QValueAxis*>(ui->pointsChartView->chart()->axisY());
    qDebug()<<minX<<" "<<minY;
    yAxis->setRange(minY*0.9999, maxY*1.0001);
    yAxis=qobject_cast<QValueAxis*>(ui->pointsChartView->chart()->axisX());
    yAxis->setRange(minX*0.9999,maxX*1.0001);
    p_badScatterSeries->replace(badPoints);
    p_goodScatterSeries->replace(goodPoints);
    ui->pointsChartView->recountLabelPositions();

}

void TestViewDialog::setGoodScatterSeries()
{
/*    QVector<QPointF> goodPoints;
    TestPoint tPoint;
    for (int i=0; i<pointsInTemplate.count();i++)
    {
        tPoint = testMap.value(pointsInTemplate.at(i));
        if (!tPoint.getTestError())
        {
            goodPoints.append(QPointF(tPoint.getX(),tPoint.getY()));
        }
    }
    qDebug()<<"GoodPOints"<<goodPoints.count();
    p_goodScatterSeries->replace(goodPoints);*/
}



TestChartView::TestChartView(QWidget *parent):QChartView(parent)
{
    //pointRects.append(new PointLabelRect());
}

TestChartView::TestChartView(QChart *chart, QWidget *parent):QChartView(chart,parent)
{

}

void TestChartView::addPointLabel()
{
    pointRects.append(new PointLabelRect(this->chart()));
}

void TestChartView::hideLastPointLabel()
{
    pointRects.last()->hide();
}
void TestChartView::setLastPointLabel(const TestPoint &point)
{
    //QPoint
    pointRects.last()->setTestPoint(point);
    pointRects.last()->setZValue(99);
    QPointF pos = this->chart()->mapToPosition(point.getPointF()) + QPointF(20,-30);
    if (pos.x()+pointRects.last()->getRect().width() > this->width())
    {
        pos.setX(pos.x()-40.0-pointRects.last()->getRect().width());
    }
    //pointRects.last()->setPos(this->chart()->mapToPosition(QPointF(point.getX(),point.getY())) + QPoint(20,-30));
    pointRects.last()->setPos(pos);
    pointRects.last()->show();
}


void TestChartView::addLineLabel(const QPointF coordinates,const QString &txt)
{
    //lineLabels.append(new LineLabelTextItem(coordinates,txt,this->chart()));
    LineLabelRect *labelRect = new LineLabelRect(this->chart());
    labelRect->setAnchor(coordinates);
    labelRect->setText(txt);
    labelRect->setPos(this->chart()->mapToPosition(coordinates) + QPoint(-20, -50));
    //labelRect->setPos(this->chart()->mapToPosition(coordinates));
    labelRect->setZValue(11);
    lineRects.append(labelRect);
}

void TestChartView::removeLineLabels()
{
    //qDeleteAll(lineLabels);
    qDeleteAll(lineRects);
    //lineLabels.clear();
    lineRects.clear();
}

void TestChartView::resizeEvent(QResizeEvent *event)
{

    QChartView::resizeEvent(event);
    repositionLabels();
}

void TestChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->modifiers()==Qt::ControlModifier)
    {
        qDebug()<<"Ctrl Pressed";
        qDebug()<<this->chart()->mapToValue(event->pos());
        emit mousePressedWithCtrl(chart()->mapToValue(event->pos()));
    }
    else{
        QChartView::mousePressEvent(event);
    }
}

void TestChartView::mouseDoubleClickEvent(QMouseEvent *event)
{
    addPointLabel();
    QChartView::mouseDoubleClickEvent(event);
}

void TestChartView::chartChanged()
{
    qDebug()<<"chartChanged";
    repositionLabels();
}

void TestChartView::repositionLabels()
{
    QList<LineLabelRect*>::iterator it = lineRects.begin();
    QPointF newPos;
    QValueAxis *axis = qobject_cast<QValueAxis*>(this->chart()->axisX());
    for ( ; it!=lineRects.end();++it)
    {
        if ((*it)->getAnchor().x()<axis->min() || (*it)->getAnchor().x()>axis->max())
        {
            (*it)->hide();
        }
        else
        {
            newPos = this->chart()->mapToPosition((*it)->getAnchor())+QPoint(-20,-50);
            if (newPos.y()<20)
            {
                newPos.setY(20);
            }
            (*it)->setPos(newPos);
            (*it)->show();
        }
    }
}

void TestChartView::recountLabelPositions()
{
    /*QList<LineLabelTextItem*>::iterator it = lineLabels.begin();
 //   (*it)->setPos(this->chart()->mapToPosition((*it)->getCoordinates()));
    for ( ; it!=lineLabels.end();++it)
    {
        qDebug() << (*it)->getCoordinates();
        qDebug()<<this->chart()->mapToPosition((*it)->getCoordinates());
        (*it)->setPos(this->chart()->mapToPosition((*it)->getCoordinates()));
    }*/
}


LineLabelTextItem::LineLabelTextItem(QGraphicsItem *parent):QGraphicsSimpleTextItem(parent)
{
    coordinates = QPointF(0.0,0.0);
}

LineLabelTextItem::LineLabelTextItem(const QString &text, QGraphicsItem *parent):QGraphicsSimpleTextItem(text,parent)
{
    coordinates = QPointF(0.0,0.0);
}

LineLabelTextItem::LineLabelTextItem(const QPointF &point, const QString &text, QGraphicsItem *parent):QGraphicsSimpleTextItem(text,parent)
{
    coordinates = point;
}

QPointF LineLabelTextItem::getCoordinates()
{
    return coordinates;
}


LineLabelRect::LineLabelRect(QGraphicsItem *parent):
    QGraphicsItem(parent)
{

}

void LineLabelRect::setAnchor(QPointF point)
{
    m_anchor = point;
}

QPointF LineLabelRect::getAnchor()
{
    return m_anchor;
}

void LineLabelRect::setText(const QString &text)
{
   m_text=text;
   QFontMetrics metrics(m_font);
   m_textRect = metrics.boundingRect(QRect(0,0,20,20),Qt::AlignLeft,m_text);
   m_textRect.translate(0,0);
   prepareGeometryChange();
   m_rect=m_textRect.adjusted(-5,-5,5,5);
}

QRectF LineLabelRect::boundingRect() const
{
    return m_rect;
}

void LineLabelRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QPainterPath path;
    path.addRoundedRect(m_rect, 5, 5);
    //path.addEllipse(QPointF(m_rect.center().rx(),m_rect.bottom()),2,2);
    //path.moveTo(QPointF(m_rect.center().rx(),m_rect.bottom()));
    //QChart *parentChart = static_cast<QChart*>(this->parentItem());
    //qDebug()<<
    //QPointF ancor = (parentChart->mapToPosition(m_anchor));
    //path.lineTo(mapFromParent(ancor));
    //QPointF anchor = mapFromParent(m_anchor);
    /*if (!m_rect.contains(anchor)) {
        QPointF point1, point2;

        // establish the position of the anchor point in relation to m_rect
        bool above = anchor.y() <= m_rect.top();
        bool aboveCenter = anchor.y() > m_rect.top() && anchor.y() <= m_rect.center().y();
        bool belowCenter = anchor.y() > m_rect.center().y() && anchor.y() <= m_rect.bottom();
        bool below = anchor.y() > m_rect.bottom();

        bool onLeft = anchor.x() <= m_rect.left();
        bool leftOfCenter = anchor.x() > m_rect.left() && anchor.x() <= m_rect.center().x();
        bool rightOfCenter = anchor.x() > m_rect.center().x() && anchor.x() <= m_rect.right();
        bool onRight = anchor.x() > m_rect.right();

        // get the nearest m_rect corner.
        qreal x = (onRight + rightOfCenter) * m_rect.width();
        qreal y = (below + belowCenter) * m_rect.height();
        bool cornerCase = (above && onLeft) || (above && onRight) || (below && onLeft) || (below && onRight);
        bool vertical = qAbs(anchor.x() - x) > qAbs(anchor.y() - y);

        qreal x1 = x + leftOfCenter * 10 - rightOfCenter * 20 + cornerCase * !vertical * (onLeft * 10 - onRight * 20);
        qreal y1 = y + aboveCenter * 10 - belowCenter * 20 + cornerCase * vertical * (above * 10 - below * 20);;
        point1.setX(x1);
        point1.setY(y1);

        qreal x2 = x + leftOfCenter * 20 - rightOfCenter * 10 + cornerCase * !vertical * (onLeft * 20 - onRight * 10);;
        qreal y2 = y + aboveCenter * 20 - belowCenter * 10 + cornerCase * vertical * (above * 20 - below * 10);;
        point2.setX(x2);
        point2.setY(y2);

        path.moveTo(point1);
        path.lineTo(mapFromParent(m_anchor));
        path.lineTo(point2);
        path = path.simplified();
    }*/
    painter->setBrush(QColor(255, 255, 255));
    painter->drawPath(path);
    painter->drawText(m_textRect, m_text);
}




PointLabelRect::PointLabelRect(QGraphicsItem *parent):
    QGraphicsTextItem(parent)
{

}

QRectF PointLabelRect::boundingRect() const
{
    //return QGraphicsTextItem::boundingRect();
    return m_rect;
}
void PointLabelRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QPainterPath path;
    path.addRoundedRect(m_rect, 5, 5);
    painter->setBrush(QBrush(QColor(255, 255, 255),Qt::SolidPattern));
    painter->drawPath(path);
    //painter->drawText(m_textRect, m_text);*/
    QGraphicsTextItem::paint(painter,option,widget);
}

void PointLabelRect::setTestPoint(const TestPoint &point)
{
    QString text;
    QColor color;
    qDebug()<<point.getLine();
    text = QString ("<table align = center>"
                    "<tr> <td align = center> Линия </td> <td align = center>%1</td></tr>"
                    "<tr> <td align = center> Пикет </td> <td align = center>%2</td></tr>").arg(point.getLine()).arg(point.getPoint());
    if (point.getTestError())
    {
        color = Qt::red;
    }
    color = point.getTiltError()? Qt::red : Qt::green;
    text.append(QString("<tr> <td align = center> Tilt </td> <td align = center bgcolor=%1>%2</td></tr>").arg(color.name()).arg(point.getTilt()));
    color = point.getResistanceError()? Qt::red : Qt::green;
    text.append(QString("<tr> <td align = center> Resistance </td> <td align = center bgcolor=%1>%2</td></tr>").arg(color.name()).arg(point.getResistance()));
    color = point.getLeakageError()? Qt::red : Qt::green;
    text.append(QString("<tr> <td align = center> Leakage </td> <td align = center bgcolor=%1>%2</td></tr>").arg(color.name()).arg(point.getLeakage()));
    text.append("</table");
    /*text =QString("<p>ЛП %1</p>"
                  "<p>ПП %2</p>").arg(point.getLine()).arg(point.getPoint());*/
    this->setHtml(text);
    m_rect = QGraphicsTextItem::boundingRect().adjusted(-3,-3,3,3);
    /*m_tpoint = point;
    m_text = QString("ЛП %1\nПП %2\nTilt %3\nResistance %4\nLeakage %5").arg(m_tpoint.getLine())
            .arg(m_tpoint.getPoint()).arg(m_tpoint.getTilt()).arg(m_tpoint.getResistance())
            .arg(m_tpoint.getLeakage());
    QFontMetrics metrics(m_font);
    m_textRect = metrics.boundingRect(QRect(0,0,150,150),Qt::AlignLeft,m_text);
    m_rect = m_textRect.adjusted(-5,-5,5,5);*/
}


QRectF PointLabelRect::getRect() const
{
    return m_rect;
}
