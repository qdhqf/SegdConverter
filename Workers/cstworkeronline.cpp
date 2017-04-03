#include "cstworkeronline.h"
#include "Segd/segdfile.h"
#include "Cst/cstfile.h"

void CstWorkerOnline::Converting()
{
    QFileInfo fInfo(segdPath);
    QDir segdDir;
    logFile = new QFile(outPath+"_log.txt");
    logFile->open(QIODevice::Text|QIODevice::WriteOnly);
    logStream = new QTextStream(logFile);
    logStream->setCodec("UTF8");
    *logStream << QString("%1 Начало конвертации\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));

    QFile cstFile;
    cstFile.setFileName(outPath);

    if (cstFile.open(QIODevice::WriteOnly))
    {
        cstFile.close();
    }
    else {
        emit sendSomeError("Ошибка создания файла СST");
        emit finished();
    }
    if (auxMode==writeInNewFile)
    {
        cstFile.setFileName(outAuxesPath);
        if (cstFile.open(QIODevice::WriteOnly))
        {
            cstFile.close();
        }
        else
        {
            emit sendSomeError("Ошибка создания файла служебный трасс. Запись служебный трасс не производится");
            auxMode=noWrite;
            //writeAuxesNewFile = false;
        }
    }





//    if (writeAuxesNewFile)
//    {
//        cstFile.setFileName(outAuxesPath);
//        if (cstFile.open(QIODevice::WriteOnly))
//        {
//            cstFile.close();
//        }
//        else
//        {
//            emit sendSomeError("Ошибка создания файла служебный трасс. Запись служебный трасс не производится");
//            writeAuxesNewFile = false;
//        }
//    }

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


    watcher->addPath(segdDir.absolutePath());
    run = new bool;
    *run = true;
    if (segdFilesInDir.isEmpty())
    {
        return;
    }
    while (fileForConvertingNum<segdFilesInDir.count()-1 && *run)
    {
        if (convertOneFile(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath()))
        {
            fileCount++;
            *logStream << QString("%1 Выполнена конвертация файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm:ss")).arg(segdFilesInDir.value(fileForConvertingNum).fileName());
            emit sendInfoMessage("Выполнена конвертация файла "+segdFilesInDir.value(fileForConvertingNum).absoluteFilePath(),Qt::darkGreen);
        }
        else
        {
            *logStream << QString("%1 Ошибка чтения файла %2. Переход к следующему файлу ").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(segdFilesInDir.value(fileForConvertingNum).fileName());
            emit sendSomeError(QString ("Ошибка чтения файла %1. Переход к следующему файлу").arg(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath()));
        }
        if (limitMaxFiles && fileCount >= maxFilesValue)
        {
            fileCount=0;
            outPath.insert(outPath.lastIndexOf('/')+1,'_');
            outAuxesPath.insert(outAuxesPath.lastIndexOf('/')+1,'_');
        }
        segdFilesInDir = segdDir.entryInfoList(filter,QDir::Files,QDir::Name);
        fileForConvertingNum++;
    }
    if (!(*run))
    {
        *logStream << QString("%1 Завершение конвертации\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));
        delete logStream;
        logFile->close();
        delete logFile;
        emit finished();
    }
    else
    {
        int w =0;
        while (!convertOneFileOnline(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath()) && w<10)
    {
        thread()->sleep(waitingTime);
        w++;
    }
        if (w<10)
        {
            *logStream << QString("%1 Выполнена конвертация файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(segdFilesInDir.value(fileForConvertingNum).fileName());
            emit sendInfoMessage("Выполнена конвертация файла " + segdFilesInDir.value(fileForConvertingNum).absoluteFilePath(),Qt::darkGreen);
        }
        else
        {
            *logStream << QString("%1 Ошибка чтения файла %2. Переход к следующему файлу ").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(segdFilesInDir.value(fileForConvertingNum).fileName());
            emit sendSomeError("Ошибка чтения файла "+segdFilesInDir.value(fileForConvertingNum).absoluteFilePath() + " Файл пропущен");
        }
        fileForConvertingNum++;
        if (!(*run))
        {
            delete logStream;
            logFile->close();
            delete logFile;
            emit finished();
        }
        else
        {
            *run=false;
        }
    }
}


bool CstWorkerOnline::convertOneFileOnline(const QString &filePath)
{
    QString fileForWork;
    SegdFile *segd;
    CstFile *cst;
    *logStream << QString("%1 Начало обработки файла %2\n").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss")).arg(filePath);
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
            *logStream << QString("%1 Ошибка создания резервной копии. Копирование файлов не производится").arg(QDateTime::currentDateTime().toString("ddd dd.MMMM.yyyy hh:mm::ss"));
            emit sendSomeError("Ошибка копирования");
            return false;
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
        cst = new CstFile(segd,this);

        delete segd;
        if (!pp.isEmpty()) {
            cst->setReceiverCoordinats(pp);
        }
        if (!pv.isEmpty()) {
            cst->setSourceCoordinats(pv);
        }
        cst->setGeometry();
        fileAttributes.append(qMakePair(cst->getFfid(),true));
        fileAttributes.append(qMakePair(cst->getLine(),true));
        fileAttributes.append(qMakePair(cst->getPoint(),true));
        fileAttributes.append(qMakePair(cst->getEasting(),true));
        fileAttributes.append(qMakePair(cst->getNorthing(),true));
        fileAttributes.append(qMakePair(cst->getElevation(),true));
 //       emit sendSegdData(segdData);
        if (checkTests)
        {
            checkingTests(segd);
        }
        if (analysisAuxes)
        {
            chekingAuxData(segd);
        }
        switch (auxMode) {
        case writeAuxesMode::writeInNewFile:
            cst->writeAuxTraces(outAuxesPath);
            break;
        case writeAuxesMode::write:
             cst->writeAuxTraces(outPath);
            break;
        default:
            break;
        }/*
        if (writeAuxesNewFile)
        {
            cst->writeAuxTraces(outAuxesPath);
        }
        if (writeAuxes)
        {
            cst->writeAuxTraces(outPath);
        }*/
        cst->writeTraces(outPath,writeMutedChannels);
        countAttributes(cst);
        emit attributesCounted();
        delete cst;
        return true;
    }
    else
    {
        return false;
    }
}

void CstWorkerOnline::segdDirChanged(QString string)
{
       QDir segdDir(string);
       int w;
       QFileInfoList segdFilesInDir;
       QStringList filter;
       filter << "*.segd";
       segdFilesInDir = segdDir.entryInfoList(filter,QDir::Files,QDir::Name);
       *run = true;
       if (fileForConvertingNum < segdFilesInDir.count())
       {
           *logStream << QString("Обнаружен файл %1").arg(segdFilesInDir.value(fileForConvertingNum).fileName());
           emit sendInfoMessage("Обнаружен файл " + segdFilesInDir.value(fileForConvertingNum).fileName(),Qt::blue);
       }
       while (fileForConvertingNum<segdFilesInDir.count() && *run)
       {
           w = 0;
           while (!convertOneFileOnline(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath()) && w<10)
           {
               w++;
               *logStream << "Ошибка доступа к файлу. Ожидание.";
               emit sendSomeError("Ошибка доступа к файлу. Ожидание.");
               thread()->sleep(waitingTime);
           }

           if (w<10)
           {
                fileCount++;
                *logStream << QString("Выполнена конвертация файла %1").arg(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath());
                emit sendInfoMessage(QString("Выполнена конвертация файла %1").arg(segdFilesInDir.value(fileForConvertingNum).absoluteFilePath()),Qt::darkGreen);

           }
           else {
               *logStream << QString ("Превышено врямя ожидания файла %1. Переход к следующему файлу").arg(segdFilesInDir.value(fileForConvertingNum).fileName());
               emit sendSomeError(QString ("Превышено врямя ожидания файла %1. Переход к следующему файлу").arg(segdFilesInDir.value(fileForConvertingNum).fileName()));
           }
           fileForConvertingNum++;
           if (limitMaxFiles && fileCount >= maxFilesValue)
           {
               fileCount=0;
               outPath.insert(outPath.lastIndexOf('/')+1,'_');
               outAuxesPath.insert(outAuxesPath.lastIndexOf('/')+1,'_');
               createFileForMissedTraces();
           }
       }
       if (!(*run))
       {
           delete logStream;
           logFile->close();
           delete logFile;
           emit finished();
       }
       else
       {
           *run=false;
       }
}


