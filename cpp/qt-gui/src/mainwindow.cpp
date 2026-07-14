#include "mainwindow.h"
#include "pythonbackend.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QComboBox>
#include <QRegularExpression>
#include <QCheckBox>
#include <QSpinBox>
#include <QInputDialog>
#include <QDateTime>
#include <QSettings>
#include <QCoreApplication>

static const char* APP_QSS = R"(
/* === Iris Mod 25% 透明度配色 === */
QMainWindow { background: rgba(255,255,255,50); }

/* -- 分组框 -- */
QGroupBox {
    font-weight: bold;
    border: 2px solid rgba(224,224,224,137);
    margin-top: 12px;
    padding-top: 16px;
    background: rgba(255,255,255,40);
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    color: #1a1a1a;
}

/* -- 按钮 default：最外层RGB224 A54% + 填充RGB35 A21% -- */
QPushButton {
    background: rgba(255,255,255,60);
    color: #1a1a1a;
    border: 2px solid rgba(180,180,180,150);
    padding: 7px 18px;
    font-weight: bold;
}
/* -- 按钮 selected/hover -- */
QPushButton:hover {
    background: rgba(255,255,255,160);
    border: 2px solid rgba(255,255,255,255);
    color: #1a1a1a;
}
QPushButton:pressed {
    background: rgba(200,200,200,180);
    border: 2px solid rgba(150,150,150,200);
}
QPushButton:disabled {
    background: rgba(200,200,200,60);
    border-color: rgba(150,150,150,80);
    color: #888888;
}

