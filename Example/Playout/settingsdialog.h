#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

struct BroadcastSettings {
    QString outputInterface; // "ndi" veya "decklink"
    QString profile;         // "atsc_1080p_50" vb.
    bool externalOutputEnabled;
};

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setSettings(const BroadcastSettings &settings);
    BroadcastSettings settings() const;

private slots:
    void onSaveClicked();

private:
    Ui::SettingsDialog *ui;
    void loadProfiles();
};

#endif // SETTINGSDIALOG_H
