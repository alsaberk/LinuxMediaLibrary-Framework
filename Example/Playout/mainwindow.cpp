#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "filedialog.h"
#include "rowcolordelegate.h"
#include "LReader.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QStorageInfo>
#include <QThreadPool>
#include <QtConcurrent>
#include <QMessageBox>

// Yardımcı: Milisaniye → HH:MM:SS:FF (Timecode formatı)
static QString msToTimecode(double ms, double fps) {
  if (ms < 0) ms = 0;
  if (fps <= 0) fps = 25.0;

  int total_f = static_cast<int>((ms * fps) / 1000.0);
  int ff = total_f % static_cast<int>(fps);
  int total_s = static_cast<int>(ms / 1000.0);
  int ss = total_s % 60;
  int mm = (total_s / 60) % 60;
  int hh = total_s / 3600;

  return QString::asprintf("%02d:%02d:%02d.%02d", hh, mm, ss, ff);
}

// ─── Kurucu ─────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_playlistModel(new QStandardItemModel(this)),
      m_lFile(new LFile()),
      m_lPreview(new LPreview()),
      m_globalProps(new globalProperties()),
      m_uiTimer(new QTimer(this)),
      m_driveTimer(new QTimer(this)),
      m_listModel(new QFileSystemModel(this))
{
  ui->setupUi(this);
  setWindowTitle("Playout");

  // GlobalProps → seçili dosya ve durum için
  m_globalProps->currentState = globalProperties::Nothing;
  m_lFile->setProps("eof_hold", "true");

  // ADD Color Filter - Only m_lFile->setProps("gpu", "false"); mode
  m_colorFilter = new LFilter(LFilter::Video);
  m_colorFilter->setProps("eq", "brightness=1.0:contrast=3.0:saturation=2.5");
  //m_lFile->addFilter(m_colorFilter); // Renk filtresi için aç

  // ADD audio Delay Filter
  m_delayFilter = new LFilter(LFilter::Audio);
  m_delayFilter->setProps("adelay", "250|250");
  //m_lFile->addFilter(m_delayFilter);

  // Sol preview (playerPreview)
  m_lPreview->setProps("ui_framework", "qt");
  m_lPreview->previewEnable(ui->playerPreview, true, true);
  m_lPreview->previewObject(m_lFile);
  m_lPreview->setProps("audio_meter", "true");
  m_lPreview->setProps("timecode.preview", "true");
  //m_lPreview->setStatus("program");
  //m_lPreview->setName("PLAYOUT");

  // Playlist tablosu kurulumu
  m_playlistModel->setColumnCount(6);
  m_playlistModel->setHorizontalHeaderLabels(
      {"#", "Filename", "In Time", "Out Time", "Duration", "Resolution"});
  ui->playlistTableView->horizontalHeader()->setDefaultAlignment(
      Qt::AlignLeft | Qt::AlignVCenter);
  ui->playlistTableView->setModel(m_playlistModel);

  QHeaderView *header = ui->playlistTableView->horizontalHeader();
  header->setSectionResizeMode(0, QHeaderView::Fixed);   header->resizeSection(0, 40);
  header->setSectionResizeMode(1, QHeaderView::Stretch);
  header->setSectionResizeMode(2, QHeaderView::Fixed);   header->resizeSection(2, 150);
  header->setSectionResizeMode(3, QHeaderView::Fixed);   header->resizeSection(3, 150);
  header->setSectionResizeMode(4, QHeaderView::Fixed);   header->resizeSection(4, 150);
  header->setSectionResizeMode(5, QHeaderView::Fixed);   header->resizeSection(5, 200);
  ui->playlistTableView->setItemDelegate(new RowColorDelegate(m_globalProps, this));

  // Buton bağlantıları
  connect(ui->addFile,  &QPushButton::clicked, this, &MainWindow::onAddFile);
  connect(ui->playBtn,  &QPushButton::clicked, this, &MainWindow::onPlayClicked);
  connect(ui->pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseClicked);
  connect(ui->stopBtn,  &QPushButton::clicked, this, &MainWindow::onStopClicked);
  connect(ui->nextBtn,  &QPushButton::clicked, this, &MainWindow::onNextClicked);
  connect(ui->backBtn,  &QPushButton::clicked, this, &MainWindow::onBackClicked);

  connect(ui->playlistTableView, &QTableView::doubleClicked,
          this, &MainWindow::onRowDoubleClicked);

  // UI timer
  connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::updateUI);
  m_uiTimer->start(50);

  // Sürücü & dosya gezgini timer ve bağlantıları
  updateButtonStates();
  refreshDrives();

  connect(m_driveTimer, &QTimer::timeout, this, &MainWindow::refreshDrives);
  m_driveTimer->start(3000);

  QString initialPath = QDir::homePath();
  m_listModel->setRootPath(initialPath);
  m_listModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDot);
  QStringList filters;
  filters << "*.mp4" << "*.mkv" << "*.avi" << "*.mov" << "*.mpg" << "*.mpeg"
          << "*.wmv" << "*.flv" << "*.webm" << "*.mxf" << "*.ts"  << "*.mts"
          << "*.m2ts" << "*.wav" << "*.mp3" << "*.aac";
  m_listModel->setNameFilters(filters);
  m_listModel->setNameFilterDisables(false);

  ui->listViewExp->setModel(m_listModel);
  ui->listViewExp->setRootIndex(m_listModel->index(initialPath));
  ui->listViewExp->setDragEnabled(true);

  connect(ui->driveComboBox, QOverload<int>::of(&QComboBox::activated),
          this, &MainWindow::onDriveChanged);
  connect(ui->listViewExp, &QListView::doubleClicked,
          this, &MainWindow::onExplorerActivated);

  // Sürükle-Bırak
  ui->playlistTableView->setAcceptDrops(true);
  ui->playlistTableView->setDropIndicatorShown(true);
  ui->playlistTableView->setDefaultDropAction(Qt::CopyAction);
  ui->playlistTableView->installEventFilter(this);

  updateUI();
}

