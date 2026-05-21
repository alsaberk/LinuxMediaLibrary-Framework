#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QVariantAnimation>

// ─── Constructor ─────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_lFile1(new LFile()),
      m_lFile2(new LFile()),
      m_lLive1(new LLive()),
      m_lLive2(new LLive()),
      m_lSwitcher(new LSwitcher()),
      m_lMixer(new LMixer()),
      m_lPreviewFile1(new LPreview()),
      m_lPreviewFile2(new LPreview()),
      m_lPreviewLive1(new LPreview()),
      m_lPreviewLive2(new LPreview()),
      m_lPreviewPrev(new LPreview()),
      m_lPreviewProg(new LPreview()),
      m_uiTimer(new QTimer(this))
{
  ui->setupUi(this);

  // 1. Configure Video Formats (1080p50)
  videoFormatProps formatProps;
  formatProps.setVideoFormat = vF::HD1080_50p;
  m_lMixer->setVideoFormat(formatProps);
  m_lSwitcher->setVideoFormat(formatProps);

  // Set normalized video and audio formats for Live inputs
  m_lLive1->setVideoFormat(formatProps);
  m_lLive2->setVideoFormat(formatProps);
  m_lLive1->setProps("gpu", "true");
  m_lLive2->setProps("gpu", "true");

  // 2. Connect Switcher to Mixer as Base Layer (z-index 0)
  LLayerItem* swicherlayer = m_lMixer->addLayer("base_switcher", m_lSwitcher, 0, 0, 0, 255, 1920, 1080);
  swicherlayer->audio = true;
  // 3. Configure Input Previews
  m_lPreviewFile1->setProps("ui_framework", "qt");
  m_lPreviewFile1->previewEnable(ui->file1Preview, false, true);
  m_lPreviewFile1->previewObject(m_lFile1);

  m_lPreviewFile2->setProps("ui_framework", "qt");
  m_lPreviewFile2->previewEnable(ui->file2Preview, false, true);
  m_lPreviewFile2->previewObject(m_lFile2);

  m_lPreviewLive1->setProps("ui_framework", "qt");
  m_lPreviewLive1->previewEnable(ui->live1Preview, false, true);
  m_lPreviewLive1->previewObject(m_lLive1);

  m_lPreviewLive2->setProps("ui_framework", "qt");
  m_lPreviewLive2->previewEnable(ui->live2Preview, false, true);
  m_lPreviewLive2->previewObject(m_lLive2);

  // 4. Configure Switcher Bus Previews
  // Preview Bus (Next) shows the cued source
  m_lPreviewPrev->setProps("ui_framework", "qt");
  m_lPreviewPrev->previewEnable(ui->previewPreview, false, true);
  m_lPreviewPrev->previewObject(getSourceObject(m_activePreviewSource));

  // Program Bus (On Air) shows the final Mixer output (Switcher + Logo)
  m_lPreviewProg->setProps("ui_framework", "qt");
  m_lPreviewProg->previewEnable(ui->programPreview, true, true); // audio meters enabled
  m_lPreviewProg->previewObject(m_lMixer);

  // 5. Initialize Transition Types
  ui->transTypeCb->addItem("Fade", "fade");
  ui->transTypeCb->addItem("Wipe Left-to-Right", "wipe_lr");
  ui->transTypeCb->addItem("Wipe Right-to-Left", "wipe_rl");
  ui->transTypeCb->addItem("Wipe Top-to-Bottom", "wipe_tb");
  ui->transTypeCb->addItem("Wipe Bottom-to-Top", "wipe_bt");
  ui->transTypeCb->addItem("Wipe Diagonal", "wipe_diagonal");
  ui->transTypeCb->addItem("Wipe Box", "wipe_box");
  ui->transTypeCb->addItem("Wipe Circle", "wipe_circle");
  ui->transTypeCb->addItem("Slide Left-to-Right", "slide_lr");
  ui->transTypeCb->addItem("Slide Right-to-Left", "slide_rl");
  ui->transTypeCb->addItem("Slide Top-to-Bottom", "slide_tb");
  ui->transTypeCb->addItem("Slide Bottom-to-Top", "slide_bt");
  ui->transTypeCb->addItem("Squeeze Left-to-Right", "squeeze_lr");
  ui->transTypeCb->addItem("Squeeze Right-to-Left", "squeeze_rl");
  ui->transTypeCb->addItem("Dip to Custom Color", "dip");
  ui->transTypeCb->addItem("Stinger Transition", "stinger");

  // 6. Connect Switcher Initial State
  m_lSwitcher->setProgram(getSourceObject(m_activeProgramSource));
  m_lSwitcher->setPreview(getSourceObject(m_activePreviewSource));
  m_lSwitcher->setTransitionType("fade");

  // 7. Connect UI Signals & Slots
  // File 1 & 2 Controls
  connect(ui->openFile1Btn, &QPushButton::clicked, this, &MainWindow::onOpenFile1);
  connect(ui->playFile1Btn, &QPushButton::clicked, this, &MainWindow::onPlayFile1);
  connect(ui->stopFile1Btn, &QPushButton::clicked, this, &MainWindow::onStopFile1);

  connect(ui->openFile2Btn, &QPushButton::clicked, this, &MainWindow::onOpenFile2);
  connect(ui->playFile2Btn, &QPushButton::clicked, this, &MainWindow::onPlayFile2);
  connect(ui->stopFile2Btn, &QPushButton::clicked, this, &MainWindow::onStopFile2);

  // Live 1 Controls
  connect(ui->live1Btn, &QPushButton::clicked, this, [this]() { onLiveToggle(1); });
  connect(ui->live1DeviceCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) { onLiveDeviceChanged(1, idx); });
  connect(ui->live1ChannelCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) { onLiveChannelChanged(1, idx); });
  connect(ui->live1FormatCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) { onLiveFormatChanged(1, idx); });

  // Live 2 Controls
  connect(ui->live2Btn, &QPushButton::clicked, this, [this]() { onLiveToggle(2); });
  connect(ui->live2DeviceCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) { onLiveDeviceChanged(2, idx); });
  connect(ui->live2ChannelCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) { onLiveChannelChanged(2, idx); });
  connect(ui->live2FormatCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) { onLiveFormatChanged(2, idx); });

  // Switcher Bus Selection Buttons
  connect(ui->progFile1Btn, &QPushButton::clicked, this, [this]() { setProgramSource(0); });
  connect(ui->progFile2Btn, &QPushButton::clicked, this, [this]() { setProgramSource(1); });
  connect(ui->progLive1Btn, &QPushButton::clicked, this, [this]() { setProgramSource(2); });
  connect(ui->progLive2Btn, &QPushButton::clicked, this, [this]() { setProgramSource(3); });

  connect(ui->prevFile1Btn, &QPushButton::clicked, this, [this]() { setPreviewSource(0); });
  connect(ui->prevFile2Btn, &QPushButton::clicked, this, [this]() { setPreviewSource(1); });
  connect(ui->prevLive1Btn, &QPushButton::clicked, this, [this]() { setPreviewSource(2); });
  connect(ui->prevLive2Btn, &QPushButton::clicked, this, [this]() { setPreviewSource(3); });

  // Transition controls
  connect(ui->transTypeCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onTransitionTypeChanged);
  connect(ui->cutBtn, &QPushButton::clicked, this, &MainWindow::onCutTransition);
  connect(ui->autoBtn, &QPushButton::clicked, this, &MainWindow::onAutoTransition);
  connect(ui->stingerBtn, &QPushButton::clicked, this, &MainWindow::onStingerTransition);
  connect(ui->tBarSlider, &QSlider::valueChanged, this, &MainWindow::onTBarChanged);

  // Stinger controls
  connect(ui->stingerPathBtn, &QPushButton::clicked, this, &MainWindow::onSelectStingerPath);
  connect(ui->stingerDurationSb, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onStingerDurationChanged);

  // Logo controls
  connect(ui->logoPathBtn, &QPushButton::clicked, this, &MainWindow::onSelectLogoPath);
  connect(ui->logoXSlider, &QSlider::valueChanged, this, &MainWindow::onLogoLayoutChanged);
  connect(ui->logoYSlider, &QSlider::valueChanged, this, &MainWindow::onLogoLayoutChanged);
  connect(ui->logoAlphaSlider, &QSlider::valueChanged, this, &MainWindow::onLogoLayoutChanged);
  connect(ui->logoToggleBtn, &QPushButton::toggled, this, &MainWindow::onToggleLogo);

  // 8. Populate Live Devices Comboboxes
  refreshLiveSources(1);
  refreshLiveSources(2);

  // Update initial active bus button highlights
  updateBusButtons();

  // 9. Start UI Update Loop Timer (50ms interval)
  connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::updateUI);
  m_uiTimer->start(50);
}

