#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

// ─── Constructor ─────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_lFile1(new LFile()),
      m_lFile2(new LFile()),
      m_lMixer(new LMixer()),
      m_lCG(new LCharacter()),
      m_lPreviewFile1(new LPreview()),
      m_lPreviewFile2(new LPreview()),
      m_lPreviewMixer(new LPreview()),
      m_uiTimer(new QTimer(this))
{
  ui->setupUi(this);

  // 1. Configure Mixer Resolution (1080p50)
  videoFormatProps mixerFormat;
  mixerFormat.setVideoFormat = vF::HD1080_50p;
  m_lMixer->setVideoFormat(mixerFormat);

  // 2. Define Mixer Layers
  // Layer 0: File 1 (Full Screen)
  m_file1Layer = m_lMixer->addLayer("file1_layer", m_lFile1, 0, 0, 0, 255, 1920, 1080);
  if (m_file1Layer) {
    m_file1Layer->audio = true; // Use audio from File 1
  }

  // Layer 1: File 2 (Picture-in-Picture)
  m_file2Layer = m_lMixer->addLayer("file2_pip_layer", m_lFile2, 1, 1200, 100, 255, 640, 360);
  if (m_file2Layer) {
    m_file2Layer->audio = false; // Mute PiP layer to avoid audio clash
    m_file2Layer->borderColor = "#FFFFFF";
    m_file2Layer->borderSize = 5;
  }

  // Layer 2: Character Generator (CG) - Top Layer
  m_lMixer->addLayer("cg_layer", m_lCG, 2, 0, 0, 255, 1920, 1080);

  // 3. Configure Previews
  // File 1 Preview
  m_lPreviewFile1->setProps("ui_framework", "qt");
  m_lPreviewFile1->previewEnable(ui->file1Preview, false, true);
  m_lPreviewFile1->previewObject(m_lFile1);

  // File 2 Preview
  m_lPreviewFile2->setProps("ui_framework", "qt");
  m_lPreviewFile2->previewEnable(ui->file2Preview, false, true);
  m_lPreviewFile2->previewObject(m_lFile2);

  // Mixer Output Preview
  m_lPreviewMixer->setProps("ui_framework", "qt");
  m_lPreviewMixer->previewEnable(ui->mixerPreview, true, true);
  m_lPreviewMixer->previewObject(m_lMixer);

  // 4. Initial Values and Limits for UI Elements
  ui->pipXSlider->setMaximum(1920);
  ui->pipXSlider->setValue(1200);
  ui->xValLbl->setText("1200 px");

  ui->pipYSlider->setMaximum(1080);
  ui->pipYSlider->setValue(100);
  ui->yValLbl->setText("100 px");

  ui->pipWSlider->setMaximum(1920);
  ui->pipWSlider->setValue(640);
  ui->wValLbl->setText("640 px");

  ui->pipHSlider->setMaximum(1080);
  ui->pipHSlider->setValue(360);
  ui->hValLbl->setText("360 px");

  ui->pipAlphaSlider->setMaximum(255);
  ui->pipAlphaSlider->setValue(255);
  ui->aValLbl->setText("255");

  // 5. Signal / Slot Connections
  // File 1 Controls
  connect(ui->openFile1Btn, &QPushButton::clicked, this, &MainWindow::onOpenFile1);
  connect(ui->playFile1Btn, &QPushButton::clicked, this, &MainWindow::onPlayFile1);
  connect(ui->stopFile1Btn, &QPushButton::clicked, this, &MainWindow::onStopFile1);

  // File 2 Controls
  connect(ui->openFile2Btn, &QPushButton::clicked, this, &MainWindow::onOpenFile2);
  connect(ui->playFile2Btn, &QPushButton::clicked, this, &MainWindow::onPlayFile2);
  connect(ui->stopFile2Btn, &QPushButton::clicked, this, &MainWindow::onStopFile2);

  // PiP Layout Controls
  connect(ui->pipXSlider, &QSlider::valueChanged, this, &MainWindow::onPipLayoutChanged);
  connect(ui->pipYSlider, &QSlider::valueChanged, this, &MainWindow::onPipLayoutChanged);
  connect(ui->pipWSlider, &QSlider::valueChanged, this, &MainWindow::onPipLayoutChanged);
  connect(ui->pipHSlider, &QSlider::valueChanged, this, &MainWindow::onPipLayoutChanged);
  connect(ui->pipAlphaSlider, &QSlider::valueChanged, this, &MainWindow::onPipLayoutChanged);

  // CG Buttons
  connect(ui->addLogoBtn, &QPushButton::clicked, this, &MainWindow::onAddLogo);
  connect(ui->addLowerThirdBtn, &QPushButton::clicked, this, &MainWindow::onAddLowerThird);
  connect(ui->addTickerBtn, &QPushButton::clicked, this, &MainWindow::onAddTicker);
  connect(ui->clearCgBtn, &QPushButton::clicked, this, &MainWindow::onClearCg);

  // UI Update Timer
  connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::updateUI);
  m_uiTimer->start(50);
}

// ─── Destructor ──────────────────────────────────────────────────────────────
MainWindow::~MainWindow() {
  if (m_lFile1) m_lFile1->stop();
  if (m_lFile2) m_lFile2->stop();

  delete m_lPreviewFile1;
  delete m_lPreviewFile2;
  delete m_lPreviewMixer;

  delete m_lFile1;
  delete m_lFile2;
  delete m_lMixer;
  delete m_lCG;
  delete ui;
}

// ─── File 1 Controls ─────────────────────────────────────────────────────────
void MainWindow::onOpenFile1() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Select Media File for Layer 1 (Full Screen)", "",
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
      this, "Select Media File for Layer 2 (Picture-in-Picture)", "",
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

