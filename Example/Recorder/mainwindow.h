#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include "LLive.h"
#include "LRecorder.h"
#include "LPreview.h"
#include "LFormat.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Live Input controls
    void refreshLiveSources();
    void onDeviceChanged(int index);
    void onChannelChanged(int index);
    void onFormatChanged(int index);
    void onLiveClicked();

    // Folder selection
    void onSelectFolderClicked();
    
    // Mode Standard vs Custom
    void onModeToggled();

    // Container changed -> Dynamic Codec mapping
    void onContainerChanged(const QString& container);

    // Recording Controls
    void onStartRecordingClicked();
    void onPauseRecordingClicked();
    void onStopRecordingClicked();

    // Periodic UI statistics update
    void updateUIStats();

    // Dynamic Recorder Synchronization helper
    void applyRecorderSettings();

private:
    Ui::MainWindow *ui;
    
    LLive* m_lLive;
    LPreview* m_lPreview;
    LRecorder* m_lRecorder;

    QTimer* m_statsTimer;
    QString m_outputDir;
    
    bool m_isRecording;
    bool m_isPaused;
};

#endif // MAINWINDOW_H