// ─── Destructor ──────────────────────────────────────────────────────────────
MainWindow::~MainWindow() {
  if (m_lFile1) m_lFile1->stop();
  if (m_lFile2) m_lFile2->stop();
  if (m_lLive1) m_lLive1->Stop();
  if (m_lLive2) m_lLive2->Stop();

  delete m_lPreviewFile1;
  delete m_lPreviewFile2;
  delete m_lPreviewLive1;
  delete m_lPreviewLive2;
  delete m_lPreviewPrev;
  delete m_lPreviewProg;

  delete m_lFile1;
  delete m_lFile2;
  delete m_lLive1;
  delete m_lLive2;
  delete m_lSwitcher;
  delete m_lMixer;
  delete ui;
}

// ─── Source Fetch Helper ─────────────────────────────────────────────────────
LObject* MainWindow::getSourceObject(int index) {
  switch (index) {
    case 0: return m_lFile1;
    case 1: return m_lFile2;
    case 2: return m_lLive1;
    case 3: return m_lLive2;
    default: return nullptr;
  }
}

// ─── File 1 Controls ─────────────────────────────────────────────────────────
void MainWindow::onOpenFile1() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Select Media File for Layer 1", "",
      "Media Files (*.mp4 *.mkv *.avi *.mov *.mpg *.mpeg *.webm *.mxf *.ts)");
  if (!filePath.isEmpty()) {
    m_lFile1->stop();
    m_lFile1->fileNameSet(filePath.toStdString());
    m_lFile1->play();
    ui->file1PathLbl->setText(QFileInfo(filePath).fileName());
  }
}

