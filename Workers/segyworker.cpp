#include "segyworker.h"
#include <Segd/segdfile.h>
#include <Segy/segyfile.h>
#include "aquila/functions.h"

QTXLSX_USE_NAMESPACE

void SegyWorker::Converting()
{
    QFileInfo fInfo(segdPath);
    QDir segdDir;

    logFile = new QFile(outPath+"_log.txt");
    logFile->open(QIODevice::Text|QIODevice::WriteOnly);
    logStream = new QTextStream(logFile);
    logStream->setCodec("UTF8");
    *logStream << QString("%1 Начало конвертации\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));

    int fileForConvertingNum;
    QFileInfoList segdFilesInDir;
    QStringList filter;
    filter << "*.segd";






    if (fInfo.isDir() && mode)
    {
        segdDir.setPath(segdPath);
        segdFilesInDir = segdDir.entryInfoList(filter,QDir::Files,QDir::Name);
        fileForConvertingNum =0;
    }
    else if (fInfo.isFile() && !mode)
    {
        segdDir=fInfo.dir();
        segdFilesInDir = segdDir.entryInfoList(filter,QDir::Files,QDir::Name);
        fileForConvertingNum = segdFilesInDir.indexOf(fInfo);
    }
    else
    {
        if (mode)
        {
            *logStream << QString ("Не найдена директория %1\n").arg(segdPath);
            emit sendSomeError(QString ("Не найдена директория %1").arg(segdPath));
        }
        else
        {
            *logStream << QString ("Не найден файл %1\n").arg(segdPath);
            emit sendSomeError(QString ("Не найден файл %1").arg(segdPath));
        }
        *logStream << QString("%1 Завершение конвертации\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));
        delete logStream;
        logFile->close();
        delete logFile;
        emit finished();
        return;
    }
    int fileCount = 0; // количество ф.н. в сводном файле
    writeFileHeaders = true;
    run = new bool;
    *run=true;
    while (fileForConvertingNum<segdFilesInDir.count() && *run)
    {
        if (convertOneFile(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath(),writeFileHeaders))
        {
            fileCount++;
            writeFileHeaders = false;
            *logStream << QString("%1 Выполнена конвертация файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(segdFilesInDir.value(fileForConvertingNum).fileName());
            emit sendInfoMessage("Выполнена конвертация файла "+segdFilesInDir.value(fileForConvertingNum).fileName(),Qt::darkGreen);
        }
        else {
            *logStream << QString("%1 Ошибка чтения файла %2. Переход к следующему файлу ").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(segdFilesInDir.value(fileForConvertingNum).fileName());
            emit sendSomeError(QString ("Ошибка чтения файла %1. Переход к следующему файлу").arg(segdFilesInDir.value(fileForConvertingNum).fileName()));
        }

        if (limitMaxFiles && fileCount >= maxFilesValue)
        {
            fileCount=0;
            outPath.insert(outPath.lastIndexOf('/')+1,'_');
            outAuxesPath.insert(outAuxesPath.lastIndexOf('/')+1,'_');
            writeFileHeaders = true;
            createFileForMissedTraces();
        }
        fileForConvertingNum++;
    }
    *logStream << QString("%1 Завершение конвертации\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));
    delete logStream;
    logFile->close();
    delete logFile;
    emit finished();
}

void SegyWorker::countAttributesFromFile(SegyFile *sgy)
{
    QVector<QVector<float> > tracesInWindow;
    QMap<QString,float> amplitudes;
    for (int i=0; i<windows.count();++i)
    {
        *logStream << QString("%1 Расчет атрибутов в окне Min Offset = %2; Max Offset = %3; Min Time = %4мс; Max Time = %5мс\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(windows.at(i).minOffset).arg(windows.at(i).maxOffset).arg(windows.at(i).minTime).arg(windows.at(i).maxTime);
        switch (exType) {
        case exclusionType::txtExcl:
            tracesInWindow = sgy->getDataInWindow(logStream,windows.at(i).minOffset,windows.at(i).maxOffset,windows.at(i).minTime,windows.at(i).maxTime,notUseMutedTraces,badTests,minAmpl,exclPoints);
            break;
        case exclusionType::mesaExcl:
            tracesInWindow = sgy->getDataInWindow(logStream,windows.at(i).minOffset,windows.at(i).maxOffset,windows.at(i).minTime,windows.at(i).maxTime,notUseMutedTraces,badTests,minAmpl,exclusions);
            break;
        default:
            tracesInWindow = sgy->getDataInWindow(logStream,windows.at(i).minOffset,windows.at(i).maxOffset,windows.at(i).minTime,windows.at(i).maxTime,notUseMutedTraces,badTests,minAmpl);
            break;
        }
        /*

        if (useExclusions)
        {
            if  (exType == exclusionType::txtExcl) {
                tracesInWindow = sgy->getDataInWindow(logStream,windows.at(i).minOffset,windows.at(i).maxOffset,windows.at(i).minTime,windows.at(i).maxTime,notUseMutedTraces,badTests,minAmpl,exclPoints);
            }
            else
            {
                tracesInWindow = sgy->getDataInWindow(logStream,windows.at(i).minOffset,windows.at(i).maxOffset,windows.at(i).minTime,windows.at(i).maxTime,notUseMutedTraces,badTests,minAmpl,exclusions);
            }
        }
        else {
            tracesInWindow = sgy->getDataInWindow(logStream,windows.at(i).minOffset,windows.at(i).maxOffset,windows.at(i).minTime,windows.at(i).maxTime,notUseMutedTraces,badTests,minAmpl);
        }*/
        countAttriburesInWindow(tracesInWindow,i,sgy->getSampleInterval(),sgy->getFileNumFirstTrace(),&amplitudes);
        tracesInWindow.clear();
    }
    countRelations(amplitudes);
    /*
    foreach (QString str, relations) {
        QString tmp = str.left(str.lastIndexOf("/"));
        float a = amplitudes.value(tmp);
        tmp = str.mid(str.indexOf("/")+1,str.indexOf(">")-str.indexOf("/")-1);
        float b = amplitudes.value(tmp);
        float attribute = a/b;
        tmp = str.mid(str.lastIndexOf(">")+1);
        float c = tmp.toFloat();
        bool checkAttribute = attribute>c ? true:false;
        fileAttributes.append(qMakePair(attribute,checkAttribute));
    }*/
}

bool SegyWorker::convertOneFile(const QString &filePath, const bool &writeHeaders)
{
    QString fileForWork;
    SegdFile *segd;
    SegyFile *segy;
    fileAttributes.clear();
    *logStream << QString("%1 Начало обработки файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(filePath);
    emit sendInfoMessage(QTime::currentTime().toString(),Qt::black);
    if (backup)
    {
        *logStream << QString("%1 Копирование файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(filePath);
        emit sendInfoMessage("Копирование файла " + filePath,Qt::black);
        fileForWork = BackupFolder +"/"+filePath.mid(filePath.lastIndexOf('/')+1);// segdFilesInDir.value(fileForConvertingNum).fileName();
        if (QFile::exists(fileForWork))
        {
            QFile::remove(fileForWork);
        }
        if (!QFile::copy(filePath,fileForWork))
        {
            backup = false;
            fileForWork = filePath;
            *logStream << QString("%1 Ошибка создания резервной копии. Копирование файлов не производится").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));
            emit sendSomeError("Ошибка создания резервной копии \n Копирование файлов не производится");
        }
    }
    else {
        fileForWork = filePath;
    }
    *logStream << QString("%1 Конвертация файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(fileForWork);
    emit sendInfoMessage("Обработка файла " +fileForWork,Qt::black);
    if (SercelMpFactor)
    {
        segd = new SegdFile(fileForWork);
    }
    else
    {
        segd = new SegdFile(fileForWork,userMpFactorValue);
    }
    if (segd->getFileState())
    {
        segy = new SegyFile(segd,this);
        if (useExternalXps)
        {
            QQueue<QString>* templates = findTemplates(segy->getFileNumFirstTrace());
            if (!templates->isEmpty())
            {
                XFile *xF = new XFile(*templates);
                if (xF->checkTemplates())
                {
                    if (!segy->setTemplates(xF))
                    {
                        sendSomeError(QString("Несоответсвие количества трасс в XPS и SEGD файлах. Геометрия присваивается из заголовков"));
                    }
                }
                else
                {
                    sendSomeError(QString("Некорректные расстановки в XPS файле. Геометрия присваивается из заголовков"));
                }
            }
            else
            {
                emit sendSomeError(QString("Файл № %1 не найден в XPS файле. Геометрия присваивается из заголовков").arg(segy->getFileNumFirstTrace()));
            }

        }


        if (!pp.isEmpty())
        {
            segy->setReceiverCoordinats(pp);
        }
        if (!pv.isEmpty())
        {
            segy->setSourceCoordinats(pv);
        }
        segy->setGeometry();
        fileAttributes.append(qMakePair(segy->getFileNumFirstTrace(),true));
        fileAttributes.append(qMakePair(segy->getSP(),true));
        fileAttributes.append(qMakePair(segy->getgetShotPointNum(),true));
        fileAttributes.append(qMakePair(segy->getSourceX(0),true));
        fileAttributes.append(qMakePair(segy->getSourceY(0),true));
        fileAttributes.append(qMakePair(segy->getSourceZ(0),true));
        if (analysisAuxes)
        {
            chekingAuxData(segd);
        }
        if (checkTests)
        {
            checkingTests(segd);
        }
        delete(segd);
        if (writeHeaders)
        {
            if (auxMode==writeAuxesMode::writeInNewFile)
            {
                segy->writeHeaders(outAuxesPath);
            }
            segy->writeHeaders(outPath);
        }
        switch (auxMode) {
        case writeAuxesMode::writeInNewFile:
            segy->writeAuxTraces(outAuxesPath);
            break;
        case writeAuxesMode::write:
            segy->writeAuxTraces(outPath);
            break;
        default:
            break;
        }
        /*

        if (writeAuxesNewFile)
        {
            segy->writeAuxTraces(outAuxesPath);
        }
        if (writeAuxes)
        {
            segy->writeAuxTraces(outPath);
        }*/
        segy->writeTraces(outPath,writeMutedChannels,writeMissedChannels);
        countAttributesFromFile(segy);
        attributes->append(AttributesFromFile(fileAttributes));
        emit attributesCounted();
        emit sendInfoMessage(QTime::currentTime().toString(),Qt::black);
        delete segy;
        return true;
    }
    else
    {
        return false;
    }
}






