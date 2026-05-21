#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QWidget>

#include "LFile.h"
#include "LMixer.h"
#include "LPreview.h"
#include "LCharacter.h"

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
  // File 1 slots
  void onOpenFile1();
  void onPlayFile1();
  void onStopFile1();

  // File 2 slots
  void onOpenFile2();
  void onPlayFile2();
  void onStopFile2();

  // PiP Layout slots
  void onPipLayoutChanged();

  // CG Overlay slots
  void onAddLogo();
  void onAddLowerThird();
  void onAddTicker();
  void onClearCg();

  // UI update
  void updateUI();

private:
  Ui::MainWindow *ui = nullptr;

  // --- LinuxMediaLibrary ---
  LFile      *m_lFile1        = nullptr;
  LFile      *m_lFile2        = nullptr;
  LMixer     *m_lMixer        = nullptr;
  LCharacter *m_lCG           = nullptr;

  // --- Previews ---
  LPreview   *m_lPreviewFile1 = nullptr;
  LPreview   *m_lPreviewFile2 = nullptr;
  LPreview   *m_lPreviewMixer = nullptr;

  // --- Mixer Layers ---
  LLayerItem *m_file1Layer    = nullptr;
  LLayerItem *m_file2Layer    = nullptr;

  // --- Timers ---
  QTimer     *m_uiTimer       = nullptr;
};

#endif // MAINWINDOW_H
