#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCoreApplication>

// ─── Constructor ─────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_lFile(new LFile()),
      m_lMixer(new LMixer()),
      m_lAnimation(new LAnimation()),
      m_lPreviewFile(new LPreview()),
      m_lPreviewProg(new LPreview()),
      m_uiTimer(new QTimer(this)),
      m_gameClockTimer(new QTimer(this))
{
  ui->setupUi(this);

  // 1. Configure Video Format (1080p50)
  videoFormatProps formatProps;
  formatProps.setVideoFormat = vF::HD1080_50p;
  m_lMixer->setVideoFormat(formatProps);
  m_lAnimation->setVideoFormat(formatProps);

  // 2. Add Background Layer (Video Player) - z-index 0
  m_lMixer->addLayer("background_layer", m_lFile, 0, 0.0f, 0.0f, 255, 1920, 1080);

  // 3. Add LAnimation Layer (Overlay CG Graphics) - z-index 1
  m_lMixer->addLayer("cg_layer", m_lAnimation, 1, 0.0f, 0.0f, 255, 1920, 1080);

  // 4. Configure Previews
  m_lPreviewFile->setProps("ui_framework", "qt");
  m_lPreviewFile->previewEnable(ui->videoPreview, false, true);
  m_lPreviewFile->previewObject(m_lFile);

  m_lPreviewProg->setProps("ui_framework", "qt");
  m_lPreviewProg->previewEnable(ui->programPreview, true, true); // Enable audio meters
  m_lPreviewProg->previewObject(m_lMixer);

  // 5. Connect UI Signals & Slots for Dynamic, Instant updates
  connect(ui->openVideoBtn, &QPushButton::clicked, this, &MainWindow::onOpenFile);
  connect(ui->playVideoBtn, &QPushButton::clicked, this, &MainWindow::onPlayVideo);
  connect(ui->stopVideoBtn, &QPushButton::clicked, this, &MainWindow::onStopVideo);

  connect(ui->startCgBtn, &QPushButton::clicked, this, &MainWindow::onStartCG);
  connect(ui->stopCgBtn, &QPushButton::clicked, this, &MainWindow::onStopCG);
  connect(ui->closeCgBtn, &QPushButton::clicked, this, &MainWindow::onCloseCG);

  connect(ui->startClockBtn, &QPushButton::clicked, this, &MainWindow::onToggleGameClock);
  connect(m_gameClockTimer, &QTimer::timeout, this, &MainWindow::onGameClockTick);

  // Dynamic Updates: whenever any field changes, immediately update CG
  connect(ui->homeNameEdit, &QLineEdit::textChanged, this, &MainWindow::onUpdateCGData);
  connect(ui->guestNameEdit, &QLineEdit::textChanged, this, &MainWindow::onUpdateCGData);
  connect(ui->homeScoreSb, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onUpdateCGData);
  connect(ui->guestScoreSb, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onUpdateCGData);
  connect(ui->updateCgBtn, &QPushButton::clicked, this, &MainWindow::onUpdateCGData);

  // Color Pickers
  connect(ui->homeBgColorBtn, &QPushButton::clicked, this, &MainWindow::onSelectHomeBgColor);
  connect(ui->homeJerseyColorBtn, &QPushButton::clicked, this, &MainWindow::onSelectHomeJerseyColor);
  connect(ui->guestBgColorBtn, &QPushButton::clicked, this, &MainWindow::onSelectGuestBgColor);
  connect(ui->guestJerseyColorBtn, &QPushButton::clicked, this, &MainWindow::onSelectGuestJerseyColor);

  // --- Lower Third CG Connections ---
  connect(ui->startLtBtn, &QPushButton::clicked, this, &MainWindow::onStartLT);
  connect(ui->stopLtBtn, &QPushButton::clicked, this, &MainWindow::onStopLT);
  connect(ui->closeLtBtn, &QPushButton::clicked, this, &MainWindow::onCloseLT);
  connect(ui->updateLtBtn, &QPushButton::clicked, this, &MainWindow::onUpdateLTData);

  connect(ui->ltUpBgBtn, &QPushButton::clicked, this, &MainWindow::onSelectLtUpBgColor);
  connect(ui->ltDownBgBtn, &QPushButton::clicked, this, &MainWindow::onSelectLtDownBgColor);

  // 6. Set initial color button styles
  updateColorButtonStyles();

  // 7. Start UI Update Timer (50ms)
  connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::updateUI);
  m_uiTimer->start(50);
}

// ─── Destructor ──────────────────────────────────────────────────────────────
MainWindow::~MainWindow() {
  if (m_lFile) m_lFile->stop();
  if (m_lAnimation) {
    m_lAnimation->stop(1);
    m_lAnimation->stop(2);
  }

  delete m_lPreviewFile;
  delete m_lPreviewProg;

  delete m_lFile;
  delete m_lMixer;
  delete m_lAnimation;
  delete ui;
}