void MainWindow::onPlayFile1() {
  m_lFile1->play();
}

void MainWindow::onStopFile1() {
  m_lFile1->stop();
}

// ─── File 2 Controls ─────────────────────────────────────────────────────────
void MainWindow::onOpenFile2() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Select Media File for Layer 2", "",
      "Media Files (*.mp4 *.mkv *.avi *.mov *.mpg *.mpeg *.webm *.mxf *.ts)");
  if (!filePath.isEmpty()) {
    m_lFile2->stop();
    m_lFile2->fileNameSet(filePath.toStdString());
    m_lFile2->play();
    ui->file2PathLbl->setText(QFileInfo(filePath).fileName());
  }
}

void MainWindow::onPlayFile2() {
  m_lFile2->play();
}

void MainWindow::onStopFile2() {
  m_lFile2->stop();
}

// ─── Live Device Controllers ─────────────────────────────────────────────────
void MainWindow::refreshLiveSources(int liveIndex) {
  LLive* live = (liveIndex == 1) ? m_lLive1 : m_lLive2;
  QComboBox* devCb = (liveIndex == 1) ? ui->live1DeviceCb : ui->live2DeviceCb;

  devCb->blockSignals(true);
  devCb->clear();
  int count = 0;
  live->DeviceGetCount(count);
  for (int i = 0; i < count; i++) {
    std::string name, desc;
    live->DeviceGetByIndex(i, name, desc);
    devCb->addItem(QString::fromStdString(name));
  }
  devCb->blockSignals(false);
  if (count > 0) {
    devCb->setCurrentIndex(0);
    onLiveDeviceChanged(liveIndex, 0);
  }
}