// ─── Yıkıcı ─────────────────────────────────────────────────────────────────
MainWindow::~MainWindow() {
  if (m_lFile) m_lFile->stop();

  delete m_lPreview;
  delete m_colorFilter;
  delete m_delayFilter;
  delete m_lFile;
  delete m_globalProps;
  delete ui;
}

// ─── Dosya Ekleme ────────────────────────────────────────────────────────────
void MainWindow::onAddFile() {
  FileDialog dlg(this);
  if (dlg.exec() == QDialog::Accepted) {
    for (const QString &fp : dlg.selectedFiles())
      addFileToPlaylist(fp);
  }
}

void MainWindow::addFileToPlaylist(const QString &filePath, int row) {
  int finalRow = (row < 0 || row > m_filePaths.size()) ? m_filePaths.size() : row;

  QThreadPool::globalInstance()->start([this, filePath, finalRow]() {
    QFileInfo fi(filePath);
    if (!fi.exists()) return;

    LReader probe;
    if (probe.open(filePath.toStdString())) {
      double durMs   = probe.getDurationMs();
      double fps     = probe.getFps();
      int height     = probe.getHeight();
      double inMs    = probe.getInPoint();
      double outMs   = probe.getOutPoint();
      QString scanType = QString::fromStdString(probe.getScanType());
      QString scanChar = (scanType == "Progressive") ? "p" : "i";
      QString resFinal = QString("%1%2%3").arg(height).arg(scanChar).arg(qRound(fps));

      QMetaObject::invokeMethod(this, [this, finalRow, filePath, fi, inMs, outMs, durMs, fps, resFinal]() {
        m_filePaths.insert(finalRow, filePath);

        auto *posItem = new QStandardItem(QString::number(finalRow + 1));
        posItem->setData(filePath, Qt::UserRole);

        QList<QStandardItem *> rowItems = {
            posItem,
            new QStandardItem(fi.fileName()),
            new QStandardItem(msToTimecode(inMs, fps)),
            new QStandardItem(msToTimecode(outMs, fps)),
            new QStandardItem(msToTimecode(durMs, fps)),
            new QStandardItem(resFinal)
        };
        m_playlistModel->insertRow(finalRow, rowItems);

        for (int i = 0; i < m_playlistModel->rowCount(); ++i) {
          if (auto *it = m_playlistModel->item(i, 0))
            it->setText(QString::number(i + 1));
        }

        updateButtonStates();
        ui->playlistTableView->viewport()->update();

        if (m_filePaths.size() == 1) {
          selectFile(0);
        }
      }, Qt::QueuedConnection);
    }
  });
}

