#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemModel>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <QWidget>

#include "LFile.h"
#include "LPreview.h"
#include "LFilter.h"
#include "globalproperties.h"

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
  void onAddFile();
  void addFileToPlaylist(const QString &filePath, int row = -1);
  void selectFile(int index);
  void onRowDoubleClicked(const QModelIndex &index);
  void onPlayClicked();
  void onPauseClicked();
  void onStopClicked();
  void onNextClicked();
  void onBackClicked();
  void updateUI();
  void updateButtonStates();

  // Explorer slots
  void onDriveChanged(int index);
  void onExplorerActivated(const QModelIndex &index);
  void refreshDrives();

private:
  Ui::MainWindow *ui;

  // --- Playlist ---
  QStandardItemModel *m_playlistModel = nullptr;
  QStringList         m_filePaths;

  // --- LinuxMediaLibrary ---
  LFile    *m_lFile         = nullptr;
  LPreview *m_lPreview  = nullptr;  // playerPreview (sol)
  LFilter  *m_colorFilter   = nullptr;
  LFilter  *m_delayFilter   = nullptr;

  // --- Durum ve Zamanlayıcılar ---
  globalProperties *m_globalProps = nullptr;
  int               m_currentIndex  = -1;
  QTimer           *m_uiTimer       = nullptr;
  QTimer           *m_driveTimer    = nullptr;

  // --- Dosya Gezgini ---
  QFileSystemModel *m_listModel     = nullptr;

  bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // MAINWINDOW_H