// ─── Video File Handlers ─────────────────────────────────────────────────────
void MainWindow::onOpenFile() {
  QString filePath = QFileDialog::getOpenFileName(
      this, "Select Media File for Background", "",
      "Media Files (*.mp4 *.mkv *.avi *.mov *.mpg *.mpeg *.webm *.mxf *.ts)");
  if (!filePath.isEmpty()) {
    m_lFile->stop();
    m_lFile->fileNameSet(filePath.toStdString());
    m_lFile->play();
    ui->filePathLbl->setText(QFileInfo(filePath).fileName());
  }
}

void MainWindow::onPlayVideo() {
  m_lFile->play();
}

void MainWindow::onStopVideo() {
  m_lFile->stop();
}

// ─── Scoreboard CG Handlers ──────────────────────────────────────────────────
void MainWindow::onStartCG() {
  // 1. Try loading scoreboard.json from the copied template folder next to the executable
  QString templatePath = QCoreApplication::applicationDirPath() + "/template/scoreboard.json";
  
  if (!QFileInfo::exists(templatePath)) {
    // 2. Fallback to desktop lower.json
    templatePath = "/home/alsaberk/Masaüstü/lower.json";
  }
  
  if (!QFileInfo::exists(templatePath)) {
    // 3. Open file dialog for custom choice
    templatePath = QFileDialog::getOpenFileName(this, "Select Scoreboard Lottie Template (JSON)", "/home/alsaberk/Masaüstü", "Lottie Templates (*.json)");
  }

  if (!templatePath.isEmpty() && m_lAnimation->loadTemplate(1, templatePath.toStdString())) {
    m_cgActive = true; // Set active flag before initial push!
    m_lAnimation->play(1);
    // Push the initial scoreboard state to the overlay
    onUpdateCGData();
  } else if (!templatePath.isEmpty()) {
    QMessageBox::critical(this, "LAnimation Error", "Could not load template file:\n" + templatePath);
  }
}

void MainWindow::onStopCG() {
  m_cgActive = false;
  m_lAnimation->stop(1);
}

void MainWindow::onCloseCG() {
  m_cgActive = false;
  m_lAnimation->close(1);
}

// ─── Game Clock Handlers ─────────────────────────────────────────────────────
void MainWindow::onToggleGameClock() {
  if (!m_gameClockRunning) {
    m_gameClockTimer->start(1000); // 1-second tick
    m_gameClockRunning = true;
    ui->startClockBtn->setText("PAUSE CLOCK");
    ui->startClockBtn->setStyleSheet("background-color: #d97706; color: white;");
  } else {
    m_gameClockTimer->stop();
    m_gameClockRunning = false;
    ui->startClockBtn->setText("START CLOCK");
    ui->startClockBtn->setStyleSheet("background-color: #4f46e5; color: white;");
  }
}

void MainWindow::onGameClockTick() {
  m_elapsedSeconds++;
  ui->clockDisplayLbl->setText(formatTime(m_elapsedSeconds));
  
  if (m_cgActive) {
    // Real-time update to LAnimation lower-third overlay clock field
    m_lAnimation->update(1, "time", formatTime(m_elapsedSeconds).toStdString());
  }
}

// ─── Update Scoreboard Data ──────────────────────────────────────────────────
void MainWindow::onUpdateCGData() {
  if (!m_cgActive) return;

  QString homeName = ui->homeNameEdit->text();
  QString guestName = ui->guestNameEdit->text();
  int homeScore = ui->homeScoreSb->value();
  int guestScore = ui->guestScoreSb->value();

  // 1. Update text fields (supporting both 'homeclub'/'guestclub' and 'home'/'guest' naming conventions)
  m_lAnimation->update(1, "homeclub", homeName.toStdString());
  m_lAnimation->update(1, "home", homeName.toStdString());
  
  m_lAnimation->update(1, "guestclub", guestName.toStdString());
  m_lAnimation->update(1, "guest", guestName.toStdString());
  
  m_lAnimation->update(1, "homescore", std::to_string(homeScore));
  
  m_lAnimation->update(1, "guestscore", std::to_string(guestScore));

  // 2. Update dynamic team colors (both jersey colors and name backgrounds!)
  m_lAnimation->updateColor(1, "homecolor", m_homeJerseyColorHex.toStdString()); // Forma rengi
  m_lAnimation->updateColor(1, "homeBG", m_homeBgColorHex.toStdString());         // İsim arkaplanı
  
  m_lAnimation->updateColor(1, "guestcolor", m_guestJerseyColorHex.toStdString()); // Forma rengi
  m_lAnimation->updateColor(1, "guestBG", m_guestBgColorHex.toStdString());         // İsim arkaplanı
}

void MainWindow::onSelectHomeBgColor() {
  QColor color = QColorDialog::getColor(QColor(m_homeBgColorHex), this, "Choose Home BG Color");
  if (color.isValid()) {
    m_homeBgColorHex = color.name();
    updateColorButtonStyles();
    onUpdateCGData(); // Auto-update On-Air
  }
}

void MainWindow::onSelectHomeJerseyColor() {
  QColor color = QColorDialog::getColor(QColor(m_homeJerseyColorHex), this, "Choose Home Jersey Color");
  if (color.isValid()) {
    m_homeJerseyColorHex = color.name();
    updateColorButtonStyles();
    onUpdateCGData(); // Auto-update On-Air
  }
}