// ─── Dosya Seçimi ────────────────────────────────────────────────────────────
void MainWindow::selectFile(int index) {
  if (index < 0 || index >= m_filePaths.size()) return;

  m_currentIndex = index;
  m_globalProps->selectfile = index;

  m_lFile->stop();
  m_lFile->fileNameSet(m_filePaths[index].toStdString());

  ui->fileLabel->setText(m_playlistModel->item(index, 1)->text());

  if (m_globalProps->currentState == globalProperties::Playing) {
    m_lFile->play();
    m_lPreview->previewObject(m_lFile);
  } else {
    m_globalProps->currentState = globalProperties::Stopped;
  }

  updateButtonStates();
  ui->playlistTableView->viewport()->update();
}

void MainWindow::onRowDoubleClicked(const QModelIndex &index) {
  if (index.isValid()) selectFile(index.row());
}

// ─── Oynatma Kontrolleri ─────────────────────────────────────────────────────
void MainWindow::onPlayClicked() {
  if (m_filePaths.isEmpty()) return;

  if (m_currentIndex < 0) selectFile(0);

  m_globalProps->currentState = globalProperties::Playing;
  m_lFile->play();
  updateButtonStates();
}

void MainWindow::onPauseClicked() {
  m_lFile->pause();
  m_globalProps->currentState = globalProperties::Paused;
  updateButtonStates();
}

void MainWindow::onStopClicked() {
  m_lFile->stop();
  m_globalProps->currentState = globalProperties::Stopped;
  updateButtonStates();
}

void MainWindow::onNextClicked() {
  if (m_currentIndex + 1 < m_filePaths.size()) {
    selectFile(m_currentIndex + 1);
  }
}

void MainWindow::onBackClicked() {
  if (m_currentIndex - 1 >= 0) {
    selectFile(m_currentIndex - 1);
  }
}

// ─── Buton Durumları ─────────────────────────────────────────────────────────
void MainWindow::updateButtonStates() {
  bool hasFiles = !m_filePaths.isEmpty();
  ui->playBtn->setEnabled(hasFiles && m_globalProps->currentState != globalProperties::Playing);
  ui->pauseBtn->setEnabled(m_globalProps->currentState == globalProperties::Playing);
  ui->stopBtn->setEnabled(hasFiles);
  ui->nextBtn->setEnabled(m_currentIndex + 1 < m_filePaths.size());
  ui->backBtn->setEnabled(m_currentIndex - 1 >= 0);
}

// ─── UI Güncelleme (50ms'de bir) ─────────────────────────────────────────────
void MainWindow::updateUI() {
  double curMs = m_lFile->PosGet();
  double durMs = m_lFile->getDurationMs();
  double fps   = m_lFile->getFPS();
  if (fps <= 0) fps = 25.0;

  bool hasFiles = !m_filePaths.isEmpty();

  if (hasFiles && m_currentIndex >= 0 && durMs > 0) {
    ui->FileProgressBar->setMaximum(static_cast<int>(durMs));
    ui->FileProgressBar->setValue(static_cast<int>(curMs));
    ui->FileProgressBar->setFormat(msToTimecode(curMs, fps) + " / " + msToTimecode(durMs, fps));
    ui->fileLabel->setText(m_playlistModel->item(m_currentIndex, 1)->text());
    ui->clipremainingTimeLbl->setText(msToTimecode(durMs - curMs, fps));

    ui->PlaylistProgressBar->setMaximum(static_cast<int>(durMs));
    ui->PlaylistProgressBar->setValue(static_cast<int>(curMs));
    ui->PlaylistProgressBar->setFormat(msToTimecode(curMs, fps));
    ui->pCurTimeLbl->setText(msToTimecode(curMs, fps));
    ui->pDurTimeLbl->setText(msToTimecode(durMs, fps));
    ui->pRemainingTimeLbl->setText(msToTimecode(durMs - curMs, fps));

  } else {
    ui->FileProgressBar->setMaximum(1);
    ui->FileProgressBar->setValue(0);
    ui->FileProgressBar->setFormat("00:00:00:00 / 00:00:00:00");

    ui->PlaylistProgressBar->setMaximum(1);
    ui->PlaylistProgressBar->setValue(0);
    ui->PlaylistProgressBar->setFormat("00:00:00:00");

    ui->pCurTimeLbl->setText("00:00:00:00");
    ui->pDurTimeLbl->setText("00:00:00:00");
    ui->pRemainingTimeLbl->setText("00:00:00:00");
    ui->clipremainingTimeLbl->setText("00:00:00:00");
  }

  ui->playlistTableView->viewport()->update();
}