/* -- A框（外层容器） -- */
QFrame[inputOuter="true"] {
    border: 2px solid rgba(224,224,224,137);
    padding: 0;
    background: rgba(255,255,255,50);
}
QFrame[inputOuter="true"] QLabel { color: #1a1a1a; }

/* -- B框（输入框内层包裹） -- */
QFrame[inputWrapper="true"] {
    border: 2px solid rgba(179,179,179,137);
    padding: 0;
    background: transparent;
}

/* -- QLineEdit 在 B框内 -- */
QFrame[inputWrapper="true"] QLineEdit {
    border: none;
    padding: 5px 8px;
    background: rgba(255,255,255,220);
    color: #1a1a1a;
    selection-background-color: #557799;
}
QFrame[inputWrapper="true"] QLineEdit:focus { background: rgba(255,255,255,240); }

/* 独立 QLineEdit 回退 */
QLineEdit {
    border: 2px solid rgba(179,179,179,137);
    padding: 6px 10px;
    background: rgba(255,255,255,220);
    color: #1a1a1a;
    selection-background-color: #557799;
}
QLineEdit:focus { border-color: rgba(255,255,255,255); }

/* -- 日志输出：深色 -- */
QTextEdit {
    border: 2px solid rgba(179,179,179,137);
    background: #1e1f29;
    color: #abb2bf;
    font-family: Consolas, 'Courier New', monospace;
    selection-background-color: #3a4a6a;
}
QTextEdit > QWidget {
    background: #1e1f29;
}

/* -- 进度条 -- */
QProgressBar {
    border: 2px solid rgba(179,179,179,137);
    background: rgba(255,255,255,80);
    text-align: center;
    font-weight: bold;
    color: #1a1a1a;
}
QProgressBar::chunk { background: rgba(163,163,163,99); }

/* -- 任务列表 -- */
QListWidget {
    border: 2px solid rgba(224,224,224,137);
    background: rgba(255,255,255,60);
}
QListWidget::item {
    padding: 8px;
    border-bottom: 1px solid rgba(224,224,224,60);
    color: #1a1a1a;
}
QListWidget::item:selected {
    background: rgba(163,163,163,120);
    color: #1a1a1a;
}
QListWidget::item:hover { background: rgba(163,163,163,60); }

/* -- 滚动区域 -- */
QScrollArea { border: none; }

/* -- 下拉框 -- */
QComboBox {
    border: 2px solid rgba(179,179,179,137);
    padding: 6px;
    background: rgba(255,255,255,180);
    color: #1a1a1a;
}
QComboBox:hover { border-color: rgba(255,255,255,255); }
QComboBox QAbstractItemView {
    background: rgba(255,255,255,220);
    border: 2px solid rgba(224,224,224,137);
    color: #1a1a1a;
    selection-background-color: rgba(163,163,163,99);
}

/* -- 对话框 -- */
QDialog { background: #ffffff; }

/* -- 标签 -- */
QLabel { color: #2a2d34; }

/* -- 复选框 -- */
QCheckBox { spacing: 4px; color: #1a1a1a; }
QCheckBox::indicator {
    width: 16px; height: 16px;
    border: 2px solid rgba(180,180,180,180);
    background: rgba(255,255,255,100);
}
QCheckBox::indicator:checked {
    background: rgba(100,100,100,150);
    border-color: rgba(255,255,255,255);
}
)";

// 语言文件加载 (Minecraft 1.16+ JSON 格式)
static QMap<QString,QString> g_lang;
static QMap<QString,QString> g_tooltips;
static void loadLangJson(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QString k = it.key();
        if (k.endsWith(".tooltip"))
            g_tooltips[k.chopped(8)] = it.value().toString(); // remove ".tooltip"
        else
            g_lang[k] = it.value().toString();
    }
}

// ===== LlmEndpointDialog =====
LlmEndpointDialog::LlmEndpointDialog(int idx, const QJsonObject &vals, QWidget *parent)
    : QDialog(parent), m_index(idx)
{
    setWindowOpacity(0.75);
    setWindowTitle(QString(g_lang.value("endpoint.title","LLM%1 端点配置")).arg(idx));
    setMinimumWidth(550); setStyleSheet(APP_QSS);
    auto *lay = new QFormLayout(this);
    auto add = [&](const QString &lkey, const QString &key, const QString &def = "") {
        auto *outer = new QFrame(this); outer->setProperty("inputOuter", true);  // A框
        auto *hl = new QHBoxLayout(outer); hl->setContentsMargins(4,2,4,2); hl->setSpacing(4);
        auto *label = new QLabel(g_lang.value(lkey, key) + ":", outer); label->setMinimumWidth(160);
        hl->addWidget(label);
        auto *wrapper = new QFrame(outer); wrapper->setProperty("inputWrapper", true); // B框
        auto *wl = new QHBoxLayout(wrapper); wl->setContentsMargins(0,0,0,0);
        auto *e = new QLineEdit(vals.value(key).toString(def.isEmpty()?"":def), wrapper); e->setMinimumWidth(250);
        wl->addWidget(e);
        hl->addWidget(wrapper, 1);
        auto *cb = new QCheckBox(outer); cb->setChecked(true);
        hl->addWidget(cb);
        m_edits[key] = e; lay->addRow(outer);
    };
    add("ep.url","API_URL"); add("ep.key","API_KEY"); add("ep.model","MODEL");
    add("ep.api_kwargs","API_KWARGS","{}");
    add("ep.temperature","TEMPERATURE","0.30"); add("ep.top_p","TOP_P","0.95");
    add("ep.top_k","TOP_K","30");
    add("ep.rp","REPETITION_PENALTY","1.1"); add("ep.pp","PRESENCE_PENALTY","0");
    add("ep.fp","FREQUENCY_PENALTY","0"); add("ep.seed","SEED","42");
    add("ep.max_retry","MAX_RETRY","8"); add("ep.conn_timeout","CONN_TIMEOUT","3");
    add("ep.timeout","TIMEOUT","300"); add("ep.retry_time","RETRY_TIME","5");
    add("ep.retry_coef","RETRY_COEF","1.2"); add("ep.max_workers","MAX_WORKERS","8");
    add("ep.min_count","MIN_COUNT","0"); add("ep.weight","WEIGHT","1.0");
    add("ep.active_start","ACTIVE_TIME_START"); add("ep.active_end","ACTIVE_TIME_END");
    auto *btnBox = new QHBoxLayout();
    auto *ok = new QPushButton(g_lang.value("btn.ok","确定")); connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    auto *cancel = new QPushButton(g_lang.value("btn.cancel","取消")); connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    btnBox->addStretch(); btnBox->addWidget(ok); btnBox->addWidget(cancel);
    lay->addRow(btnBox);
}

QJsonObject LlmEndpointDialog::values() const {
    QJsonObject o;
    for (auto it=m_edits.begin(); it!=m_edits.end(); ++it) {
        auto *inner = it.value()->parentWidget();               // QFrame(B框)
        auto *outer = inner ? inner->parentWidget() : nullptr;  // QFrame(A框)
        if (outer) { auto *cb = outer->findChild<QCheckBox*>(); if (cb && !cb->isChecked()) continue; }
        if (!it.value()->text().isEmpty()) o[it.key()] = it.value()->text();
    }
    return o;
}

// ===== SettingsDialog =====
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), m_nextLlmIdx(0)
{
    setWindowOpacity(0.75);
    setWindowTitle(g_lang.value("settings.title","设置")); setMinimumSize(760, 620); setStyleSheet(APP_QSS);
    auto *outer = new QVBoxLayout(this);
    auto *scroll = new QScrollArea(this); scroll->setWidgetResizable(true);
    auto *container = new QWidget(); m_form = new QFormLayout(container);
    m_form->setLabelAlignment(Qt::AlignRight); scroll->setWidget(container);
    outer->addWidget(scroll, 1);

    addEdit("LANGUAGE_INPUT", "en_us"); addEdit("LANGUAGE_OUTPUT", "zh_cn");
    addEdit("LLM_API_URL"); addEdit("LLM_API_KEY"); addEdit("LLM_API_KWARGS", "{}"); addEdit("LLM_MODEL");
    addEdit("LLM_TOP_K", "30"); addEdit("LLM_TOP_P", "0.95"); addEdit("LLM_TEMP", "0.30");
    addEdit("LLM_RP", "1.1"); addEdit("LLM_PP", "0"); addEdit("LLM_FP", "0"); addEdit("LLM_SEED", "42");
    addEdit("LLM_MAX_WORKERS", "24"); addEdit("LLM_MIN_COUNT", "0");
    addEdit("LLM_ACTIVE_TIME_START"); addEdit("LLM_ACTIVE_TIME_END");
    addEdit("LLM_TIER_INTERLEAVE", "false"); addEdit("LLM_TIER_DYNAMIC", "false");
    addEdit("LLM_TIER_CASCADE", "false"); addEdit("LLM_TIER_CASCADE_RATIO", "0.7");
    addEdit("LLM_TIER_MULTI_OVERLAP", "false"); addEdit("LLM_TIER_MULTI_WEIGHT", "1.0");
    addEdit("LLM_TIER_OVERLAP", "false"); addEdit("LLM_TIER_OVERLAP_RATIO", "[0.3, 0.7]");
    addEdit("LLM_CONTEXTS", "false"); addEdit("LLM_MAX_RETRY", "8"); addEdit("LLM_TIMEOUT", "300");
    addEdit("LLM_CONN_TIMEOUT", "3"); addEdit("LLM_RETRY_TIME", "5"); addEdit("LLM_RETRY_COEF", "1.2");
    addEdit("LLM_TOKEN_USAGE", "0");
    addEdit("EMB_API_URL"); addEdit("EMB_API_KEY"); addEdit("EMB_MODEL", "LiquidAI/LFM2.5-Embedding-350M");
    addEdit("EMB_MODEL_ACC_MODE", "bfloat16"); addEdit("EMB_MODEL_DEVICE", "cpu");
    addEdit("EMB_ENCODE_PROMPT_NAME"); addEdit("EMB_ENCODE_NORMALIZE", "true");
    addEdit("EMB_MAX_TOKENS", "512"); addEdit("EMB_TOKENSTOTEXT_RATIO", "3.0");
    addEdit("EMB_MAX_WORKERS", "24"); addEdit("EMB_MAX_RETRY", "8"); addEdit("EMB_TIMEOUT", "90");
    addEdit("EMB_CONN_TIMEOUT", "3"); addEdit("EMB_RETRY_TIME", "5"); addEdit("EMB_RETRY_COEF", "1.2");
    addEdit("RERANKER_API_URL"); addEdit("RERANKER_API_KEY");
    addEdit("RERANKER_MODEL", "Qwen/Qwen3-Reranker-0.6B"); addEdit("RERANKER_MODEL_DEVICE", "cpu");
    addEdit("RERANKER_INSTRUCT", "Which Chinese translation best matches...");
    addEdit("RERANKER_MAX_WORKERS", "24"); addEdit("RERANKER_MAX_RETRY", "8");
    addEdit("RERANKER_TIMEOUT", "300"); addEdit("RERANKER_CONN_TIMEOUT", "3");
    addEdit("RERANKER_RETRY_TIME", "5"); addEdit("RERANKER_RETRY_COEF", "1.2");
    addEdit("VEC_INT_DTYPE", "[list]"); addEdit("VEC_FLOAT_DTYPE", "[list]");
    addEdit("VEC_FILE_PATH", "./Vectors"); addEdit("VEC_FILE_NAME", "Vectors");
    addEdit("VEC_READ_CACHE", "false"); addEdit("VEC_CACHE_PATH", "./Vectors");
    addEdit("VEC_CACHE_NAME", "VectorsCache"); addEdit("VEC_CACHE_DECAY_GRACE", "256");
    addEdit("VEC_CACHE_DECAY_THRESHOLD", "0.05"); addEdit("VEC_CACHE_MAX_SIZE", "128000");
    addEdit("VEC_QUANTIZATION", "GSQ4_K"); addEdit("VEC_QUANTILE", "0.998");
    addEdit("VEC_QUANTIZATION_ITRS_SVD", "50"); addEdit("VEC_QUANTIZATION_SPL_SVD", "10000000");
    addEdit("VEC_QUANTIZATION_ITRS_LM", "200"); addEdit("VEC_QUANTIZATION_SPL_LM", "10000000");
    addEdit("VEC_QUANTIZATION_ES_LM", "1e-6"); addEdit("VEC_QUANTIZATION_BLOCK_SIZE", "32");
    addEdit("VEC_QUANTIZATION_SCALE_TYPE", "Float16_E0M15");
    addEdit("VEC_RERANKER", "true"); addEdit("VEC_RERANKER_INDEX_RERANKER_BLOCK_SIZE", "128");
    addEdit("VEC_RERANKER_INDEX_FACTOR", "4"); addEdit("VEC_RERANKER_INDEX_BASE_MODE", "IP");
    addEdit("VEC_RERANKER_INDEX_MODE", "IVF"); addEdit("VEC_RERANKER_INDEX_BASE_SQ", "Q8");
    addEdit("VEC_RERANKER_INDEX_SQ", "Q8"); addEdit("VEC_RERANKER_INDEX_SAMPLING", "0.05");
    addEdit("VEC_RERANKER_INDEX_SAMPLING_MIN", "1");
    addEdit("VEC_RERANKER_INDEX_RE_MINMAX", "false"); addEdit("VEC_RERANKER_INDEX_RE_MEANSTD", "false");
    addEdit("VEC_RERANKER_INDEX_RE_QUANTILES", "false"); addEdit("VEC_RERANKER_INDEX_RE_OPTIM", "false");
    addEdit("VEC_RERANKER_INDEX_HNSW_M", "128"); addEdit("VEC_RERANKER_INDEX_HNSW_CONSTRUCTION", "720");
    addEdit("VEC_RERANKER_INDEX_HNSW_SEARCH", "480"); addEdit("VEC_RERANKER_INDEX_HNSW_NBITS", "8");
    addEdit("VEC_RERANKER_INDEX_HNSW_PQ_M", "8"); addEdit("VEC_RERANKER_INDEX_IVF_NPROBE", "32");
    addEdit("VEC_RERANKER_INDEX_IVF_NLITS", "8"); addEdit("VEC_RERANKER_INDEX_IVF_NLIST", "512");
    addEdit("VEC_RERANKER_INDEX_IVF_PQ_M", "8"); addEdit("VEC_RERANKER_INDEX_IVF_RQ", "true");
    addEdit("VEC_RERANKER_INDEX_REFINEFLAT_K_FACTOR", "6.0");
    addEdit("TRANSLATOR_CACHE_WRITE", "true"); addEdit("TRANSLATOR_CACHE_READ", "true");
    addEdit("TRANSLATOR_CACHE_PATH", "./Translator_Cache"); addEdit("TRANSLATOR_CACHE_NAME", "Translator_Cache");
    addEdit("TRANSLATOR_REFINE_ROUNDS", "0"); addEdit("TRANSLATOR_BATCH", "3");
    addEdit("TRANSLATOR_BATCH_RETRY", "2"); addEdit("TRANSLATOR_ORIGINAL_REFERENCE", "false");
    addEdit("TRANSLATOR_MODPACK_MOD_CONCURRENT", "8");
    addEdit("TRANSLATOR_USER_PROMPT", "[prompt list]"); addEdit("TRANSLATOR_SYSTEM_PROMPT", "[system prompt]");
    addEdit("PATH_CACHE", "./Cache"); addEdit("CACHE_CHECK_INTERVAL", "24"); addEdit("CACHE_TTL_HOURS", "48");
    addEdit("DEBUG_MODE", "false"); addEdit("LOGS_FILE_PATH", "./Logs"); addEdit("LOGS_FILE_NAME", "logs");
    addEdit("LOGS_GLOBAL", "false"); addEdit("LOGS_FLUSH_INTERVAL", "3");
    addEdit("LANG_PATH", "./Lang"); addEdit("LANGUAGE", "zh_CN"); addEdit("TQDM_FPS", "2");
    addEdit("QUESTS_READ_MAX_CONCURRENT", "4"); addEdit("QUESTS_WRITE_MAX_CONCURRENT", "4");
    addEdit("SCRIPT_READ_MAX_CONCURRENT", "4"); addEdit("SCRIPT_WRITE_MAX_CONCURRENT", "4");
    addEdit("MENU_READ_MAX_CONCURRENT", "4"); addEdit("MENU_WRITE_MAX_CONCURRENT", "4");
    addEdit("BOOK_READ_MAX_CONCURRENT", "4"); addEdit("BOOK_WRITE_MAX_CONCURRENT", "4");
    addEdit("DATA_READ_MAX_CONCURRENT", "4"); addEdit("DATA_WRITE_MAX_CONCURRENT", "4");
    addEdit("LANG_READ_MAX_CONCURRENT", "4"); addEdit("LANG_WRITE_MAX_CONCURRENT", "4");
    addEdit("DLL_READ_MAX_CONCURRENT", "4"); addEdit("DLL_WRITE_MAX_CONCURRENT", "4");
    addEdit("SCRIPT_CRT_WRITE_UNICODE", "true");
    addEdit("MONO_CECIL_DLL_PATH", "./dll"); addEdit("MONO_CECIL_DLL_NAME", "Mono.Cecil.dll");
    addEdit("DATA_COMMAND_PATH", "./DataPack_Command"); addEdit("DATA_COMMAND_FILE", "DataPack_Command.txt");
    addEdit("PACK_META_TEMPLATE_TRANSLATE", "{name} {lang} by {author}, model: {model}");
    addEdit("PACK_META_TEMPLATE_MERGE", "{name} {lang} by {author}, auto merged");
    addEdit("PACK_META_TEMPLATE_CASUALTIESUNKNOWN", "{lang} by {author}, model: {model}");
    addEdit("PACK_AUTHOR");
    addEdit("INDEX_TEXT_K", "2"); addEdit("INDEX_WORD_K", "2"); addEdit("INDEX_LANG_K", "2");
    addEdit("INDEX_QUESTS_BASIC_WORDS", "[]");
    addEdit("INDEX_BASE_MODE", "HNSWPQ"); addEdit("INDEX_MODE", "Refine"); addEdit("INDEX_LANG_MODE", "IP");
    addEdit("INDEX_BASE_SQ", "Q8"); addEdit("INDEX_SQ", "Q8"); addEdit("INDEX_SAMPLING", "0.05");
    addEdit("INDEX_SAMPLING_MIN", "1");
    addEdit("INDEX_RE_MINMAX", "false"); addEdit("INDEX_RE_MEANSTD", "false");
    addEdit("INDEX_RE_QUANTILES", "false"); addEdit("INDEX_RE_OPTIM", "false");
    addEdit("INDEX_HNSW_M", "128"); addEdit("INDEX_HNSW_CONSTRUCTION", "720");
    addEdit("INDEX_HNSW_SEARCH", "480"); addEdit("INDEX_HNSW_NBITS", "8"); addEdit("INDEX_HNSW_PQ_M", "8");
    addEdit("INDEX_IVF_NPROBE", "32"); addEdit("INDEX_IVF_NLITS", "8"); addEdit("INDEX_IVF_NLIST", "512");
    addEdit("INDEX_IVF_PQ_M", "8"); addEdit("INDEX_IVF_RQ", "true"); addEdit("INDEX_REFINEFLAT_K_FACTOR", "6.0");
    addEdit("INDEX_GSQ_RERANKER_BLOCK_SIZE", "128"); addEdit("INDEX_GSQ_RERANKER_FACTOR", "4");
    addEdit("INDEX_GSQ_BLOCK_SIZE", "128"); addEdit("INDEX_GSQ_MOE_BLOCK_SIZE", "64");
    addEdit("INDEX_GSQ_MOE_EXP", "0.01"); addEdit("INDEX_GSQ_MOE_SPL_SVD", "5");
    addEdit("INDEX_GSQ_MOE_SPL_LM", "20"); addEdit("INDEX_GSQ_MOE_KM_LM", "16");
    addEdit("INDEX_CPU_COUNT", "0.8");

    auto *btnRow = new QHBoxLayout();
    auto *epBtn = new QPushButton(g_lang.value("btn.manage_endpoints","管理 LLM 端点...")); connect(epBtn, &QPushButton::clicked, this, &SettingsDialog::onManageEndpoints);
    btnRow->addWidget(epBtn); btnRow->addStretch();
    m_configCombo = new QComboBox(this); m_configCombo->setEditable(true); m_configCombo->setMinimumWidth(120);
    btnRow->addWidget(new QLabel(g_lang.value("label.config","配置:"), this)); btnRow->addWidget(m_configCombo);
    auto *loadCfg = new QPushButton(g_lang.value("btn.load","加载")); connect(loadCfg, &QPushButton::clicked, this, &SettingsDialog::onLoadConfigFile);
    auto *saveCfg = new QPushButton(g_lang.value("btn.save","保存")); connect(saveCfg, &QPushButton::clicked, this, &SettingsDialog::onSaveConfigFile);
    auto *delCfg = new QPushButton(g_lang.value("btn.delete","删除")); connect(delCfg, &QPushButton::clicked, this, &SettingsDialog::onDeleteConfigFile);
    btnRow->addWidget(loadCfg); btnRow->addWidget(saveCfg); btnRow->addWidget(delCfg);
    auto *saveBtn = new QPushButton(g_lang.value("btn.save_settings","保存设置"));
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    btnRow->addWidget(saveBtn);
    outer->addLayout(btnRow);

    QFile f("config.json");
    if (f.open(QIODevice::ReadOnly)) {
        auto doc = QJsonDocument::fromJson(f.readAll());
        for (const auto &k : doc.object().keys()) m_configCombo->addItem(k);
    }
}

void SettingsDialog::addEdit(const QString &key, const QString &def) {
    auto *outer = new QFrame(this); outer->setProperty("inputOuter", true);  // A框
    auto *hl = new QHBoxLayout(outer); hl->setContentsMargins(4,2,4,2); hl->setSpacing(4);
    auto *label = new QLabel(key + ":", outer); label->setMinimumWidth(220);
    hl->addWidget(label);
    auto *wrapper = new QFrame(outer); wrapper->setProperty("inputWrapper", true); // B框
    auto *wl = new QHBoxLayout(wrapper); wl->setContentsMargins(0,0,0,0);
    auto *e = new QLineEdit(def, wrapper); e->setMinimumWidth(300);
    wl->addWidget(e);
    hl->addWidget(wrapper, 1);
    auto *cb = new QCheckBox(outer); cb->setChecked(false);
    hl->addWidget(cb);
    m_edits[key] = e;
    if (g_tooltips.contains(key)) { e->setToolTip(g_tooltips[key]); cb->setToolTip(g_tooltips[key]); }
    m_form->addRow(outer); // 占满整行
}

QJsonObject SettingsDialog::getConfig() const {
    QJsonObject cfg = m_llmEndpoints;
    for (auto it=m_edits.begin(); it!=m_edits.end(); ++it) {
        auto *inner = it.value()->parentWidget();               // QFrame(B框)
        auto *outer = inner ? inner->parentWidget() : nullptr;  // QFrame(A框)
        if (outer) { auto *cb = outer->findChild<QCheckBox*>(); if (cb && !cb->isChecked()) continue; }
        cfg[it.key()] = it.value()->text();
    }
    return cfg;
}

void SettingsDialog::setConfig(const QJsonObject &cfg) {
    for (auto it=cfg.begin(); it!=cfg.end(); ++it) {
        QString k = it.key();
        QRegularExpression re("^LLM(\\d+)_"); auto m = re.match(k);
        if (m.hasMatch()) { m_llmEndpoints[k]=it.value(); int idx=m.captured(1).toInt(); if (idx>=m_nextLlmIdx) m_nextLlmIdx=idx+1; continue; }
        if (m_edits.contains(k)) {
            m_edits[k]->setText(it.value().toString());
            auto *inner = m_edits[k]->parentWidget();               // QFrame(B框)
            auto *outer = inner ? inner->parentWidget() : nullptr;  // QFrame(A框)
            if (outer) { auto *cb = outer->findChild<QCheckBox*>(); if (cb) cb->setChecked(true); }
        }
    }
}

void SettingsDialog::onSave() { emit configSaved(getConfig()); accept(); }

void SettingsDialog::onManageEndpoints() {
    QDialog dlg(this); dlg.setWindowTitle(g_lang.value("endpoint_mgr.title","LLM 端点管理")); dlg.setMinimumWidth(500); dlg.setWindowOpacity(0.75); dlg.setStyleSheet(APP_QSS);
    auto *lay = new QVBoxLayout(&dlg); auto *list = new QListWidget(&dlg); lay->addWidget(list);
    QMap<int,QJsonObject> eps;
    for (auto it=m_llmEndpoints.begin(); it!=m_llmEndpoints.end(); ++it) {
        QString k=it.key(); QRegularExpression re("^LLM(\\d+)_"); auto m=re.match(k);
        if (m.hasMatch() && k.endsWith("_API_URL")) { int idx=m.captured(1).toInt(); eps[idx][k.mid(k.lastIndexOf('_')+1)]=it.value(); }
    }
    auto refresh=[&](){ list->clear(); for (auto it=eps.begin(); it!=eps.end(); ++it) { auto *item=new QListWidgetItem(QString("LLM%1 - %2").arg(it.key()).arg(it.value()["API_URL"].toString())); item->setFlags(item->flags()|Qt::ItemIsUserCheckable); item->setCheckState(Qt::Checked); item->setData(Qt::UserRole,it.key()); list->addItem(item); } };
    refresh();
    auto *br=new QHBoxLayout();
    auto *addB=new QPushButton(g_lang.value("btn.new_endpoint","+ 新建")); auto *editB=new QPushButton(g_lang.value("btn.edit","编辑")); auto *delB=new QPushButton(g_lang.value("btn.delete","删除"));
    br->addWidget(addB); br->addWidget(editB); br->addWidget(delB);
    auto *closeB=new QPushButton(g_lang.value("btn.close","关闭")); connect(closeB,&QPushButton::clicked,&dlg,&QDialog::accept); br->addStretch(); br->addWidget(closeB);
    lay->addLayout(br);
    connect(addB,&QPushButton::clicked,[&](){ int idx=m_nextLlmIdx++; LlmEndpointDialog d(idx,{},&dlg); if(d.exec()==QDialog::Accepted){ auto v=d.values(); eps[idx]=v; for(auto it2=v.begin();it2!=v.end();++it2) m_llmEndpoints[QString("LLM%1_%2").arg(idx).arg(it2.key())]=it2.value(); refresh(); } });
    connect(editB,&QPushButton::clicked,[&](){ auto *item=list->currentItem(); if(!item)return; int idx=item->data(Qt::UserRole).toInt(); LlmEndpointDialog d(idx,eps[idx],&dlg); if(d.exec()==QDialog::Accepted){ auto v=d.values(); eps[idx]=v; for(auto it2=v.begin();it2!=v.end();++it2) m_llmEndpoints[QString("LLM%1_%2").arg(idx).arg(it2.key())]=it2.value(); refresh(); } });
    connect(delB,&QPushButton::clicked,[&](){ auto *item=list->currentItem(); if(!item)return; int idx=item->data(Qt::UserRole).toInt(); eps.remove(idx); for(const auto &k:m_llmEndpoints.keys()) if(k.startsWith(QString("LLM%1_").arg(idx))) m_llmEndpoints.remove(k); refresh(); });
    dlg.exec();
}

void SettingsDialog::onSaveConfigFile() {
    QString n=m_configCombo->currentText().trimmed(); if(n.isEmpty())return;
    QJsonObject all; QFile f("config.json"); if(f.open(QIODevice::ReadOnly)) all=QJsonDocument::fromJson(f.readAll()).object(); f.close();
    all[n]=getConfig(); if(f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(all).toJson());
    if(m_configCombo->findText(n)<0) m_configCombo->addItem(n);
}
void SettingsDialog::onLoadConfigFile() {
    QString n=m_configCombo->currentText().trimmed(); if(n.isEmpty())return;
    QFile f("config.json"); if(!f.open(QIODevice::ReadOnly)) return;
    QJsonObject all=QJsonDocument::fromJson(f.readAll()).object();
    if(all.contains(n)) setConfig(all[n].toObject());
}
void SettingsDialog::onDeleteConfigFile() {
    QString n=m_configCombo->currentText().trimmed();
    QFile f("config.json"); if(!f.open(QIODevice::ReadOnly)) return;
    QJsonObject all=QJsonDocument::fromJson(f.readAll()).object(); all.remove(n); f.close();
    if(f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(all).toJson());
    int idx=m_configCombo->findText(n); if(idx>=0) m_configCombo->removeItem(idx);
}

// ===== TaskDialog =====
TaskDialog::TaskDialog(TaskType type, QWidget *parent) : QDialog(parent), m_type(type) {
    setWindowOpacity(0.75);
    setWindowTitle(g_lang.value("new_task.title","新建任务")); setMinimumSize(550,320); setStyleSheet(APP_QSS); setupUi(type);
}
void TaskDialog::setupUi(TaskType type) {
    auto *lay=new QVBoxLayout(this);
    lay->addWidget(new QLabel("任务名称:",this));
    m_nameEdit=new QLineEdit(QString("任务_%1").arg(QDateTime::currentDateTime().toString("MMddhhmm")),this);
    lay->addWidget(m_nameEdit);
    auto addFR=[&](const QString &lkey, QLineEdit *&edit, const QString &filter){
        auto *box=new QGroupBox(g_lang.value(lkey,lkey),this); auto *bl=new QHBoxLayout(box);
        edit=new QLineEdit(this); edit->setReadOnly(true);
        auto *btn=new QPushButton(g_lang.value("btn.browse","浏览..."),this);
        connect(btn,&QPushButton::clicked,[this,edit,filter](){ QString p=browseFile(filter); if(!p.isEmpty()) edit->setText(p); });
        bl->addWidget(edit); bl->addWidget(btn); lay->addWidget(box);
    };
    if(type==Translate){ lay->addWidget(new QLabel(g_lang.value("task.translate","翻译文件"),this)); addFR("task.main_file",m_file0Edit,"所有支持格式 (*.jar *.zip *.mrpack *.lang *.json *.zs *.dll);;所有文件 (*)"); addFR("task.ref_file",m_file1Edit,"所有格式 (*.lang *.json *.jar *.zip);;所有文件 (*)"); m_allModeCheck=new QCheckBox(g_lang.value("task.all_mode","全量翻译模式"),this); lay->addWidget(m_allModeCheck); m_exportCheck=new QCheckBox(g_lang.value("task.export_inspect","导出审核文件(.translang)"),this); lay->addWidget(m_exportCheck); }
    else if(type==Separate){ lay->addWidget(new QLabel(g_lang.value("task.separate","分离语言更新"),this)); addFR("task.src_file_new",m_file0Edit,"所有格式 (*.jar *.zip *.lang *.json);;所有文件 (*)"); addFR("task.ref_file_old",m_file1Edit,"所有格式 (*.jar *.zip *.lang *.json);;所有文件 (*)"); }
    else { lay->addWidget(new QLabel(g_lang.value("task.merge","合并语言更新"),this)); addFR("task.src_file_orig",m_file0Edit,"所有格式 (*.jar *.zip *.lang *.json);;所有文件 (*)"); addFR("task.notlang_file",m_notlangEdit,"语言文件 (*.lang *.json *.translang);;所有文件 (*)"); addFR("task.ref_file",m_file1Edit,"所有格式 (*.jar *.zip *.lang *.json);;所有文件 (*)"); }
    auto *btnBox=new QHBoxLayout(); auto *ok=new QPushButton(g_lang.value("btn.start","开始"),this); connect(ok,&QPushButton::clicked,this,&QDialog::accept); auto *cancel=new QPushButton(g_lang.value("btn.cancel","取消"),this); connect(cancel,&QPushButton::clicked,this,&QDialog::reject); btnBox->addStretch(); btnBox->addWidget(ok); btnBox->addWidget(cancel); lay->addLayout(btnBox);
}
QString TaskDialog::file0Path()const{return m_file0Edit?m_file0Edit->text():QString();}
QString TaskDialog::file1Path()const{return m_file1Edit?m_file1Edit->text():QString();}
QString TaskDialog::notlangPath()const{return m_notlangEdit?m_notlangEdit->text():QString();}
bool TaskDialog::allMode()const{return m_allModeCheck&&m_allModeCheck->isChecked();}
bool TaskDialog::exportInspection()const{return m_exportCheck&&m_exportCheck->isChecked();}
QString TaskDialog::taskName()const{return m_nameEdit?m_nameEdit->text():QString();}
QString TaskDialog::browseFile(const QString &filter){return QFileDialog::getOpenFileName(this,g_lang.value("btn.browse","选择文件"),QString(),filter);}

// ===== MainWindow =====
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_backend(new PythonBackend(this)), m_settingsDlg(nullptr) {
    // 加载语言文件
    QStringList langPaths = {"gui_zh_CN.json", "../gui_zh_CN.json", "../../gui_zh_CN.json",
                              QCoreApplication::applicationDirPath()+"/gui_zh_CN.json"};
    for (const auto &p : langPaths) { if (QFile::exists(p)) { loadLangJson(p); break; } }

    // 持久化：加载设置
    QSettings s("TranslatorMC", "TranslatorMinecraft");
    m_apiUrl = s.value("api/url", "https://api.tanslamc.top").toString();
    m_apiKey = s.value("api/key", "sk-123456").toString();
    m_savePath = s.value("save/path", QCoreApplication::applicationDirPath()).toString();
    m_maxLogLines = s.value("log/maxLines", 1500).toInt();
    m_pollIntervalMs = s.value("log/pollInterval", 500).toInt();
    m_autoScroll = s.value("log/autoScroll", true).toBool();
    m_backend->setPollInterval(m_pollIntervalMs);
    restoreGeometry(s.value("ui/geometry").toByteArray());

    qApp->setStyleSheet(APP_QSS); setupUi();
    connect(m_backend,&PythonBackend::taskCreated,this,&MainWindow::onTaskCreated);
    connect(m_backend,&PythonBackend::taskStatusChanged,this,&MainWindow::onTaskStatusChanged);
    connect(m_backend,&PythonBackend::taskLogReceived,this,&MainWindow::onTaskLogReceived);
    connect(m_backend,&PythonBackend::taskCompleted,this,&MainWindow::onTaskCompleted);
    connect(m_backend,&PythonBackend::errorOccurred,this,&MainWindow::onServerError);
    connect(m_backend,&PythonBackend::serverLog,this,&MainWindow::onServerLog);
    // 恢复任务列表
    QFile tf("tasks.json");
    if (tf.open(QIODevice::ReadOnly)) {
        QJsonArray arr = QJsonDocument::fromJson(tf.readAll()).array();
        for (const auto &v : arr) {
            QJsonObject o = v.toObject();
            TaskInfo info; info.taskId = o["id"].toString(); info.displayName = o["name"].toString();
            info.typeName = o["type"].toString(); info.status = o["status"].toString();
            info.progress = o["progress"].toInt();
            info.filename = o["file"].toString(); info.downloadUrl = o["url"].toString();
            for (const auto &l : o["logs"].toArray()) info.logs.append(l.toString());
            m_tasks[info.taskId] = info;
        }
        refreshTaskList();
    }
    m_backend->setApiConfig(m_apiUrl,m_apiKey);
}
MainWindow::~MainWindow(){
    // 持久化：保存设置
    QSettings s("TranslatorMC", "TranslatorMinecraft");
    s.setValue("api/url", m_apiUrl);
    s.setValue("api/key", m_apiKey);
    s.setValue("save/path", m_savePath);
    s.setValue("log/maxLines", m_maxLogLines);
    s.setValue("log/pollInterval", m_pollIntervalMs);
    s.setValue("log/autoScroll", m_autoScroll);
    s.setValue("ui/geometry", saveGeometry());
    // 保存任务列表
    QJsonArray arr;
    for (auto it=m_tasks.begin(); it!=m_tasks.end(); ++it) {
        QJsonObject o; o["id"]=it->taskId; o["name"]=it->displayName;
        o["type"]=it->typeName; o["status"]=it->status; o["progress"]=it->progress;
        o["file"]=it->filename; o["url"]=it->downloadUrl;
        QJsonArray la; for (const auto &l : it->logs) la.append(l); o["logs"]=la;
        arr.append(o);
    }
    QFile tf("tasks.json");
    if (tf.open(QIODevice::WriteOnly)) { tf.write(QJsonDocument(arr).toJson()); tf.close(); }
    m_backend->stopServer();
}

void MainWindow::setupUi(){
    auto *c=new QWidget(this); c->setStyleSheet("background: rgba(255,255,255,50);");
    setCentralWidget(c); auto *lay=new QVBoxLayout(c); lay->setContentsMargins(0,0,0,0); lay->setSpacing(0);
    setupTitleBar(lay);
    auto *content=new QWidget(c); auto *clay=new QVBoxLayout(content); clay->setContentsMargins(12,8,12,8); clay->setSpacing(6);

    auto *splitter=new QSplitter(Qt::Horizontal,this);
    auto *left=new QWidget(this); auto *ll=new QVBoxLayout(left); ll->setContentsMargins(0,0,0,0);
    ll->addWidget(new QLabel(g_lang.value("label.task_list","任务列表"),this));
    m_taskList=new QListWidget(this); m_taskList->setMinimumWidth(200);
    m_taskList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_taskList,&QListWidget::customContextMenuRequested,this,[this](const QPoint &pos){
        auto *item = m_taskList->itemAt(pos);
        if (!item) return;
        QMenu ctx;
        ctx.addAction("重命名", [this,item](){
            bool ok; QString name = QInputDialog::getText(this, "重命名任务", "新名称:", QLineEdit::Normal, item->text(), &ok);
            if (ok && !name.isEmpty()) {
                QString tid = item->data(Qt::UserRole).toString();
                if (m_tasks.contains(tid)) { m_tasks[tid].displayName = name; refreshTaskList(); }
            }
        });
        ctx.addAction("删除任务", [this,item](){
            QString tid = item->data(Qt::UserRole).toString();
            m_tasks.remove(tid); refreshTaskList();
            if (m_selectedTaskId == tid) { m_selectedTaskId.clear(); m_detailLabel->setText("选择一个任务查看详情"); m_statusLabel->setText("状态: -"); m_progressBar->setValue(0); m_logView->clear(); }
        });
        ctx.exec(m_taskList->mapToGlobal(pos));
    });
    connect(m_taskList,&QListWidget::currentItemChanged,this,[this](){onTaskListSelectionChanged();}); ll->addWidget(m_taskList);
    auto *br=new QHBoxLayout(); br->setSpacing(2);
    m_newTaskBtn=new QPushButton(g_lang.value("btn.new_task","新建任务"),this);
    connect(m_newTaskBtn,&QPushButton::clicked,this,[this](){
        QMenu menu;
        menu.addAction(g_lang.value("task.translate","翻译文件"),this,[this](){ onNewTaskType(0); });
        menu.addAction(g_lang.value("task.separate","分离语言更新"),this,[this](){ onNewTaskType(1); });
        menu.addAction(g_lang.value("task.merge","合并语言更新"),this,[this](){ onNewTaskType(2); });
        menu.exec(QCursor::pos());
    });
    br->addWidget(m_newTaskBtn,1);
    m_settingsBtn=new QPushButton("云设置",this); connect(m_settingsBtn,&QPushButton::clicked,this,&MainWindow::onOpenSettings); br->addWidget(m_settingsBtn,1);
    auto *localBtn=new QPushButton("本地设置",this); connect(localBtn,&QPushButton::clicked,this,[this](){
        QDialog dlg(this); dlg.setWindowTitle("本地设置"); dlg.setMinimumWidth(450); dlg.setWindowOpacity(0.75); dlg.setStyleSheet(APP_QSS);
        auto *fl=new QFormLayout(&dlg);
        auto makeInput=[&](QLineEdit *&e, const QString &txt){
            auto *outer=new QFrame(&dlg); outer->setProperty("inputOuter",true);
            auto *ol=new QHBoxLayout(outer); ol->setContentsMargins(4,2,4,2);
            auto *label=new QLabel(txt,outer); ol->addWidget(label);
            auto *inner=new QFrame(outer); inner->setProperty("inputWrapper",true);
            auto *il=new QHBoxLayout(inner); il->setContentsMargins(0,0,0,0); il->addWidget(e);
            ol->addWidget(inner,1);
            fl->addRow(outer);
        };
        auto *urlE=new QLineEdit(m_apiUrl,&dlg); makeInput(urlE,"API URL:");
        auto *keyE=new QLineEdit(m_apiKey,&dlg); keyE->setEchoMode(QLineEdit::Password); makeInput(keyE,"API KEY:");
        auto *pathRow=new QWidget(&dlg); auto *phl=new QHBoxLayout(pathRow); phl->setContentsMargins(0,0,0,0);
        auto *pathEW=new QFrame(pathRow); pathEW->setProperty("inputWrapper",true);
        auto *pathEWl=new QHBoxLayout(pathEW); pathEWl->setContentsMargins(0,0,0,0);
        auto *pathE=new QLineEdit(m_savePath,pathEW); pathEWl->addWidget(pathE);
        phl->addWidget(pathEW);
        auto *pathBrowse=new QPushButton("浏览...",pathRow); connect(pathBrowse,&QPushButton::clicked,[&](){ QString p=QFileDialog::getExistingDirectory(&dlg,"选择保存路径",pathE->text()); if(!p.isEmpty()) pathE->setText(p); });
        phl->addWidget(pathBrowse); fl->addRow("保存路径:",pathRow);
        auto *logMaxE=new QLineEdit(QString::number(m_maxLogLines),&dlg); makeInput(logMaxE,"日志上限(行):");
        auto *logIntervalE=new QLineEdit(QString::number(m_pollIntervalMs),&dlg); makeInput(logIntervalE,"日志刷新间隔(ms):");
        auto *bb=new QHBoxLayout(); auto *ok=new QPushButton("保存",&dlg); auto *cancel=new QPushButton("取消",&dlg);
        connect(ok,&QPushButton::clicked,&dlg,&QDialog::accept); connect(cancel,&QPushButton::clicked,&dlg,&QDialog::reject);
        bb->addStretch(); bb->addWidget(ok); bb->addWidget(cancel); fl->addRow(bb);
        if(dlg.exec()==QDialog::Accepted){ m_apiUrl=urlE->text(); m_apiKey=keyE->text(); m_savePath=pathE->text(); m_maxLogLines=logMaxE->text().toInt(); m_pollIntervalMs=logIntervalE->text().toInt(); m_backend->setApiConfig(m_apiUrl,m_apiKey); m_backend->setPollInterval(m_pollIntervalMs); }
    }); br->addWidget(localBtn,1);
    ll->addLayout(br);
    splitter->addWidget(left);
    auto *right=new QWidget(this); auto *rl=new QVBoxLayout(right); rl->setContentsMargins(0,0,0,0);
    m_detailLabel=new QLabel(g_lang.value("label.select_task","选择一个任务查看详情"),this); m_detailLabel->setStyleSheet("font-weight:bold;font-size:15px;"); rl->addWidget(m_detailLabel);
    auto *sr=new QHBoxLayout(); m_statusLabel=new QLabel(g_lang.value("label.status","状态:")+" -",this); sr->addWidget(m_statusLabel); sr->addStretch();
    m_autoScrollBtn=new QPushButton("自动滚动: ON",this); m_autoScrollBtn->setMaximumWidth(120); m_autoScrollBtn->setCheckable(true); m_autoScrollBtn->setChecked(m_autoScroll); m_autoScrollBtn->setStyleSheet("background:#6c757d;"); connect(m_autoScrollBtn,&QPushButton::clicked,[this](){ m_autoScroll=!m_autoScroll; m_autoScrollBtn->setText(m_autoScroll?"自动滚动: ON":"自动滚动: OFF"); }); sr->addWidget(m_autoScrollBtn);
    m_cancelBtn=new QPushButton(g_lang.value("btn.cancel_task","取消任务"),this); m_cancelBtn->setMaximumWidth(100); m_cancelBtn->setStyleSheet("background:#6c757d;"); connect(m_cancelBtn,&QPushButton::clicked,this,&MainWindow::onCancelCurrentTask); sr->addWidget(m_cancelBtn);
    rl->addLayout(sr); m_progressBar=new QProgressBar(this); m_progressBar->setRange(0,100); m_progressBar->setMinimumHeight(22); rl->addWidget(m_progressBar);
    rl->addWidget(new QLabel(g_lang.value("label.log_output","日志输出:"),this)); m_logView=new QTextEdit(this); m_logView->setReadOnly(true);
    m_logView->setStyleSheet("QTextEdit { background: #1e1f29; color: #abb2bf; border: 2px solid rgba(179,179,179,137); font-family: Consolas, 'Courier New', monospace; selection-background-color: #3a4a6a; }");
    rl->addWidget(m_logView,1);
    splitter->addWidget(right); splitter->setStretchFactor(0,0); splitter->setStretchFactor(1,1); clay->addWidget(splitter,1);
    lay->addWidget(content,1);
}