void MainWindow::onSelectGuestBgColor() {
  QColor color = QColorDialog::getColor(QColor(m_guestBgColorHex), this, "Choose Guest BG Color");
  if (color.isValid()) {
    m_guestBgColorHex = color.name();
    updateColorButtonStyles();
    onUpdateCGData(); // Auto-update On-Air
  }
}

void MainWindow::onSelectGuestJerseyColor() {
  QColor color = QColorDialog::getColor(QColor(m_guestJerseyColorHex), this, "Choose Guest Jersey Color");
  if (color.isValid()) {
    m_guestJerseyColorHex = color.name();
    updateColorButtonStyles();
    onUpdateCGData(); // Auto-update On-Air
  }
}

// ─── Helper Functions ────────────────────────────────────────────────────────
void MainWindow::updateColorButtonStyles() {
  ui->homeBgColorBtn->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold; border-radius: 6px;")
                                      .arg(m_homeBgColorHex));
  ui->homeJerseyColorBtn->setStyleSheet(QString("background-color: %1; color: black; font-weight: bold; border-radius: 6px;")
                                      .arg(m_homeJerseyColorHex));
  ui->guestBgColorBtn->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold; border-radius: 6px;")
                                       .arg(m_guestBgColorHex));
  ui->guestJerseyColorBtn->setStyleSheet(QString("background-color: %1; color: black; font-weight: bold; border-radius: 6px;")
                                       .arg(m_guestJerseyColorHex));

  // Style lower third buttons!
  ui->ltUpBgBtn->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold; border-radius: 6px;")
                                      .arg(m_ltUpBgColorHex));
  ui->ltDownBgBtn->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold; border-radius: 6px;")
                                      .arg(m_ltDownBgColorHex));
}

QString MainWindow::formatTime(int totalSeconds) {
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  return QString("%1:%2")
      .arg(minutes, 2, 10, QChar('0'))
      .arg(seconds, 2, 10, QChar('0'));
}

// ─── UI / Preview Update Loop ────────────────────────────────────────────────
void MainWindow::updateUI() {
  // Trigger update on preview windows
  ui->videoPreview->update();
  ui->programPreview->update();

  // Fetch LAnimation output frame statistics
  double fps = m_lAnimation->getFPS();
  ui->fpsValLbl->setText(QString::number(fps, 'f', 2));
}

// ─── Lower Third CG Handlers ──────────────────────────────────────────────────
void MainWindow::onStartLT() {
  QString templatePath = QCoreApplication::applicationDirPath() + "/template/lowerthird.json";
  
  if (!QFileInfo::exists(templatePath)) {
    templatePath = "/home/alsaberk/Masaüstü/lowerthird.json";
  }
  
  if (!QFileInfo::exists(templatePath)) {
    templatePath = QFileDialog::getOpenFileName(this, "Select Lower Third Lottie Template (JSON)", "/home/alsaberk/Masaüstü", "Lottie Templates (*.json)");
  }

  if (!templatePath.isEmpty() && m_lAnimation->loadTemplate(2, templatePath.toStdString())) {
    // Stop marker: 30, Out marker: 31 as requested by the user
    m_lAnimation->addMarker(2, "STOP", 30);
    m_lAnimation->addMarker(2, "OUT", 31);
    
    m_ltActive = true;
    m_lAnimation->play(2);
    onUpdateLTData(); // Initial data push
  } else if (!templatePath.isEmpty()) {
    QMessageBox::critical(this, "LAnimation Error", "Could not load Lower Third template:\n" + templatePath);
  }
}

void MainWindow::onStopLT() {
  m_ltActive = false;
  m_lAnimation->stop(2);
}

void MainWindow::onCloseLT() {
  m_ltActive = false;
  m_lAnimation->close(2);
}

void MainWindow::onUpdateLTData() {
  if (!m_ltActive) return;

  QString upText = ui->upTextEdit->text();
  QString downText = ui->downTextEdit->text();

  // Update text layers
  m_lAnimation->update(2, "upText", upText.toStdString());
  m_lAnimation->update(2, "downText", downText.toStdString());

  // Update color layers (upTextBG and downTextBG)
  m_lAnimation->updateColor(2, "upTextBG", m_ltUpBgColorHex.toStdString());
  m_lAnimation->updateColor(2, "downTextBG", m_ltDownBgColorHex.toStdString());
}

void MainWindow::onSelectLtUpBgColor() {
  QColor color = QColorDialog::getColor(QColor(m_ltUpBgColorHex), this, "Choose Upper BG Color");
  if (color.isValid()) {
    m_ltUpBgColorHex = color.name();
    updateColorButtonStyles();
  }
}

void MainWindow::onSelectLtDownBgColor() {
  QColor color = QColorDialog::getColor(QColor(m_ltDownBgColorHex), this, "Choose Lower BG Color");
  if (color.isValid()) {
    m_ltDownBgColorHex = color.name();
    updateColorButtonStyles();
  }
}