// ─── Sürükle-Bırak ──────────────────────────────────────────────────────────
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
  if (watched == ui->playlistTableView->viewport() ||
      watched == ui->playlistTableView) {
    if (event->type() == QEvent::DragEnter) {
      auto *dee = static_cast<QDragEnterEvent *>(event);
      if (dee->mimeData()->hasUrls()) { dee->acceptProposedAction(); return true; }
    } else if (event->type() == QEvent::DragMove) {
      auto *dme = static_cast<QDragMoveEvent *>(event);
      QModelIndex idx = ui->playlistTableView->indexAt(dme->position().toPoint());
      int newRow = idx.isValid() ? idx.row() : -1;
      if (newRow != m_globalProps->dropIndicatorRow) {
        m_globalProps->dropIndicatorRow = newRow;
        ui->playlistTableView->viewport()->update();
      }
      dme->acceptProposedAction(); return true;
    } else if (event->type() == QEvent::DragLeave) {
      m_globalProps->dropIndicatorRow = -1;
      ui->playlistTableView->viewport()->update(); return true;
    } else if (event->type() == QEvent::Drop) {
      auto *de = static_cast<QDropEvent *>(event);
      m_globalProps->dropIndicatorRow = -1;
      ui->playlistTableView->viewport()->update();
      const QMimeData *md = de->mimeData();
      if (md->hasUrls()) {
        QModelIndex idx = ui->playlistTableView->indexAt(de->position().toPoint());
        int dropRow = idx.isValid() ? idx.row() + 1 : -1;
        for (const QUrl &url : md->urls()) {
          QString fp = url.toLocalFile();
          if (!fp.isEmpty()) {
            addFileToPlaylist(fp, dropRow);
            if (dropRow != -1) dropRow++;
          }
        }
        de->acceptProposedAction(); return true;
      }
    }
  }
  return QMainWindow::eventFilter(watched, event);
}

// ─── Disk Gezgini ───────────────────────────────────────────────────────────
void MainWindow::onDriveChanged(int index) {
  QString path = ui->driveComboBox->itemData(index).toString();
  if (!path.isEmpty())
    ui->listViewExp->setRootIndex(m_listModel->setRootPath(path));
}

void MainWindow::onExplorerActivated(const QModelIndex &index) {
  if (m_listModel->isDir(index))
    ui->listViewExp->setRootIndex(
        m_listModel->setRootPath(m_listModel->filePath(index)));
}

void MainWindow::refreshDrives() {
  QString currentPath = ui->driveComboBox->currentData().toString();
  QStringList newPaths;
  QList<QPair<QString, QString>> driveList;

  for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
    if (storage.isValid() && storage.isReady() && !storage.isReadOnly()) {
      QString path = storage.rootPath();
      bool ok = (path == "/") || (path == "/home") ||
                path.startsWith("/media") || path.startsWith("/mnt") ||
                path.startsWith("/run/media");
      if (ok) {
        QString name = storage.displayName();
        if (path == "/")      name = "Sistem (Root)";
        else if (path == "/home") name = "Kullanici (Home)";
        else if (name.isEmpty())  name = path;
        driveList.append({name, path});
        newPaths << path;
      }
    }
  }

  static QStringList lastPaths;
  if (newPaths == lastPaths) return;
  lastPaths = newPaths;

  ui->driveComboBox->blockSignals(true);
  ui->driveComboBox->clear();
  int toSelect = 0;
  for (int i = 0; i < driveList.size(); ++i) {
    ui->driveComboBox->addItem(driveList[i].first, driveList[i].second);
    if (driveList[i].second == currentPath) toSelect = i;
  }
  ui->driveComboBox->setCurrentIndex(toSelect);
  ui->driveComboBox->blockSignals(false);
}
