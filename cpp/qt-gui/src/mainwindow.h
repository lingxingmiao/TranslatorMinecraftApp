#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QDialog>
#include <QScrollArea>
#include <QFormLayout>
#include <QJsonObject>
#include <QGroupBox>

class PythonBackend;

struct TaskInfo {
    QString taskId, displayName, typeName, status;
    int progress = 0;
    QStringList logs;
    QString filename, downloadUrl;
};

// ================================================================
// LlmEndpointDialog - 新建/编辑 LLM 端点 (_build_tier 全部字段)
// ================================================================
class LlmEndpointDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LlmEndpointDialog(int index, const QJsonObject &vals = {}, QWidget *parent = nullptr);
    int index() const { return m_index; }
    QJsonObject values() const;
private:
    int m_index;
    QMap<QString, QLineEdit*> m_edits;
};

// ================================================================
// SettingsDialog - 全部 QLineEdit + tooltip 从 json 加载
// ================================================================
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    QJsonObject getConfig() const;
    void setConfig(const QJsonObject &cfg);

signals:
    void configSaved(const QJsonObject &cfg);

private slots:
    void onSave();
    void onManageEndpoints();
    void onSaveConfigFile();
    void onLoadConfigFile();
    void onDeleteConfigFile();

private:
    void addEdit(const QString &key, const QString &def = "");

    QFormLayout *m_form;
    QComboBox *m_configCombo;
    QJsonObject m_llmEndpoints;
    int m_nextLlmIdx;
    QMap<QString, QLineEdit*> m_edits;
};

// ================================================================
// TaskDialog
// ================================================================
class TaskDialog : public QDialog
{
    Q_OBJECT
public:
    enum TaskType { Translate, Separate, Merge };
    explicit TaskDialog(TaskType type, QWidget *parent = nullptr);
    QString file0Path() const;
    QString file1Path() const;
    QString notlangPath() const;
    bool allMode() const;
    bool exportInspection() const;
    QString taskName() const;
private:
    void setupUi(TaskType type);
    QString browseFile(const QString &filter);
    TaskType m_type;
    QLineEdit *m_file0Edit{}, *m_file1Edit{}, *m_notlangEdit{};
    QCheckBox *m_allModeCheck{}, *m_exportCheck{};
    QLineEdit *m_nameEdit{};
};

// ================================================================
// MainWindow
// ================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewTaskType(int type);
    void onOpenSettings();
    void onTaskListSelectionChanged();
    void onDownloadCurrentTask();
    void onCancelCurrentTask();
    void onTaskCreated(const QString &typeName, const QString &taskId);
    void onTaskStatusChanged(const QString &taskId, const QString &status, int progress, const QString &error);
    void onTaskLogReceived(const QString &taskId, const QStringList &logs);
    void onTaskCompleted(const QString &taskId, const QString &filename, const QString &downloadUrl);
    void onServerError(const QString &error);
    void onServerLog(const QString &message);
    void onConfigSaved(const QJsonObject &cfg);

private:
    void setupUi();
    void addTask(const QString &typeName, const QString &taskId);
    void updateTaskInfo(const QString &taskId, const QString &status, int progress);
    void appendTaskLog(const QString &taskId, const QString &log);
    void refreshTaskList();
    void showTaskDetail(const QString &taskId);
    void applyStyleSheet(QWidget *w);
    void setupTitleBar(QVBoxLayout *topLayout);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    PythonBackend *m_backend;
    SettingsDialog *m_settingsDlg;

    QWidget *m_titleBar;
    QLabel *m_detailLabel, *m_statusLabel;
    QProgressBar *m_progressBar;
    QTextEdit *m_logView;
    QListWidget *m_taskList;
    QPushButton *m_newTaskBtn, *m_settingsBtn, *m_cancelBtn, *m_autoScrollBtn;

    QMap<QString, TaskInfo> m_tasks;
    QString m_selectedTaskId, m_apiUrl, m_apiKey, m_savePath, m_pendingTaskName;
    int m_maxLogLines = 1500;
    int m_pollIntervalMs = 500;
    bool m_autoScroll = true;
    bool m_dragging = false;
    QPoint m_dragStart;
    QMap<QString,int> m_taskLogCounts;
    int m_counters[3]{};
    QString m_styleSheet;
};

#endif
