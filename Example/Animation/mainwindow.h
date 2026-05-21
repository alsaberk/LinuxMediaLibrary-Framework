#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QColor>
#include <QColorDialog>
#include <string>

#include "LFile.h"
#include "LMixer.h"
#include "LAnimation.h"
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
  // --- Video File Controls ---
  void onOpenFile();
  void onPlayVideo();
  void onStopVideo();

  // --- Scoreboard CG Controls ---
  void onStartCG();
  void onStopCG();
  void onCloseCG();
  
  // --- Game Clock Slot ---
  void onToggleGameClock();
  void onGameClockTick();

  // --- Update Scoreboard Data Slot ---
  void onUpdateCGData();
  void onSelectHomeBgColor();
  void onSelectHomeJerseyColor();
  void onSelectGuestBgColor();
  void onSelectGuestJerseyColor();

  // --- Lower Third CG Controls ---
  void onStartLT();
  void onStopLT();
  void onCloseLT();
  void onUpdateLTData();
  void onSelectLtUpBgColor();
  void onSelectLtDownBgColor();

  // --- UI Update Loop ---
  void updateUI();

private:
  Ui::MainWindow *ui = nullptr;

  // --- Media Framework Components ---
  LFile      *m_lFile      = nullptr;
  LMixer     *m_lMixer     = nullptr;
  LAnimation *m_lAnimation = nullptr;

  // --- Previews ---
  LPreview   *m_lPreviewFile = nullptr;
  LPreview   *m_lPreviewProg = nullptr;

  // --- UI Timer ---
  QTimer     *m_uiTimer      = nullptr;

  // --- Match Game Clock Variables ---
  QTimer     *m_gameClockTimer = nullptr;
  int        m_elapsedSeconds  = 0;
  bool       m_gameClockRunning = false;

  // --- Team Colors ---
  QString    m_homeBgColorHex = "#ef4444";      // Red background
  QString    m_homeJerseyColorHex = "#ffffff";  // White jersey accent
  QString    m_guestBgColorHex = "#3b82f6";     // Blue background
  QString    m_guestJerseyColorHex = "#ffffff"; // White jersey accent
  bool       m_cgActive = false;                // Track active CG status

  // --- Lower Third Colors & State ---
  QString    m_ltUpBgColorHex = "#4f46e5";      // Indigo/Purple background
  QString    m_ltDownBgColorHex = "#10b981";    // Emerald/Green background
  bool       m_ltActive = false;                // Track active LT status

  // --- Helpers ---
  void updateColorButtonStyles();
  QString formatTime(int totalSeconds);
};

#endif // MAINWINDOW_H