void MainWindow::onLiveDeviceChanged(int liveIndex, int deviceIdx) {
  if (deviceIdx < 0) return;
  LLive* live = (liveIndex == 1) ? m_lLive1 : m_lLive2;
  QComboBox* chanCb = (liveIndex == 1) ? ui->live1ChannelCb : ui->live2ChannelCb;

  int status = 0;
  live->statusGet(status);
  if (status == 1) live->Stop();

  live->DeviceSet(deviceIdx);

  chanCb->blockSignals(true);
  chanCb->clear();
  int count = 0;
  live->DeviceChannelGetCount(deviceIdx, count);
  for (int i = 0; i < count; i++) {
    std::string name, desc;
    live->DeviceChannelGetByIndex(deviceIdx, i, name, desc);
    chanCb->addItem(QString::fromStdString(name));
  }
  chanCb->blockSignals(false);
  if (count > 0) {
    chanCb->setCurrentIndex(0);
    onLiveChannelChanged(liveIndex, 0);
  }

  if (status == 1) live->Start();
}

void MainWindow::onLiveChannelChanged(int liveIndex, int channelIdx) {
  if (channelIdx < 0) return;
  LLive* live = (liveIndex == 1) ? m_lLive1 : m_lLive2;
  QComboBox* devCb = (liveIndex == 1) ? ui->live1DeviceCb : ui->live2DeviceCb;
  QComboBox* fmtCb = (liveIndex == 1) ? ui->live1FormatCb : ui->live2FormatCb;
  int devIdx = devCb->currentIndex();

  int status = 0;
  live->statusGet(status);
  if (status == 1) live->Stop();

  live->DeviceChannelSet(channelIdx);

  fmtCb->blockSignals(true);
  fmtCb->clear();
  int count = 0;
  live->DeviceFormatVideoGetCount(devIdx, channelIdx, count);
  for (int i = 0; i < count; i++) {
    std::string name;
    videoFormatProps props;
    live->DeviceFormatVideoGetByIndex(devIdx, channelIdx, i, name, props);
    fmtCb->addItem(QString::fromStdString(name), i);
  }
  fmtCb->blockSignals(false);
  if (count > 0) {
    fmtCb->setCurrentIndex(0);
    onLiveFormatChanged(liveIndex, 0);
  }

  if (status == 1) live->Start();
}

void MainWindow::onLiveFormatChanged(int liveIndex, int formatIdx) {
  if (formatIdx < 0) return;
  LLive* live = (liveIndex == 1) ? m_lLive1 : m_lLive2;

  int status = 0;
  live->statusGet(status);
  if (status == 1) live->Stop();

  live->DeviceFormatVideoSet(formatIdx);

  if (status == 1) live->Start();
}

void MainWindow::onLiveToggle(int liveIndex) {
  LLive* live = (liveIndex == 1) ? m_lLive1 : m_lLive2;
  QPushButton* btn = (liveIndex == 1) ? ui->live1Btn : ui->live2Btn;
  QComboBox* fmtCb = (liveIndex == 1) ? ui->live1FormatCb : ui->live2FormatCb;

  int status = 0;
  live->statusGet(status);
  if (status == 0) {
    int formatIdx = fmtCb->currentData().toInt();
    live->DeviceFormatVideoSet(formatIdx);
    if (live->Start()) {
      btn->setText(QString("Stop Live %1").arg(liveIndex));
      btn->setStyleSheet("background-color: #ef4444; color: white; font-weight: bold;");
    }
  } else {
    live->Stop();
    btn->setText(QString("Start Live %1").arg(liveIndex));
    btn->setStyleSheet("");
  }
}

// ─── Switcher Source Selection ───────────────────────────────────────────────
void MainWindow::setProgramSource(int sourceIdx) {
  m_activeProgramSource = sourceIdx;
  m_lSwitcher->setProgram(getSourceObject(sourceIdx));
  updateBusButtons();
}

void MainWindow::setPreviewSource(int sourceIdx) {
  m_activePreviewSource = sourceIdx;
  m_lSwitcher->setPreview(getSourceObject(sourceIdx));
  m_lPreviewPrev->previewObject(getSourceObject(sourceIdx));
  updateBusButtons();
}