// ─── PiP Position & Size Update ──────────────────────────────────────────────
void MainWindow::onPipLayoutChanged() {
  int x = ui->pipXSlider->value();
  int y = ui->pipYSlider->value();
  int w = ui->pipWSlider->value();
  int h = ui->pipHSlider->value();
  int alpha = ui->pipAlphaSlider->value();

  // Update labels
  ui->xValLbl->setText(QString("%1 px").arg(x));
  ui->yValLbl->setText(QString("%1 px").arg(y));
  ui->wValLbl->setText(QString("%1 px").arg(w));
  ui->hValLbl->setText(QString("%1 px").arg(h));
  ui->aValLbl->setText(QString("%1").arg(alpha));

  // Update layer properties
  if (m_file2Layer) {
    m_file2Layer->posX = x;
    m_file2Layer->posY = y;
    m_file2Layer->width = w;
    m_file2Layer->height = h;
    m_file2Layer->alpha = alpha;
  }
}

// ─── CG Overlay (Logo) Addition ──────────────────────────────────────────────
void MainWindow::onAddLogo() {
  m_lCG->remove("logo_bg");
  m_lCG->remove("logo_text");

  // 1. Logo Background Rectangle (Eco Style)
  LCGRectProps bgProps;
  bgProps.bgType = "Solid";
  bgProps.bgColor = "#ff5500";
  bgProps.bgAlpha = 220;
  bgProps.bgRadius = 15;
  bgProps.borderColor = "#ffffff";
  bgProps.borderSize = 2;
  bgProps.borderAlpha = 255;
  m_lCG->addItem("logo_bg", 80, 80, 240, 70, bgProps);

  // 2. Logo Text
  LCGTextProps txtProps;
  txtProps.text = "LML MIXER";
  txtProps.font = "Arial";
  txtProps.fontSize = 32;
  txtProps.color = "#ffffff";
  txtProps.hAlign = "Center";
  txtProps.vAlign = "Center";
  m_lCG->addItem("logo_text", 80, 80, 240, 70, txtProps);
}

// ─── CG Overlay (Lower Third) Addition ───────────────────────────────────────
void MainWindow::onAddLowerThird() {
  m_lCG->remove("lt_bg");
  m_lCG->remove("lt_title");
  m_lCG->remove("lt_subtitle");

  // 1. Lower Third Background (Modern Dark Gradient Design)
  LCGRectProps bgProps;
  bgProps.bgType = "Gradient";
  bgProps.bgGradientStartColor = "#0055ff";
  bgProps.bgGradientEndColor = "#0a0a1a";
  bgProps.bgAlpha = 240;
  bgProps.bgRadius = 8;
  m_lCG->addItem("lt_bg", 150, 800, 700, 120, bgProps);

  // 2. Title Text (Name / Surname)
  LCGTextProps titleProps;
  titleProps.text = "ALİ SABER";
  titleProps.font = "Courier New";
  titleProps.fontSize = 36;
  titleProps.color = "#ffffff";
  titleProps.hAlign = "Left";
  titleProps.vAlign = "Center";
  titleProps.padding = 20;
  m_lCG->addItem("lt_title", 150, 800, 700, 70, titleProps);

  // 3. Subtitle Text (Title / Description)
  LCGTextProps subProps;
  subProps.text = "Lead Broadcast Engineer - LML Developer";
  subProps.font = "Courier New";
  subProps.fontSize = 22;
  subProps.color = "#55aaff";
  subProps.hAlign = "Left";
  subProps.vAlign = "Center";
  subProps.padding = 20;
  m_lCG->addItem("lt_subtitle", 150, 860, 700, 50, subProps);
}

// ─── CG Overlay (Scrolling Ticker / Crawl) Addition ──────────────────────────
void MainWindow::onAddTicker() {
  m_lCG->remove("ticker_craw");

  // Crawl Ticker Properties
  LCGCrawlProps crawl;
  crawl.text = "::: LINUXMEDIALIBRARY MULTI-LAYER VIDEO MIXER DEMO ::: PLAYING 2 CHANNELS OF SIMULTANEOUS HIGH-DEFINITION MEDIA CHANNELS + HARDWARE-ACCELERATED PREVIEWS + TRANSPARENT VECTOR GRAPHICS GENERATION ON INTEL/AMD/NVIDIA GPU ARCHITECTURES! :::";
  crawl.font = "Arial";
  crawl.fontSize = 30;
  crawl.color = "#ffff00";
  crawl.textAlpha = 255;
  crawl.bgType = "Color";
  crawl.bgColor = "#111111";
  crawl.bgAlpha = 220;
  crawl.textMove = "Horizontal";
  crawl.textSpeed = -4.0f; // Scroll speed to the left

  m_lCG->addItem("ticker_craw1", 0, 980, 1920, 80, crawl);
}

// ─── Clear All CG Overlays ───────────────────────────────────────────────────
void MainWindow::onClearCg() {
  m_lCG->remove("logo_bg");
  m_lCG->remove("logo_text");
  m_lCG->remove("lt_bg");
  m_lCG->remove("lt_title");
  m_lCG->remove("lt_subtitle");
  m_lCG->remove("ticker_craw");
  m_lCG->remove("ticker_craw1");
  m_lCG->remove("ticker_craw2");
  m_lCG->remove("ticker_craw3");
  m_lCG->remove("ticker_craw4");
  m_lCG->remove("ticker_craw5");
  m_lCG->remove("ticker_craw6");
}

// ─── UI / Preview Update ─────────────────────────────────────────────────────
void MainWindow::updateUI() {
  // Trigger update on preview windows
  ui->file1Preview->update();
  ui->file2Preview->update();
  ui->mixerPreview->update();
}
