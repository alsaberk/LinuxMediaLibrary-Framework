#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>

#include "LFile.h"
#include "LLive.h"
#include "LMixer.h"
#include "LSwitcher.h"
#include "LPreview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private slots:
  // --- File 1 & 2 Controls ---
  void onOpenFile1();
  void onPlayFile1();
  void onStopFile1();

  void onOpenFile2();
  void onPlayFile2();
  void onStopFile2();

  // --- Live 1 & 2 Controls ---
  void refreshLiveSources(int liveIndex);
  void onLiveDeviceChanged(int liveIndex, int deviceIdx);
  void onLiveChannelChanged(int liveIndex, int channelIdx);
  void onLiveFormatChanged(int liveIndex, int formatIdx);
  void onLiveToggle(int liveIndex);

  // --- Switcher Program/Preview Bus Controls ---
  void setProgramSource(int sourceIdx); // 0: File 1, 1: File 2, 2: Live 1, 3: Live 2
  void setPreviewSource(int sourceIdx); // 0: File 1, 1: File 2, 2: Live 1, 3: Live 2

  // --- Switcher Transition Controls ---
  void onTransitionTypeChanged(int index);
  void onCutTransition();
  void onAutoTransition();
  void onStingerTransition();
  void onTBarChanged(int value);

  // --- Stinger Controls ---
  void onSelectStingerPath();
  void onStingerDurationChanged(int value);

  // --- Mixer Logo Controls ---
  void onSelectLogoPath();
  void onLogoLayoutChanged();
  void onToggleLogo(bool checked);

  // --- UI Update Loop ---
  void updateUI();

private:
  Ui::MainWindow *ui = nullptr;

  // --- Core Media Modules ---
  LFile      *m_lFile1        = nullptr;
  LFile      *m_lFile2        = nullptr;
  LLive      *m_lLive1        = nullptr;
  LLive      *m_lLive2        = nullptr;
  LSwitcher  *m_lSwitcher     = nullptr;
  LMixer     *m_lMixer        = nullptr;

  // --- Previews ---
  LPreview   *m_lPreviewFile1 = nullptr;
  LPreview   *m_lPreviewFile2 = nullptr;
  LPreview   *m_lPreviewLive1 = nullptr;
  LPreview   *m_lPreviewLive2 = nullptr;
  LPreview   *m_lPreviewPrev  = nullptr;
  LPreview   *m_lPreviewProg  = nullptr;

  // --- Active Source Selection ---
  int m_activeProgramSource = 0; // 0: File 1, 1: File 2, 2: Live 1, 3: Live 2
  int m_activePreviewSource = 1; // 0: File 1, 1: File 2, 2: Live 1, 3: Live 2

  // --- Stinger & Logo Properties ---
  std::string m_stingerPath = "";
  std::string m_logoPath = "";
  bool m_logoEnabled = false;
  bool m_tBarReversed = false;

  // --- Update Timer ---
  QTimer     *m_uiTimer       = nullptr;

  // --- Helper Methods ---
  LObject* getSourceObject(int index);
  void updateBusButtons();
};

#endif // MAINWINDOW_H