void MainWindow::updateBusButtons() {
  // Reset all styles
  QString defaultStyle = "background-color: #1f2937; color: #f3f4f6; border: 1px solid #374151; font-weight: bold;";
  QString activeProgStyle = "background-color: #ef4444; color: white; border: 1px solid #f87171; font-weight: bold;";
  QString activePrevStyle = "background-color: #22c55e; color: white; border: 1px solid #4ade80; font-weight: bold;";

  ui->progFile1Btn->setStyleSheet(defaultStyle);
  ui->progFile2Btn->setStyleSheet(defaultStyle);
  ui->progLive1Btn->setStyleSheet(defaultStyle);
  ui->progLive2Btn->setStyleSheet(defaultStyle);

  ui->prevFile1Btn->setStyleSheet(defaultStyle);
  ui->prevFile2Btn->setStyleSheet(defaultStyle);
  ui->prevLive1Btn->setStyleSheet(defaultStyle);
  ui->prevLive2Btn->setStyleSheet(defaultStyle);

  // Set active styles
  switch (m_activeProgramSource) {
    case 0: ui->progFile1Btn->setStyleSheet(activeProgStyle); break;
    case 1: ui->progFile2Btn->setStyleSheet(activeProgStyle); break;
    case 2: ui->progLive1Btn->setStyleSheet(activeProgStyle); break;
    case 3: ui->progLive2Btn->setStyleSheet(activeProgStyle); break;
  }

  switch (m_activePreviewSource) {
    case 0: ui->prevFile1Btn->setStyleSheet(activePrevStyle); break;
    case 1: ui->prevFile2Btn->setStyleSheet(activePrevStyle); break;
    case 2: ui->prevLive1Btn->setStyleSheet(activePrevStyle); break;
    case 3: ui->prevLive2Btn->setStyleSheet(activePrevStyle); break;
  }
}

// ─── Switcher Transition Slots ───────────────────────────────────────────────
void MainWindow::onTransitionTypeChanged(int index) {
  if (index < 0) return;
  std::string transType = ui->transTypeCb->itemData(index).toString().toStdString();
  m_lSwitcher->setTransitionType(transType);
}

void MainWindow::onCutTransition() {
  m_lSwitcher->setCutTransition();
  std::swap(m_activeProgramSource, m_activePreviewSource);

  // Re-sync preview objects & switcher layer sources
  m_lSwitcher->setProgram(getSourceObject(m_activeProgramSource));
  m_lSwitcher->setPreview(getSourceObject(m_activePreviewSource));
  m_lPreviewPrev->previewObject(getSourceObject(m_activePreviewSource));

  m_lSwitcher->setTransition(0.0f);

  // In CUT, since we complete the transition instantly, we flip the polarity
  // if the physical slider is at the opposite end.
  if (ui->tBarSlider->value() >= 100) {
    m_tBarReversed = true;
  } else if (ui->tBarSlider->value() <= 0) {
    m_tBarReversed = false;
  }

  updateBusButtons();
}

void MainWindow::onAutoTransition() {
  int startVal = ui->tBarSlider->value();
  int endVal = (startVal < 50) ? 100 : 0;

  QVariantAnimation* anim = new QVariantAnimation(this);
  anim->setStartValue(startVal);
  anim->setEndValue(endVal);
  anim->setDuration(1200);

  connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
    ui->tBarSlider->setValue(val.toInt());
  });

  connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
  anim->start();
}

void MainWindow::onStingerTransition() {
  if (m_stingerPath.empty()) {
    QMessageBox::warning(this, "Stinger Transition", "Please select a stinger transition folder path first!");
    return;
  }

  m_lSwitcher->setTransitionType("stinger");
  m_lSwitcher->setStingerDuration(ui->stingerDurationSb->value());

  int startVal = ui->tBarSlider->value();
  int endVal = (startVal < 50) ? 100 : 0;

  QVariantAnimation* anim = new QVariantAnimation(this);
  anim->setStartValue(startVal);
  anim->setEndValue(endVal);
  anim->setDuration(ui->stingerDurationSb->value());

  connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
    ui->tBarSlider->setValue(val.toInt());
  });

  connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
  anim->start();
}

