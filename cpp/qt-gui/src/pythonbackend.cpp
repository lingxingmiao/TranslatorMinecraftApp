#include "pythonbackend.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QMessageBox>
#include <QThread>

PythonBackend::PythonBackend(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_network(new QNetworkAccessManager(this))
    , m_pollTimer(new QTimer(this))
    , m_port(25561)
    , m_taskRunning(false)
{
    connect(m_process, &QProcess::started,
            this, &PythonBackend::onProcessStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PythonBackend::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &PythonBackend::onProcessReadyRead);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &PythonBackend::onProcessReadyRead);
    connect(m_process, &QProcess::errorOccurred,
            this, &PythonBackend::onProcessError);

    m_pollTimer->setInterval(2000); // 2秒轮询一次
    connect(m_pollTimer, &QTimer::timeout, this, [this]() {
        if (!m_currentTaskId.isEmpty()) {
            pollTaskStatus(m_currentTaskId);
            fetchTaskLogs(m_currentTaskId);
        }
    });
}

PythonBackend::~PythonBackend()
{
    stopServer();
}

void PythonBackend::startServer(quint16 port)
{
    m_port = port;

    // 查找 Python 解释器
    QString python = "python";
    QStringList args;

    // 找到 TranslatorAPI.py 的路径
    QString scriptPath = QCoreApplication::applicationDirPath() + "/TranslatorAPI.py";
    if (!QFile::exists(scriptPath)) {
        // 尝试相对路径
        scriptPath = QDir::currentPath() + "/TranslatorAPI.py";
    }
    if (!QFile::exists(scriptPath)) {
        // 尝试项目根目录
        scriptPath = QCoreApplication::applicationDirPath() + "/../../TranslatorAPI.py";
    }

    // 检查 python 是否可用
    QProcess testPython;
    testPython.start("python", {"--version"});
    testPython.waitForFinished(3000);
    if (testPython.exitCode() != 0) {
        testPython.start("python3", {"--version"});
        testPython.waitForFinished(3000);
        if (testPython.exitCode() == 0)
            python = "python3";
        else {
            emit errorOccurred("找不到 Python 解释器。请确保 Python 已安装并在 PATH 中。");
            return;
        }
    }

    args << scriptPath;

    m_process->setWorkingDirectory(QFileInfo(scriptPath).absolutePath());
    m_process->start(python, args);

    if (!m_process->waitForStarted(5000)) {
        emit errorOccurred("无法启动 Python API 服务。");
    }
}

void PythonBackend::stopServer()
{
    m_pollTimer->stop();
    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
    }
}

bool PythonBackend::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

QString PythonBackend::apiBaseUrl() const
{
    return m_apiUrl;
}

void PythonBackend::setApiConfig(const QString &apiUrl, const QString &apiKey)
{
    m_apiUrl = apiUrl;
    m_apiKey = apiKey;
}

void PythonBackend::setPollInterval(int ms)
{
    m_pollTimer->setInterval(ms);
}

// ================================================================
// 进程回调
// ================================================================

void PythonBackend::onProcessStarted()
{
    emit serverLog("Python API 服务启动中...");
    // 给服务一些初始化时间
    QThread::msleep(1500);
    emit serverStarted();
}

void PythonBackend::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    emit serverLog(QString("Python 进程已退出 (code=%1)").arg(exitCode));
    emit serverStopped();
}

void PythonBackend::onProcessReadyRead()
{
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    if (!output.trimmed().isEmpty())
        emit serverLog(output.trimmed());
    QString errOutput = QString::fromUtf8(m_process->readAllStandardError());
    if (!errOutput.trimmed().isEmpty())
        emit serverLog("[stderr] " + errOutput.trimmed());
}

void PythonBackend::onProcessError(QProcess::ProcessError error)
{
    QString msg;
    switch (error) {
    case QProcess::FailedToStart: msg = "无法启动 Python 进程"; break;
    case QProcess::Crashed:       msg = "Python 进程崩溃"; break;
    case QProcess::Timedout:      msg = "Python 进程超时"; break;
    default:                      msg = "Python 进程未知错误"; break;
    }
    emit errorOccurred(msg);
}

// ================================================================
// 任务提交
// ================================================================

