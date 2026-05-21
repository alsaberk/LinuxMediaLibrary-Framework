#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    loadProfiles();

    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::loadProfiles() {
    // Yaygın Yayın Formatları
    QStringList profiles;
    profiles << "atsc_1080p_50" << "atsc_1080p_60" 
             << "atsc_1080i_50" << "atsc_1080i_60"
             << "atsc_720p_50"  << "atsc_720p_60"
             << "pal_dv"        << "ntsc_dv";
    
    ui->profileCombo->addItems(profiles);
}

void SettingsDialog::setSettings(const BroadcastSettings &settings) {
    int deviceIndex = (settings.outputInterface == "decklink") ? 1 : 0;
    ui->outputDeviceCombo->setCurrentIndex(deviceIndex);
    
    int profileIndex = ui->profileCombo->findText(settings.profile);
    if (profileIndex != -1) ui->profileCombo->setCurrentIndex(profileIndex);
    
    ui->enableOutputCheck->setChecked(settings.externalOutputEnabled);
}

BroadcastSettings SettingsDialog::settings() const {
    BroadcastSettings s;
    s.outputInterface = (ui->outputDeviceCombo->currentIndex() == 1) ? "decklink" : "ndi";
    s.profile = ui->profileCombo->currentText();
    s.externalOutputEnabled = ui->enableOutputCheck->isChecked();
    return s;
}

void SettingsDialog::onSaveClicked() {
    accept();
}