void MainWindow::setupTitleBar(QVBoxLayout *topLayout){
    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(32);
    m_titleBar->setStyleSheet("background: rgba(255,255,255,30);");
    auto *hb = new QHBoxLayout(m_titleBar);
    hb->setContentsMargins(10,0,4,0); hb->setSpacing(4);

    auto *title = new QLabel("TranslatorMinecraft", m_titleBar);
    title->setStyleSheet("color:#1a1a1a; font-weight:bold; font-size:13px;");
    hb->addWidget(title);
    hb->addStretch();

    auto makeBtn = [&](const QString &text, const QString &style){
        auto *btn = new QPushButton(text, m_titleBar);
        btn->setFixedSize(28,22);
        btn->setStyleSheet(style);
        return btn;
    };
    auto *minBtn = makeBtn("—", "background:transparent; color:#4a4a4a; border:none; font-size:14px; padding:0;");
    connect(minBtn, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    hb->addWidget(minBtn);

    auto *maxBtn = makeBtn("□", "background:transparent; color:#4a4a4a; border:none; font-size:14px; padding:0;");
    connect(maxBtn, &QPushButton::clicked, this, [this](){
        if (isMaximized()) showNormal(); else showMaximized();
    });
    hb->addWidget(maxBtn);

    auto *closeBtn = makeBtn("×", "background:transparent; color:#4a4a4a; border:none; font-size:16px; padding:0;");
    closeBtn->setStyleSheet(closeBtn->styleSheet() + " QPushButton:hover { background:#e81123; color:white; }");
    connect(closeBtn, &QPushButton::clicked, this, &QMainWindow::close);
    hb->addWidget(closeBtn);

    topLayout->addWidget(m_titleBar);
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton && m_titleBar) {
        QPoint p = m_titleBar->mapFromGlobal(event->globalPosition().toPoint());
        if (p.y() >= 0 && p.y() < m_titleBar->height()) {
            m_dragging = true;
            m_dragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragStart);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event);
    m_dragging = false;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event){
    if (m_titleBar) {
        QPoint p = m_titleBar->mapFromGlobal(event->globalPosition().toPoint());
        if (p.y() >= 0 && p.y() < m_titleBar->height()) {
            if (isMaximized()) showNormal(); else showMaximized();
        }
    }
}