void PythonBackend::submitTranslate(const QString &file0Path,
                                     const QString &file1Path,
                                     bool allMode, bool exportInspection)
{
    QFile file0(file0Path);
    if (!file0.open(QIODevice::ReadOnly)) {
        emit errorOccurred("无法读取文件: " + file0Path);
        return;
    }
    QByteArray file0Data = file0.readAll();
    file0.close();
    QString file0Name = QFileInfo(file0Path).fileName();

    QByteArray file1Data;
    QString file1Name;
    if (!file1Path.isEmpty() && QFile::exists(file1Path)) {
        QFile file1(file1Path);
        if (file1.open(QIODevice::ReadOnly)) {
            file1Data = file1.readAll();
            file1Name = QFileInfo(file1Path).fileName();
            file1.close();
        }
    }

    QJsonObject extra;
    extra["all_mode"] = allMode ? "true" : "false";
    extra["export_inspection"] = exportInspection ? "true" : "false";

    // 构建 translatorcore 配置
    QJsonObject core;
    core["LLM_API_URL"] = m_apiUrl;
    core["LLM_API_KEY"] = m_apiKey;
    extra["translatorcore"] = QString::fromUtf8(QJsonDocument(core).toJson(QJsonDocument::Compact));

    doUploadAndSubmit("翻译", "/translate", file0Data, file0Name,
                      file1Data, file1Name, extra);
}

void PythonBackend::submitSeparate(const QString &file0Path,
                                    const QString &file1Path)
{
    QFile file0(file0Path);
    if (!file0.open(QIODevice::ReadOnly)) {
        emit errorOccurred("无法读取文件: " + file0Path);
        return;
    }
    QByteArray file0Data = file0.readAll();
    file0.close();

    QByteArray file1Data;
    QString file1Name;
    if (!file1Path.isEmpty() && QFile::exists(file1Path)) {
        QFile file1(file1Path);
        if (file1.open(QIODevice::ReadOnly)) {
            file1Data = file1.readAll();
            file1Name = QFileInfo(file1Path).fileName();
            file1.close();
        }
    }

    QJsonObject extra;
    QJsonObject core;
    core["LLM_API_URL"] = m_apiUrl;
    core["LLM_API_KEY"] = m_apiKey;
    extra["translatorcore"] = QString::fromUtf8(QJsonDocument(core).toJson(QJsonDocument::Compact));

    doUploadAndSubmit("分离", "/separatelangupdate", file0Data,
                      QFileInfo(file0Path).fileName(),
                      file1Data, file1Name, extra);
}

void PythonBackend::submitMerge(const QString &file0Path,
                                 const QString &notlangPath,
                                 const QString &file1Path)
{
    QFile file0(file0Path);
    QFile notlang(notlangPath);
    if (!file0.open(QIODevice::ReadOnly) || !notlang.open(QIODevice::ReadOnly)) {
        emit errorOccurred("无法读取输入文件");
        return;
    }

    QByteArray file0Data = file0.readAll();
    QByteArray notlangData = notlang.readAll();
    file0.close(); notlang.close();

    QByteArray file1Data;
    QString file1Name;
    if (!file1Path.isEmpty() && QFile::exists(file1Path)) {
        QFile file1(file1Path);
        if (file1.open(QIODevice::ReadOnly)) {
            file1Data = file1.readAll();
            file1Name = QFileInfo(file1Path).fileName();
            file1.close();
        }
    }

    // 合并使用 multipart 上传
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    auto addFilePart = [&](const QString &name, const QByteArray &data,
                            const QString &filename) {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"%1\"; filename=\"%2\"")
                       .arg(name, filename));
        part.setHeader(QNetworkRequest::ContentTypeHeader,
                       "application/octet-stream");
        part.setBody(data);
        multiPart->append(part);
    };

    auto addTextPart = [&](const QString &name, const QString &value) {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"%1\"").arg(name));
        part.setBody(value.toUtf8());
        multiPart->append(part);
    };

    addFilePart("file0", file0Data, QFileInfo(file0Path).fileName());
    addFilePart("notlang_file", notlangData, QFileInfo(notlangPath).fileName());
    addTextPart("file_name0", QFileInfo(file0Path).fileName());
    addTextPart("nolang_file_name", QFileInfo(notlangPath).fileName());

    QJsonObject core;
    core["LLM_API_URL"] = m_apiUrl;
    core["LLM_API_KEY"] = m_apiKey;
    addTextPart("translatorcore", QJsonDocument(core).toJson(QJsonDocument::Compact));

    if (!file1Data.isEmpty()) {
        addFilePart("file1", file1Data, file1Name);
        addTextPart("file_name1", file1Name);
    }

    QNetworkRequest request(QUrl(apiBaseUrl() + "/mergelangupdate"));
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());

    QNetworkReply *reply = m_network->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("请求失败: " + reply->errorString());
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject resp = doc.object();
        QString tid = resp["task_id"].toString();
        m_currentTaskId = tid;
        m_taskRunning = true;
        m_pollTimer->start();
        emit taskCreated("合并", tid);
        emit taskStatusChanged(tid, "queued", 0, "");
    });
}

