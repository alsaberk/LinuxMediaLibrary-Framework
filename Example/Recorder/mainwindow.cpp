#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <iomanip>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_lLive(new LLive()),
      m_lPreview(new LPreview()),
      m_lRecorder(new LRecorder()),
      m_statsTimer(new QTimer(this)),
      m_outputDir("/home/alsaberk/Development/LinuxMediaLibrary"),
      m_isRecording(false),
      m_isPaused(false)
{
    ui->setupUi(this);

    // Default parameters for live input
    m_lLive->setProps("gpu", "true");
    m_lLive->setProps("timecode", "true");
    m_lLive->setProps("timecodeSource", "0");

    // Connect preview EXACTLY in correct order: previewEnable first, then previewObject!
    m_lPreview->previewEnable(ui->livePreview, true, true);
    m_lPreview->previewObject(m_lLive);
    m_lPreview->setProps("audio_meter", "true");
    m_lPreview->setProps("maintain_ar", "true");
    m_lPreview->setProps("timecode.preview", "true");

    // Populate Dynamic Containers
    std::vector<std::string> containers = LRecorder::getAvailableContainers();
    for (const std::string& container : containers) {
        ui->containerCombo->addItem(QString::fromStdString(container));
    }
    int mp4Index = ui->containerCombo->findText("mp4", Qt::MatchExactly);
    if (mp4Index != -1) {
        ui->containerCombo->setCurrentIndex(mp4Index);
    }

    // Populate Standard Video Formats Combo with direct enum data
    ui->standardVideoFormatCombo->addItem("HD 720p 50", static_cast<int>(vF::HD720_50p));
    ui->standardVideoFormatCombo->addItem("HD 720p 59.94", static_cast<int>(vF::HD720_5994p));
    ui->standardVideoFormatCombo->addItem("HD 720p 60", static_cast<int>(vF::HD720_60p));
    ui->standardVideoFormatCombo->addItem("HD 1080p 23.98", static_cast<int>(vF::HD1080_2398p));
    ui->standardVideoFormatCombo->addItem("HD 1080p 24", static_cast<int>(vF::HD1080_24p));
    ui->standardVideoFormatCombo->addItem("HD 1080p 25", static_cast<int>(vF::HD1080_25p));
    ui->standardVideoFormatCombo->addItem("HD 1080p 29.97", static_cast<int>(vF::HD1080_2997p));
    ui->standardVideoFormatCombo->addItem("HD 1080p 30", static_cast<int>(vF::HD1080_30p));
    ui->standardVideoFormatCombo->addItem("HD 1080i 50 (Interlaced)", static_cast<int>(vF::HD1080_50i));
    ui->standardVideoFormatCombo->addItem("HD 1080p 50", static_cast<int>(vF::HD1080_50p));
    ui->standardVideoFormatCombo->addItem("HD 1080i 59.94 (Interlaced)", static_cast<int>(vF::HD1080_5994i));
    ui->standardVideoFormatCombo->addItem("HD 1080p 59.94", static_cast<int>(vF::HD1080_5994p));
    ui->standardVideoFormatCombo->addItem("HD 1080i 60 (Interlaced)", static_cast<int>(vF::HD1080_60i));
    ui->standardVideoFormatCombo->addItem("HD 1080p 60", static_cast<int>(vF::HD1080_60p));
    ui->standardVideoFormatCombo->addItem("2K DCI 23.98p", static_cast<int>(vF::_2K_DCI_2398p));
    ui->standardVideoFormatCombo->addItem("2K DCI 24p", static_cast<int>(vF::_2K_DCI_24p));
    ui->standardVideoFormatCombo->addItem("2K DCI 25p", static_cast<int>(vF::_2K_DCI_25p));
    ui->standardVideoFormatCombo->addItem("2K DCI 50p", static_cast<int>(vF::_2K_DCI_50p));
    ui->standardVideoFormatCombo->addItem("2K DCI 60p", static_cast<int>(vF::_2K_DCI_60p));
    ui->standardVideoFormatCombo->addItem("4K UHD 23.98p", static_cast<int>(vF::_4K_UHD_2398p));
    ui->standardVideoFormatCombo->addItem("4K UHD 24p", static_cast<int>(vF::_4K_UHD_24p));
    ui->standardVideoFormatCombo->addItem("4K UHD 25p", static_cast<int>(vF::_4K_UHD_25p));
    ui->standardVideoFormatCombo->addItem("4K UHD 29.97p", static_cast<int>(vF::_4K_UHD_2997p));
    ui->standardVideoFormatCombo->addItem("4K UHD 30p", static_cast<int>(vF::_4K_UHD_30p));
    ui->standardVideoFormatCombo->addItem("4K UHD 50i (Interlaced)", static_cast<int>(vF::_4K_UHD_50i));
    ui->standardVideoFormatCombo->addItem("4K UHD 50p", static_cast<int>(vF::_4K_UHD_50p));
    ui->standardVideoFormatCombo->addItem("4K UHD 59.94i (Interlaced)", static_cast<int>(vF::_4K_UHD_5994i));
    ui->standardVideoFormatCombo->addItem("4K UHD 59.94p", static_cast<int>(vF::_4K_UHD_5994p));
    ui->standardVideoFormatCombo->addItem("4K UHD 60i (Interlaced)", static_cast<int>(vF::_4K_UHD_60i));
    ui->standardVideoFormatCombo->addItem("4K UHD 60p", static_cast<int>(vF::_4K_UHD_60p));
    ui->standardVideoFormatCombo->addItem("NTSC 486i (525i59.94)", static_cast<int>(vF::_525i_5994));
    ui->standardVideoFormatCombo->addItem("PAL 576i (625i50)", static_cast<int>(vF::_625i_50));
    ui->standardVideoFormatCombo->addItem("4K DCI 23.98p", static_cast<int>(vF::_4K_DCI_2398p));
    ui->standardVideoFormatCombo->addItem("4K DCI 24p", static_cast<int>(vF::_4K_DCI_24p));
    ui->standardVideoFormatCombo->addItem("4K DCI 25p", static_cast<int>(vF::_4K_DCI_25p));
    ui->standardVideoFormatCombo->addItem("4K DCI 50p", static_cast<int>(vF::_4K_DCI_50p));
    ui->standardVideoFormatCombo->addItem("4K DCI 60p", static_cast<int>(vF::_4K_DCI_60p));
    ui->standardVideoFormatCombo->setCurrentIndex(9); // HD 1080p 50 default

    // Populate Standard Audio Formats Combo with direct enum data
    ui->standardAudioFormatCombo->addItem("16-Bit / 11kHz / 2-Ch", static_cast<int>(aF::_16B_11K_2CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 22kHz / 2-Ch", static_cast<int>(aF::_16B_22K_2CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 44.1kHz / 2-Ch", static_cast<int>(aF::_16B_44K_2CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 44.1kHz / 6-Ch (5.1)", static_cast<int>(aF::_16B_44K_6CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 48kHz / 2-Ch", static_cast<int>(aF::_16B_48K_2CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 48kHz / 6-Ch (5.1)", static_cast<int>(aF::_16B_48K_6CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 48kHz / 8-Ch", static_cast<int>(aF::_16B_48K_8CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 48kHz / 16-Ch", static_cast<int>(aF::_16B_48K_16CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 96kHz / 2-Ch", static_cast<int>(aF::_16B_96K_2CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 96kHz / 6-Ch (5.1)", static_cast<int>(aF::_16B_96K_6CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 96kHz / 8-Ch", static_cast<int>(aF::_16B_96K_8CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 192kHz / 2-Ch", static_cast<int>(aF::_16B_192K_2CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 192kHz / 6-Ch (5.1)", static_cast<int>(aF::_16B_192K_6CH));
    ui->standardAudioFormatCombo->addItem("16-Bit / 192kHz / 8-Ch", static_cast<int>(aF::_16B_192K_8CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 44.1kHz / 2-Ch", static_cast<int>(aF::_24B_44K_2CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 48kHz / 2-Ch", static_cast<int>(aF::_24B_48K_2CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 48kHz / 6-Ch (5.1)", static_cast<int>(aF::_24B_48K_6CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 48kHz / 8-Ch", static_cast<int>(aF::_24B_48K_8CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 48kHz / 16-Ch", static_cast<int>(aF::_24B_48K_16CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 96kHz / 2-Ch", static_cast<int>(aF::_24B_96K_2CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 96kHz / 6-Ch (5.1)", static_cast<int>(aF::_24B_96K_6CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 96kHz / 8-Ch", static_cast<int>(aF::_24B_96K_8CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 192kHz / 2-Ch", static_cast<int>(aF::_24B_192K_2CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 192kHz / 6-Ch (5.1)", static_cast<int>(aF::_24B_192K_6CH));
    ui->standardAudioFormatCombo->addItem("24-Bit / 192kHz / 8-Ch", static_cast<int>(aF::_24B_192K_8CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 44.1kHz / 2-Ch", static_cast<int>(aF::_32F_44K_2CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 48kHz / 2-Ch", static_cast<int>(aF::_32F_48K_2CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 48kHz / 6-Ch (5.1)", static_cast<int>(aF::_32F_48K_6CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 48kHz / 8-Ch", static_cast<int>(aF::_32F_48K_8CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 48kHz / 16-Ch", static_cast<int>(aF::_32F_48K_16CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 96kHz / 2-Ch", static_cast<int>(aF::_32F_96K_2CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 96kHz / 6-Ch (5.1)", static_cast<int>(aF::_32F_96K_6CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 96kHz / 8-Ch", static_cast<int>(aF::_32F_96K_8CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 192kHz / 2-Ch", static_cast<int>(aF::_32F_192K_2CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 192kHz / 6-Ch (5.1)", static_cast<int>(aF::_32F_192K_6CH));
    ui->standardAudioFormatCombo->addItem("32-Bit Float / 192kHz / 8-Ch", static_cast<int>(aF::_32F_192K_8CH));
    ui->standardAudioFormatCombo->setCurrentIndex(4); // 16-Bit / 48kHz / 2-Ch default

    // Populate Custom Resolutions
    ui->customResCombo->addItem("1920x1080 (Full HD)", "1920x1080");
    ui->customResCombo->addItem("1280x720 (HD)", "1280x720");
    ui->customResCombo->addItem("2048x1080 (2K DCI)", "2048x1080");
    ui->customResCombo->addItem("3840x2160 (4K UHD)", "3840x2160");
    ui->customResCombo->addItem("4096x2160 (4K DCI)", "4096x2160");
    ui->customResCombo->addItem("720x576 (PAL)", "720x576");
    ui->customResCombo->addItem("720x480 (NTSC)", "720x480");
    ui->customResCombo->setCurrentIndex(0);

    // Populate Custom FPS
    ui->customFpsCombo->addItem("25 fps", 25.0);
    ui->customFpsCombo->addItem("29.97 fps", 29.97);
    ui->customFpsCombo->addItem("30 fps", 30.0);
    ui->customFpsCombo->addItem("50 fps", 50.0);
    ui->customFpsCombo->addItem("59.94 fps", 59.94);
    ui->customFpsCombo->addItem("60 fps", 60.0);
    ui->customFpsCombo->setCurrentIndex(3); // 50 fps default

    // Populate Custom Audio Sample Rate
    ui->customSampleRateCombo->addItem("48000 Hz (Broadcast)", 48000);
    ui->customSampleRateCombo->addItem("44100 Hz (CD Quality)", 44100);
    ui->customSampleRateCombo->addItem("96000 Hz (High-Res)", 96000);
    ui->customSampleRateCombo->addItem("192000 Hz (Studio)", 192000);
    ui->customSampleRateCombo->addItem("22050 Hz (Low)", 22050);
    ui->customSampleRateCombo->addItem("11025 Hz (Phone)", 11025);
    ui->customSampleRateCombo->setCurrentIndex(0);

    // Populate Custom Audio Channels
    ui->customChannelsCombo->addItem("2 channels (Stereo)", 2);
    ui->customChannelsCombo->addItem("6 channels (5.1 Surround)", 6);
    ui->customChannelsCombo->addItem("8 channels", 8);
    ui->customChannelsCombo->addItem("16 channels (Broadcast)", 16);
    ui->customChannelsCombo->setCurrentIndex(0);

    // Populate Video/Audio Bitrates
    ui->videoBitrateCombo->addItem("2 Mbps (Mobile)", 2000);
    ui->videoBitrateCombo->addItem("5 Mbps (SD/HD Standard)", 5000);
    ui->videoBitrateCombo->addItem("10 Mbps (HD Good)", 10000);
    ui->videoBitrateCombo->addItem("15 Mbps (HD Broadcast)", 15000);
    ui->videoBitrateCombo->addItem("20 Mbps (HD Premium)", 20000);
    ui->videoBitrateCombo->setCurrentIndex(2); // 10 Mbps default

    ui->audioBitrateCombo->addItem("128 kbps", 128);
    ui->audioBitrateCombo->addItem("192 kbps", 192);
    ui->audioBitrateCombo->addItem("256 kbps", 256);
    ui->audioBitrateCombo->addItem("320 kbps (Broadcast)", 320);
    ui->audioBitrateCombo->setCurrentIndex(1); // 192 kbps default

    // Initial folder setting
    ui->outputFolderLbl->setText(m_outputDir);

    // Live Device connections
    connect(ui->liveBtn, &QPushButton::clicked, this, &MainWindow::onLiveClicked);
    connect(ui->deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDeviceChanged);
    connect(ui->deviceChannelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onChannelChanged);
    connect(ui->deviceFormatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFormatChanged);

    // Mode connections (Standard vs Custom)
    connect(ui->modeStandardRadio, &QRadioButton::toggled, this, &MainWindow::onModeToggled);
    connect(ui->modeCustomRadio, &QRadioButton::toggled, this, &MainWindow::onModeToggled);

    // Container changed connection
    connect(ui->containerCombo, &QComboBox::currentTextChanged, this, &MainWindow::onContainerChanged);

    // Folder selection
    connect(ui->outputFolderBtn, &QPushButton::clicked, this, &MainWindow::onSelectFolderClicked);

    // Recording Controls
    connect(ui->startRecBtn, &QPushButton::clicked, this, &MainWindow::onStartRecordingClicked);
    connect(ui->pauseRecBtn, &QPushButton::clicked, this, &MainWindow::onPauseRecordingClicked);
    connect(ui->stopRecBtn, &QPushButton::clicked, this, &MainWindow::onStopRecordingClicked);

    // Stats Timer
    connect(m_statsTimer, &QTimer::timeout, this, &MainWindow::updateUIStats);

    // Real-time dynamic Recorder synchronization connections
    connect(ui->standardVideoFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->standardAudioFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->customResCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->customFpsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->customSampleRateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->customChannelsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->videoCodecCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->audioCodecCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->videoBitrateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->audioBitrateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyRecorderSettings);
    connect(ui->fileNameEdit, &QLineEdit::textChanged, this, &MainWindow::applyRecorderSettings);

    // Initial load
    refreshLiveSources();
    if (ui->containerCombo->count() > 0) {
        onContainerChanged(ui->containerCombo->currentText());
    }

    // Establish initial visibility of Standard vs Custom options
    onModeToggled();
}

MainWindow::~MainWindow() {
    if (m_isRecording) {
        m_lRecorder->Stop();
    }
    if (m_lLive) {
        m_lLive->Stop();
    }
    delete m_statsTimer;
    delete m_lRecorder;
    delete m_lPreview;
    delete m_lLive;
    delete ui;
}

void MainWindow::refreshLiveSources() {
    ui->deviceComboBox->blockSignals(true);
    ui->deviceComboBox->clear();
    int count = 0;
    m_lLive->DeviceGetCount(count);
    for (int i = 0; i < count; i++) {
        std::string name, desc;
        m_lLive->DeviceGetByIndex(i, name, desc);
        ui->deviceComboBox->addItem(QString::fromStdString(name));
    }
    ui->deviceComboBox->blockSignals(false);
    if (count > 0) {
        ui->deviceComboBox->setCurrentIndex(0);
        onDeviceChanged(0);
    }
}

void MainWindow::onDeviceChanged(int index) {
    if (index < 0) return;

    int status = 0;
    m_lLive->statusGet(status);
    bool wasRunning = (status == 1);
    if (wasRunning) m_lLive->Stop();

    m_lLive->DeviceSet(index);

    ui->deviceChannelComboBox->blockSignals(true);
    ui->deviceChannelComboBox->clear();
    int count = 0;
    m_lLive->DeviceChannelGetCount(index, count);
    for (int i = 0; i < count; i++) {
        std::string name, desc;
        m_lLive->DeviceChannelGetByIndex(index, i, name, desc);
        ui->deviceChannelComboBox->addItem(QString::fromStdString(name));
    }
    ui->deviceChannelComboBox->blockSignals(false);
    if (count > 0) {
        ui->deviceChannelComboBox->setCurrentIndex(0);
        onChannelChanged(0);
    }

    if (wasRunning) m_lLive->Start();
}

void MainWindow::onChannelChanged(int index) {
    if (index < 0) return;
    int deviceIdx = ui->deviceComboBox->currentIndex();

    int status = 0;
    m_lLive->statusGet(status);
    bool wasRunning = (status == 1);
    if (wasRunning) m_lLive->Stop();

    m_lLive->DeviceChannelSet(index);

    ui->deviceFormatComboBox->blockSignals(true);
    ui->deviceFormatComboBox->clear();
    int count = 0;
    m_lLive->DeviceFormatVideoGetCount(deviceIdx, index, count);
    for (int i = 0; i < count; ++i) {
        std::string name;
        videoFormatProps props;
        m_lLive->DeviceFormatVideoGetByIndex(deviceIdx, index, i, name, props);
        ui->deviceFormatComboBox->addItem(QString::fromStdString(name), i);
    }
    ui->deviceFormatComboBox->blockSignals(false);
    if (count > 0) {
        ui->deviceFormatComboBox->setCurrentIndex(0);
        onFormatChanged(0);
    }

    if (wasRunning) m_lLive->Start();
}

void MainWindow::onFormatChanged(int index) {
    if (index < 0) return;
    int status = 0;
    m_lLive->statusGet(status);
    bool wasRunning = (status == 1);
    if (wasRunning) m_lLive->Stop();
    m_lLive->DeviceFormatVideoSet(index);
    if (wasRunning) m_lLive->Start();
}

void MainWindow::onLiveClicked() {
    int status = 0;
    m_lLive->statusGet(status);
    bool isLive = (status == 1);

    if (!isLive) {
        int formatIdx = ui->deviceFormatComboBox->currentData().toInt();
        m_lLive->DeviceFormatVideoSet(formatIdx);
        if (m_lLive->Start()) {
            ui->liveBtn->setText("STOP LIVE INPUT");
            ui->liveBtn->setStyleSheet("background-color: #dc2626; color: white; font-weight: bold;");
        }
    } else {
        m_lLive->Stop();
        ui->liveBtn->setText("START LIVE INPUT");
        ui->liveBtn->setStyleSheet("background-color: #16a34a; color: white; font-weight: bold;");
    }
}

void MainWindow::onSelectFolderClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", m_outputDir);
    if (!dir.isEmpty()) {
        m_outputDir = dir;
        ui->outputFolderLbl->setText(m_outputDir);
        applyRecorderSettings();
    }
}

void MainWindow::onModeToggled() {
    bool isStandard = ui->modeStandardRadio->isChecked();

    // Show/hide standard format selectors and their labels
    ui->standardVideoFormatLbl->setVisible(isStandard);
    ui->standardVideoFormatCombo->setVisible(isStandard);
    ui->standardAudioFormatLbl->setVisible(isStandard);
    ui->standardAudioFormatCombo->setVisible(isStandard);

    // Show/hide custom property selectors and their labels
    ui->customResLbl->setVisible(!isStandard);
    ui->customResCombo->setVisible(!isStandard);
    ui->customFpsLbl->setVisible(!isStandard);
    ui->customFpsCombo->setVisible(!isStandard);

    // Show/hide custom audio parameters
    ui->customSampleRateLbl->setVisible(!isStandard);
    ui->customSampleRateCombo->setVisible(!isStandard);
    ui->customChannelsLbl->setVisible(!isStandard);
    ui->customChannelsCombo->setVisible(!isStandard);

    // Explicitly enable/disable controls based on mode so they do not stay disabled!
    ui->customResCombo->setEnabled(!isStandard);
    ui->customFpsCombo->setEnabled(!isStandard);
    ui->customSampleRateCombo->setEnabled(!isStandard);
    ui->customChannelsCombo->setEnabled(!isStandard);

    applyRecorderSettings();
}

void MainWindow::onContainerChanged(const QString& container) {
    ui->videoCodecCombo->blockSignals(true);
    ui->audioCodecCombo->blockSignals(true);

    ui->videoCodecCombo->clear();
    ui->audioCodecCombo->clear();

    std::string containerStr = container.toLower().toStdString();
    std::vector<std::string> videoCodecs = LRecorder::getAvailableVideoCodecs(containerStr);
    std::vector<std::string> audioCodecs = LRecorder::getAvailableAudioCodecs(containerStr);

    for (const std::string& codec : videoCodecs) {
        ui->videoCodecCombo->addItem(QString::fromStdString(codec));
    }
    for (const std::string& codec : audioCodecs) {
        ui->audioCodecCombo->addItem(QString::fromStdString(codec));
    }

    // Set libx264 as default video codec if found, otherwise fallback to general h264/264
    int h264Index = ui->videoCodecCombo->findText("libx264", Qt::MatchExactly);
    if (h264Index == -1) h264Index = ui->videoCodecCombo->findText("libx264", Qt::MatchContains);
    if (h264Index == -1) h264Index = ui->videoCodecCombo->findText("h264", Qt::MatchContains);
    if (h264Index == -1) h264Index = ui->videoCodecCombo->findText("264", Qt::MatchContains);
    if (h264Index != -1) {
        ui->videoCodecCombo->setCurrentIndex(h264Index);
    }

    // Set aac as default audio codec if found
    int aacIndex = ui->audioCodecCombo->findText("aac", Qt::MatchContains);
    if (aacIndex != -1) {
        ui->audioCodecCombo->setCurrentIndex(aacIndex);
    }

    ui->videoCodecCombo->blockSignals(false);
    ui->audioCodecCombo->blockSignals(false);

    applyRecorderSettings();
}

void MainWindow::applyRecorderSettings() {
    if (m_isRecording) return; // Do not alter properties during active recording

    videoFormatProps vProps;
    audioFormatProps aProps;

    if (ui->modeStandardRadio->isChecked()) {
        // Standard Mode: directly bind selected format value
        vProps.setVideoFormat = static_cast<vF_Type>(ui->standardVideoFormatCombo->currentData().toInt());
        aProps.setAudioFormat = static_cast<aF_Type>(ui->standardAudioFormatCombo->currentData().toInt());
    } else {
        // Custom Mode: set Custom and configure resolution/fps
        vProps.setVideoFormat = vF::Custom;
        
        QString resStr = ui->customResCombo->currentData().toString();
        QStringList parts = resStr.split('x');
        if (parts.size() == 2) {
            vProps.width = parts[0].toInt();
            vProps.height = parts[1].toInt();
        } else {
            vProps.width = 1920;
            vProps.height = 1080;
        }
        vProps.fps = ui->customFpsCombo->currentData().toDouble();

        // Custom Audio Properties
        aProps.setAudioFormat = aF::Custom;
        aProps.sampleRate = ui->customSampleRateCombo->currentData().toInt();
        aProps.channels = ui->customChannelsCombo->currentData().toInt();
        aProps.bitsPerSample = 16;
    }

    m_lRecorder->setVideoFormat(vProps);
    m_lRecorder->setAudioFormat(aProps);

    // Set general recorder specifications
    m_lRecorder->setProps("gpu", "true");
    m_lRecorder->setVideoBitrate(ui->videoBitrateCombo->currentData().toInt());
    m_lRecorder->setAudioBitrate(ui->audioBitrateCombo->currentData().toInt());

    std::string container = ui->containerCombo->currentText().toLower().toStdString();
    m_lRecorder->setContainer(container);

    std::string videoCodec = ui->videoCodecCombo->currentText().toStdString();
    std::string audioCodec = ui->audioCodecCombo->currentText().toStdString();
    m_lRecorder->setVideoCodec(videoCodec);
    m_lRecorder->setAudioCodec(audioCodec);

    // Complete target file path
    QString fileName = ui->fileNameEdit->text();
    QString fullPath = m_outputDir + "/" + fileName + "." + QString::fromStdString(container);
    m_lRecorder->setFilePath(fullPath.toStdString());

    // Connect Live source object
    m_lRecorder->recordObject(m_lLive);
}

void MainWindow::onStartRecordingClicked() {
    if (m_isRecording) return;

    // Apply the absolute latest settings to be safe
    applyRecorderSettings();

    if (m_lRecorder->Record()) {
        m_isRecording = true;
        m_isPaused = false;

        // UI states during active recording
        ui->startRecBtn->setEnabled(false);
        ui->startRecBtn->setText("⚫ RECORDING...");
        ui->startRecBtn->setStyleSheet("background-color: #991b1b; color: white; font-weight: bold;");

        ui->pauseRecBtn->setEnabled(true);
        ui->pauseRecBtn->setText("⏸ PAUSE");

        ui->stopRecBtn->setEnabled(true);
        ui->stopRecBtn->setStyleSheet("background-color: #dc2626; color: white; font-weight: bold;");

        // Disable setting edits during active record
        ui->destGroup->setEnabled(false);
        ui->encodingGroup->setEnabled(false);
        ui->liveInputGroup->setEnabled(false);

        // Start updates
        m_statsTimer->start(200);

        std::string container = ui->containerCombo->currentText().toLower().toStdString();
        std::string videoCodec = ui->videoCodecCombo->currentText().toStdString();
        std::string audioCodec = ui->audioCodecCombo->currentText().toStdString();
        QString fileName = ui->fileNameEdit->text();

        ui->statsStatusVal->setText(QString("RECORDING (%1 + %2) -> %3")
                                    .arg(QString::fromStdString(videoCodec))
                                    .arg(QString::fromStdString(audioCodec))
                                    .arg(fileName + "." + QString::fromStdString(container)));
    } else {
        QMessageBox::critical(this, "Recording Error", "Failed to initialize FFmpeg recording engine! Please check container and codec compatibility.");
    }
}

void MainWindow::onPauseRecordingClicked() {
    if (!m_isRecording) return;

    if (!m_isPaused) {
        m_lRecorder->Pause(true);
        m_isPaused = true;
        ui->pauseRecBtn->setText("▶ RESUME");
        ui->pauseRecBtn->setStyleSheet("background-color: #16a34a; color: white;");
        ui->startRecBtn->setText("⚫ RECORDING PAUSED");
        ui->statsStatusVal->setText("RECORDING PAUSED - Waiting for resume");
    } else {
        m_lRecorder->Pause(false);
        m_isPaused = false;
        ui->pauseRecBtn->setText("⏸ PAUSE");
        ui->pauseRecBtn->setStyleSheet("");
        ui->startRecBtn->setText("⚫ RECORDING...");
        std::string videoCodec = ui->videoCodecCombo->currentText().toStdString();
        std::string audioCodec = ui->audioCodecCombo->currentText().toStdString();
        ui->statsStatusVal->setText(QString("RECORDING (%1 + %2) - Active")
                                    .arg(QString::fromStdString(videoCodec))
                                    .arg(QString::fromStdString(audioCodec)));
    }
}

void MainWindow::onStopRecordingClicked() {
    if (!m_isRecording) return;

    m_statsTimer->stop();
    m_lRecorder->Stop();

    m_isRecording = false;
    m_isPaused = false;

    ui->startRecBtn->setEnabled(true);
    ui->startRecBtn->setText("⚫ START RECORDING");
    ui->startRecBtn->setStyleSheet("background-color: #dc2626; color: white;");

    ui->pauseRecBtn->setEnabled(false);
    ui->pauseRecBtn->setText("⏸ PAUSE");
    ui->pauseRecBtn->setStyleSheet("");

    ui->stopRecBtn->setEnabled(false);
    ui->stopRecBtn->setStyleSheet("background-color: #3f3f46;");

    // Enable settings again
    ui->destGroup->setEnabled(true);
    ui->encodingGroup->setEnabled(true);
    ui->liveInputGroup->setEnabled(true);

    ui->statsStatusVal->setText("IDLE - Ready to record");

    QMessageBox::information(this, "Recording Complete", "The recording was saved successfully!");
}

void MainWindow::updateUIStats() {
    if (!m_isRecording) return;

    LRecorder::Stats stats;
    m_lRecorder->getStats(stats);

    // Format Duration (HH:MM:SS.t)
    double durationSec = stats.durationWrittenMs / 1000.0;
    int hrs = static_cast<int>(durationSec) / 3600;
    int mins = (static_cast<int>(durationSec) % 3600) / 60;
    int secs = static_cast<int>(durationSec) % 60;
    int ms = static_cast<int>(stats.durationWrittenMs) % 1000;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hrs << ":"
       << std::setfill('0') << std::setw(2) << mins << ":"
       << std::setfill('0') << std::setw(2) << secs << "."
       << std::setfill('0') << std::setw(1) << (ms / 100);
    ui->statsDurationVal->setText(QString::fromStdString(ss.str()));

    // Format File Size
    double sizeMB = static_cast<double>(stats.fileSizeBytes) / (1024.0 * 1024.0);
    ui->statsSizeVal->setText(QString("%1 MB").arg(sizeMB, 0, 'f', 2));
}