void MainWindow::onNewTaskType(int idx){ TaskDialog::TaskType tt; QString tn; if(idx==0){tt=TaskDialog::Translate;tn="翻译";}else if(idx==1){tt=TaskDialog::Separate;tn="分离";}else{tt=TaskDialog::Merge;tn="合并";} TaskDialog dlg(tt,this); int result=dlg.exec(); if(result!=QDialog::Accepted||dlg.file0Path().isEmpty()){if(result==QDialog::Accepted)QMessageBox::warning(this,g_lang.value("msg.select_file","提示"),g_lang.value("msg.select_file","请选择输入文件"));return;} m_backend->setApiConfig(m_apiUrl,m_apiKey); m_pendingTaskName=dlg.taskName().isEmpty()?tn:dlg.taskName(); if(tt==TaskDialog::Translate)m_backend->submitTranslate(dlg.file0Path(),dlg.file1Path(),dlg.allMode(),dlg.exportInspection()); else if(tt==TaskDialog::Separate)m_backend->submitSeparate(dlg.file0Path(),dlg.file1Path()); else m_backend->submitMerge(dlg.file0Path(),dlg.notlangPath(),dlg.file1Path()); }
void MainWindow::onOpenSettings(){if(!m_settingsDlg){m_settingsDlg=new SettingsDialog(this);connect(m_settingsDlg,&SettingsDialog::configSaved,this,&MainWindow::onConfigSaved);}m_settingsDlg->exec();}
void MainWindow::onConfigSaved(const QJsonObject &cfg){m_apiUrl=cfg["LLM_API_URL"].toString();m_apiKey=cfg["LLM_API_KEY"].toString();m_backend->setApiConfig(m_apiUrl,m_apiKey);}
void MainWindow::onTaskCreated(const QString &tn,const QString &tid){addTask(tn,tid);}
void MainWindow::addTask(const QString &tn,const QString &tid){int idx=(tn=="翻译")?0:(tn=="分离")?1:2;m_counters[idx]++;QString name=m_pendingTaskName.isEmpty()?QString("%1_%2").arg(tn).arg(m_counters[idx]):m_pendingTaskName;m_pendingTaskName.clear();TaskInfo info{tid,name,tn,"queued",0,{},"",""};m_tasks[tid]=info;refreshTaskList();m_selectedTaskId=tid;for(int i=0;i<m_taskList->count();i++)if(m_taskList->item(i)->data(Qt::UserRole).toString()==tid){m_taskList->blockSignals(true);m_taskList->setCurrentRow(i);m_taskList->blockSignals(false);break;}showTaskDetail(tid);}
void MainWindow::onTaskStatusChanged(const QString &tid,const QString &st,int pg,const QString &){updateTaskInfo(tid,st,pg);if(m_selectedTaskId==tid)showTaskDetail(tid);}
void MainWindow::onTaskLogReceived(const QString &tid,const QStringList &logs){if(!m_tasks.contains(tid))return;int added=0;for(auto &l:logs){bool dup=false;for(auto &existing:m_tasks[tid].logs){if(existing==l){dup=true;break;}}if(!dup){m_tasks[tid].logs.append(l);added++;while(m_tasks[tid].logs.size()>m_maxLogLines)m_tasks[tid].logs.removeFirst();}}if(added>0&&m_selectedTaskId==tid){for(int i=m_tasks[tid].logs.size()-added;i<m_tasks[tid].logs.size();i++)m_logView->append(m_tasks[tid].logs[i]);if(m_autoScroll){auto c=m_logView->textCursor();c.movePosition(QTextCursor::End);m_logView->setTextCursor(c);}}}
void MainWindow::onTaskCompleted(const QString &tid,const QString &fn,const QString &url){
    if(m_tasks.contains(tid)){m_tasks[tid].status="completed";m_tasks[tid].progress=100;m_tasks[tid].filename=fn;m_tasks[tid].downloadUrl=url;}
    updateTaskInfo(tid,"completed",100);refreshTaskList();if(m_selectedTaskId==tid)showTaskDetail(tid);
    // 静默保存到本地设置路径
    if(!url.isEmpty()){
        QDir().mkpath(m_savePath);
        QString savePath = m_savePath + "/" + fn;
        auto *req = new QNetworkRequest(QUrl(url));
        req->setRawHeader("Authorization","Bearer "+m_apiKey.toUtf8());
        QNetworkReply *reply = m_backend->networkManager()->get(*req);
        connect(reply,&QNetworkReply::finished,this,[reply,savePath,req](){
            if(reply->error()==QNetworkReply::NoError){
                QFile f(savePath); if(f.open(QIODevice::WriteOnly)){f.write(reply->readAll());f.close();}
            }
            reply->deleteLater(); delete req;
        });
    }
}
void MainWindow::onServerError(const QString &e){appendTaskLog("","[ERROR] "+e);}
void MainWindow::onServerLog(const QString &m){ /* process stdout - informational only */ }
void MainWindow::updateTaskInfo(const QString &tid,const QString &st,int pg){if(m_tasks.contains(tid)){m_tasks[tid].status=st;m_tasks[tid].progress=pg;}refreshTaskList();}
void MainWindow::appendTaskLog(const QString &tid,const QString &l){if(tid.isEmpty()||!m_tasks.contains(tid))return;m_tasks[tid].logs.append(l);while(m_tasks[tid].logs.size()>m_maxLogLines)m_tasks[tid].logs.removeFirst();}
void MainWindow::refreshTaskList(){QString sel=m_selectedTaskId;m_taskList->clear();for(auto it=m_tasks.begin();it!=m_tasks.end();++it){QString t=it->displayName;if(it->status=="completed")t+=" ✓";else if(it->status=="failed")t+=" ✗";else if(it->status=="processing")t+=" …";else t+=" ⌛";auto *item=new QListWidgetItem(t);item->setData(Qt::UserRole,it->taskId);if(it->status=="completed")item->setForeground(QColor("#28a745"));else if(it->status=="failed")item->setForeground(QColor("#dc3545"));else if(it->status=="processing")item->setForeground(QColor("#4361ee"));m_taskList->addItem(item);}if(!sel.isEmpty()){for(int i=0;i<m_taskList->count();i++){if(m_taskList->item(i)->data(Qt::UserRole).toString()==sel){m_taskList->setCurrentRow(i);break;}}}}
void MainWindow::onTaskListSelectionChanged(){auto *item=m_taskList->currentItem();if(item){QString tid=item->data(Qt::UserRole).toString();if(m_selectedTaskId!=tid){m_selectedTaskId=tid;showTaskDetail(tid);}}}
void MainWindow::showTaskDetail(const QString &tid){if(!m_tasks.contains(tid))return;auto &i=m_tasks[tid];m_detailLabel->setText(i.displayName+" ("+i.typeName+")");m_statusLabel->setText(g_lang.value("label.status","状态:")+" "+i.status);m_progressBar->setValue(i.progress);m_logView->clear();for(auto &l:i.logs)m_logView->append(l);auto c=m_logView->textCursor();c.movePosition(QTextCursor::End);m_logView->setTextCursor(c);m_cancelBtn->setVisible(i.status!="completed");}
void MainWindow::onDownloadCurrentTask(){if(!m_selectedTaskId.isEmpty()&&m_tasks.contains(m_selectedTaskId)){auto &i=m_tasks[m_selectedTaskId];if(!i.downloadUrl.isEmpty())QDesktopServices::openUrl(QUrl(i.downloadUrl));}}
void MainWindow::onCancelCurrentTask(){if(!m_selectedTaskId.isEmpty())updateTaskInfo(m_selectedTaskId,"cancelled",0);refreshTaskList();}