void PythonBackend::doUploadAndSubmit(const QString &typeName, const QString &endpoint,
                                       const QByteArray &file0Data,
                                       const QString &file0Name,
                                       const QByteArray &file1Data,
                                       const QString &file1Name,
                                       const QJsonObject &extraFields)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    auto addFilePart = [&](const QString &name, const QByteArray &data,
                            const QString &filename) {
        if (data.isEmpty()) return;
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"%1\"; filename=\"%2\"")
                       .arg(name, filename));
        part.setHeader(QNetworkRequest::ContentTypeHeader,
                       "application/octet-stream");
        part.setBody(data);
        multiPart->append(part);
    };

    auto addTextPart = [&](const QString &name, const QString &value) {
        QHttpPart part;
        part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"%1\"").arg(name));
        part.setBody(value.toUtf8());
        multiPart->append(part);
    };

    addFilePart("file0", file0Data, file0Name);
    addTextPart("file_name0", file0Name);

    if (!file1Data.isEmpty()) {
        addFilePart("file1", file1Data, file1Name);
        addTextPart("file_name1", file1Name);
    }

    // 附加 JSON 字段
    for (auto it = extraFields.begin(); it != extraFields.end(); ++it) {
        if (it.value().isString())
            addTextPart(it.key(), it.value().toString());
        else
            addTextPart(it.key(),
                        QJsonDocument(QJsonObject{{it.key(), it.value()}})
                        .toJson(QJsonDocument::Compact));
    }

    QNetworkRequest request(QUrl(apiBaseUrl() + endpoint));
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());
    request.setTransferTimeout(15000); // 15秒超时

    QNetworkReply *reply = m_network->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, typeName]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("请求失败: " + reply->errorString());
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject resp = doc.object();
        if (resp.contains("task_id")) {
            QString tid = resp["task_id"].toString();
            m_currentTaskId = tid;
            m_taskRunning = true;
            m_pollTimer->start();
            emit taskCreated(typeName, tid);
            emit taskStatusChanged(tid, "queued", 0, "");
        } else {
            emit errorOccurred("服务器返回异常: " +
                               QString::fromUtf8(reply->readAll()));
        }
    });
}

// ================================================================
// 任务轮询
// ================================================================

void PythonBackend::pollTaskStatus(const QString &taskId)
{
    QNetworkRequest request(QUrl(apiBaseUrl() + "/task/status/" + taskId));
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        QString status = obj["status"].toString();
        int progress = obj["progress"].toInt();
        QString error = obj["error"].toString();

        emit taskStatusChanged(taskId, status, progress, error);

        if (status == "completed") {
            m_pollTimer->stop();
            m_taskRunning = false;
            QString filename = obj["filename"].toString();
            QString downloadUrl = apiBaseUrl() + "/task/download/" + taskId;
            emit taskCompleted(taskId, filename, downloadUrl);
        } else if (status == "failed") {
            m_pollTimer->stop();
            m_taskRunning = false;
        }
    });
}

void PythonBackend::fetchTaskLogs(const QString &taskId)
{
    QNetworkRequest request(QUrl(apiBaseUrl() + "/task/logs/" + taskId + "/json"));
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();
        QJsonArray logs = obj["logs"].toArray();
        QStringList logList;
        for (const auto &log : logs)
            logList << log.toString();
        emit taskLogReceived(obj["task_id"].toString(), logList);
    });
}
