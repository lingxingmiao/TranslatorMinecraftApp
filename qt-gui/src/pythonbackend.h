#ifndef PYTHONBACKEND_H
#define PYTHONBACKEND_H

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

/**
 * PythonBackend - 管理 Python 翻译 API 服务进程
 *
 * 职责：
 * 1. 启动/停止 Python TranslatorAPI.py 子进程
 * 2. 通过 HTTP 与 Python API 通信
 * 3. 轮询任务状态
 */
class PythonBackend : public QObject
{
    Q_OBJECT

public:
    explicit PythonBackend(QObject *parent = nullptr);
    ~PythonBackend();

    /// 启动 Python API 服务
    void startServer(quint16 port = 25561);
    /// 停止服务
    void stopServer();
    /// 是否正在运行
    bool isRunning() const;

    /// 提交翻译任务
    void submitTranslate(const QString &file0Path, const QString &file1Path,
                         bool allMode, bool exportInspection);
    /// 提交分离任务
    void submitSeparate(const QString &file0Path, const QString &file1Path);
    /// 提交合并任务
    void submitMerge(const QString &file0Path, const QString &notlangPath,
                     const QString &file1Path);

    /// 设置 API 地址和密钥
    void setApiConfig(const QString &apiUrl, const QString &apiKey);
    /// 设置轮询间隔（毫秒）
    void setPollInterval(int ms);

    /// 获取 NetworkManager (供 MainWindow 下载文件)
    QNetworkAccessManager* networkManager() const { return m_network; }
    /// 获取服务端口
    quint16 port() const { return m_port; }
    /// 获取 API 基础 URL
    QString apiBaseUrl() const;

signals:
    /// 服务已启动
    void serverStarted();
    /// 服务已停止
    void serverStopped();
    /// 服务输出日志
    void serverLog(const QString &message);
    /// 新任务已创建
    void taskCreated(const QString &typeName, const QString &taskId);
    /// 任务状态更新
    void taskStatusChanged(const QString &taskId, const QString &status,
                           int progress, const QString &error);
    /// 任务日志更新
    void taskLogReceived(const QString &taskId, const QStringList &logs);
    /// 翻译完成，可下载
    void taskCompleted(const QString &taskId, const QString &filename,
                       const QString &downloadUrl);
    /// 错误
    void errorOccurred(const QString &error);

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessReadyRead();
    void onProcessError(QProcess::ProcessError error);

private:
    void pollTaskStatus(const QString &taskId);
    void fetchTaskLogs(const QString &taskId);
    void doUploadAndSubmit(const QString &typeName, const QString &endpoint,
                           const QByteArray &file0Data, const QString &file0Name,
                           const QByteArray &file1Data, const QString &file1Name,
                           const QJsonObject &extraFields);

    QProcess *m_process;
    QNetworkAccessManager *m_network;
    QTimer *m_pollTimer;
    quint16 m_port;
    QString m_apiUrl;
    QString m_apiKey;
    QString m_currentTaskId;
    bool m_taskRunning;
};

#endif // PYTHONBACKEND_H