void MainWindow::onTBarChanged(int value) {
  float progress = (float)value / 100.0f;
  if (m_tBarReversed) {
    progress = (100 - value) / 100.0f;
  }

  m_lSwitcher->setTransition(progress);

  // If transition progress reaches 1.0f (100% complete), swap logically!
  if (progress >= 1.0f) {
    m_lSwitcher->setCutTransition(); // Internally swaps layers
    m_lSwitcher->setTransition(0.0f); // Reset transition progress to 0.0f for the cued sources

    std::swap(m_activeProgramSource, m_activePreviewSource);
    m_lPreviewPrev->previewObject(getSourceObject(m_activePreviewSource));

    // Flip the slider direction polarity
    m_tBarReversed = !m_tBarReversed;

    updateBusButtons();
  }
}

// ─── Stinger Controls ────────────────────────────────────────────────────────
void MainWindow::onSelectStingerPath() {
  QString dirPath = QFileDialog::getExistingDirectory(
      this, "Select Stinger Image Sequence Directory (contains PNGs)", "/home/alsaberk/Masaüstü");
  if (!dirPath.isEmpty()) {
    // Check if there is Transition format name
    QString formatPath = dirPath + "/Transition #08_%05d.png";
    m_stingerPath = formatPath.toStdString();
    
    m_lSwitcher->setStinger(m_stingerPath);
    m_lSwitcher->setStingerDuration(ui->stingerDurationSb->value());

    ui->stingerPathLbl->setText(QFileInfo(dirPath).fileName());
  }
}

void MainWindow::onStingerDurationChanged(int value) {
  m_lSwitcher->setStingerDuration(value);
}

// ─── Mixer Logo Controls ─────────────────────────────────────────────────────
void MainWindow::onSelectLogoPath() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Select Logo Image (PNG with Alpha)", "/home/alsaberk/Masaüstü",
      "Images (*.png *.jpg *.jpeg *.bmp)");
  if (!filePath.isEmpty()) {
    m_logoPath = filePath.toStdString();
    ui->logoPathLbl->setText(QFileInfo(filePath).fileName());
    onLogoLayoutChanged();
  }
}

void MainWindow::onLogoLayoutChanged() {
  if (m_logoEnabled && !m_logoPath.empty()) {
    m_lMixer->removeLayer("logo_layer");

    float x = ui->logoXSlider->value();
    float y = ui->logoYSlider->value();
    int alpha = ui->logoAlphaSlider->value();

    // Add Logo to Mixer (Z-Index = 1, above Switcher base layer)
    m_lMixer->addImage("logo_layer", m_logoPath, 1, x, y, alpha);
  }
}

void MainWindow::onToggleLogo(bool checked) {
  m_logoEnabled = checked;
  if (checked) {
    if (m_logoPath.empty()) {
      QMessageBox::warning(this, "Logo Overlay", "Please select a PNG logo image first!");
      ui->logoToggleBtn->setChecked(false);
      m_logoEnabled = false;
      return;
    }
    ui->logoToggleBtn->setText("DISABLE LOGO OVERLAY");
    ui->logoToggleBtn->setStyleSheet("background-color: #ef4444; color: white; font-weight: bold;");
    onLogoLayoutChanged();
  } else {
    ui->logoToggleBtn->setText("ENABLE LOGO OVERLAY");
    ui->logoToggleBtn->setStyleSheet("");
    m_lMixer->removeLayer("logo_layer");
  }
}

// ─── UI / Preview Update Loop ────────────────────────────────────────────────
void MainWindow::updateUI() {
  // Trigger update on preview windows
  ui->file1Preview->update();
  ui->file2Preview->update();
  ui->live1Preview->update();
  ui->live2Preview->update();
  ui->previewPreview->update();
  ui->programPreview->update();

  // Fetch Switcher stats and update UI labels
  LSwitcher::SwitcherStats stats;
  m_lSwitcher->getStats(stats);

  ui->fpsValLbl->setText(QString::number(stats.fps, 'f', 2));
  ui->tcodeValLbl->setText(QString::fromStdString(stats.timecode));
  ui->onAirValLbl->setText(QString::fromStdString(stats.onAir));
}
