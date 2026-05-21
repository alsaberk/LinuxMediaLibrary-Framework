#include "graphicsedit.h"
#include "ui_graphicsedit.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

GraphicsEdit::GraphicsEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphicsEdit)
{
    ui->setupUi(this);

    // logobox için opaklık efekti oluştur
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(this);
    ui->logobox->setGraphicsEffect(opacityEffect);

    // logobox ve LogoDrawingArea için event filter'ı kur
    ui->logobox->installEventFilter(this);
    ui->LogoDrawingArea->installEventFilter(this);
    ui->logobox->setMouseTracking(true); 
    ui->logobox->hide();
    ui->logobox->setText(""); 

    // SpinBox değişikliklerini logoya yansıt
    connect(ui->logoXPos, QOverload<int>::of(&QSpinBox::valueChanged), this, &GraphicsEdit::updateLogoFromSpinBoxes);
    connect(ui->logoYPos, QOverload<int>::of(&QSpinBox::valueChanged), this, &GraphicsEdit::updateLogoFromSpinBoxes);
    connect(ui->logoWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &GraphicsEdit::updateLogoFromSpinBoxes);
    connect(ui->logoHeight, QOverload<int>::of(&QSpinBox::valueChanged), this, &GraphicsEdit::updateLogoFromSpinBoxes);
    
    // Opacity slider değiştiğinde hem görseli güncelle hem kaydet
    connect(ui->logoSliderAlpha, &QSlider::valueChanged, this, [this, opacityEffect](int value){
        // Eğer animasyon çalışmıyorsa anlık güncelle
        opacityEffect->setOpacity(value / 255.0);
        saveSettings();
    });

    connect(ui->logoShowCheck, &QPushButton::clicked, this, [this](){
        // Show butonu sadece MLT çıkışını etkileyecek (daha sonra eklenecek)
        // Önizlemedeki logobox görünürlüğünü etkilemez.
        saveSettings();
    });
    
    // Apply butonu tıklandığında kaydet
    connect(ui->applyLogoBtn, &QPushButton::clicked, this, &GraphicsEdit::saveSettings);

    loadSettings();
}




GraphicsEdit::~GraphicsEdit()
{
    delete ui;
}

void GraphicsEdit::saveSettings()
{
    QJsonObject obj;
    obj["logoPath"] = m_logoPath;
    obj["x"] = ui->logoXPos->value();
    obj["y"] = ui->logoYPos->value();
    obj["width"] = ui->logoWidth->value();
    obj["height"] = ui->logoHeight->value();
    obj["opacity"] = ui->logoSliderAlpha->value();
    obj["show"] = ui->logoShowCheck->isChecked();
    obj["applyTime"] = ui->logoApplyInput->value();

    QJsonDocument doc(obj);
    QFile file("graphics_settings.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void GraphicsEdit::loadSettings()
{
    QFile file("graphics_settings.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    m_logoPath = obj["logoPath"].toString();
    ui->logoXPos->setValue(obj["x"].toInt());
    ui->logoYPos->setValue(obj["y"].toInt());
    ui->logoWidth->setValue(obj["width"].toInt());
    ui->logoHeight->setValue(obj["height"].toInt());
    ui->logoSliderAlpha->setValue(obj["opacity"].toInt(255));
    ui->logoShowCheck->setChecked(obj["show"].toBool());
    ui->logoApplyInput->setValue(obj["applyTime"].toInt());

    if (!m_logoPath.isEmpty()) {
        QPixmap pix(m_logoPath);
        if (!pix.isNull()) {
            m_aspectRatio = (double)pix.width() / pix.height();
            ui->logobox->show();
            ui->logobox->setStyleSheet("background-color: transparent; border: 1px solid red;");
            updateLogoFromSpinBoxes();
        }
    }
}

void GraphicsEdit::on_openLogoBtn_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Logo", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!filePath.isEmpty()) {
        m_logoPath = filePath;
        QPixmap pix(filePath);
        if (!pix.isNull()) {
            m_aspectRatio = (double)pix.width() / pix.height();
            ui->logobox->setPixmap(pix.scaled(ui->logobox->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->logobox->setStyleSheet("background-color: transparent; border: 1px solid red;");
            ui->logobox->show();
            
            ui->logoWidth->setValue(pix.width());
            ui->logoHeight->setValue(pix.height());
            updateLogoFromSpinBoxes();
            saveSettings();
        }
    }
}

void GraphicsEdit::on_emptyLogoBtn_clicked()
{
    ui->logobox->hide();
    ui->logobox->clear();
    ui->logobox->setStyleSheet("");
    m_logoPath = "";
    saveSettings();
}

GraphicsEdit::Edge GraphicsEdit::getEdge(const QPoint &pos)
{
    int margin = 10;
    int w = ui->logobox->width();
    int h = ui->logobox->height();

    if (pos.x() < margin && pos.y() < margin) return TopLeft;
    if (pos.x() > w - margin && pos.y() < margin) return TopRight;
    if (pos.x() < margin && pos.y() > h - margin) return BottomLeft;
    if (pos.x() > w - margin && pos.y() > h - margin) return BottomRight;
    
    if (pos.y() < margin && pos.x() > w/2 - margin && pos.x() < w/2 + margin) return TopCenter;
    if (pos.y() > h - margin && pos.x() > w/2 - margin && pos.x() < w/2 + margin) return BottomCenter;
    if (pos.x() < margin && pos.y() > h/2 - margin && pos.y() < h/2 + margin) return LeftCenter;
    if (pos.x() > w - margin && pos.y() > h/2 - margin && pos.y() < h/2 + margin) return RightCenter;
    
    return None;
}

void GraphicsEdit::updateCursor(Edge edge)
{
    switch (edge) {
        case TopLeft:
        case BottomRight:
            ui->logobox->setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRight:
        case BottomLeft:
            ui->logobox->setCursor(Qt::SizeBDiagCursor);
            break;
        case TopCenter:
        case BottomCenter:
            ui->logobox->setCursor(Qt::SizeVerCursor);
            break;
        case LeftCenter:
        case RightCenter:
            ui->logobox->setCursor(Qt::SizeHorCursor);
            break;
        default:
            ui->logobox->setCursor(Qt::ArrowCursor);
            break;
    }
}

void GraphicsEdit::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

bool GraphicsEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->logobox) {
        if (event->type() == QEvent::Paint) {
            ui->logobox->removeEventFilter(this);
            QApplication::sendEvent(ui->logobox, event);
            ui->logobox->installEventFilter(this);
            
            if (ui->logobox->isVisible()) {
                QPainter painter(ui->logobox);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setBrush(Qt::red);
                painter.setPen(Qt::NoPen);
                
                int w = ui->logobox->width();
                int h = ui->logobox->height();
                int s = 6;
                
                painter.drawRect(0, 0, s, s);
                painter.drawRect(w/2 - s/2, 0, s, s);
                painter.drawRect(w - s, 0, s, s);
                painter.drawRect(0, h/2 - s/2, s, s);
                painter.drawRect(0, h - s, s, s);
                painter.drawRect(w - s, h/2 - s/2, s, s);
                painter.drawRect(w - s, h - s, s, s);
                painter.drawRect(w/2 - s/2, h - s, s, s);
            }
            return true;
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                Edge edge = getEdge(me->pos());
                if (edge != None) {
                    m_isResizing = true;
                    m_resizeEdge = edge;
                } else {
                    m_isDragging = true;
                }
                m_dragOffset = me->pos();
                return true;
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (!m_isDragging && !m_isResizing) {
                updateCursor(getEdge(me->pos()));
            }

            if (m_isDragging) {
                QPoint newPos = ui->logobox->mapToParent(me->pos() - m_dragOffset);
                int maxX = ui->LogoDrawingArea->width() - ui->logobox->width();
                int maxY = ui->LogoDrawingArea->height() - ui->logobox->height();
                newPos.setX(qBound(0, newPos.x(), maxX));
                newPos.setY(qBound(0, newPos.y(), maxY));
                ui->logobox->move(newPos);
                updateSpinBoxesFromLogo();
            }
            else if (m_isResizing) {
                QRect geom = ui->logobox->geometry();
                QPoint globalPos = ui->logobox->mapToParent(me->pos());
                bool shiftPressed = (me->modifiers() & Qt::ShiftModifier);

                if (m_resizeEdge == BottomRight) {
                    int newW = globalPos.x() - geom.left();
                    int newH = globalPos.y() - geom.top();
                    if (!shiftPressed) {
                        if (newW / (double)newH > m_aspectRatio) newW = newH * m_aspectRatio;
                        else newH = newW / m_aspectRatio;
                    }
                    geom.setWidth(qMax(20, newW));
                    geom.setHeight(qMax(20, newH));
                }
                else if (m_resizeEdge == TopLeft) {
                    int newW = geom.right() - globalPos.x();
                    int newH = geom.bottom() - globalPos.y();
                    if (!shiftPressed) {
                        if (newW / (double)newH > m_aspectRatio) newW = newH * m_aspectRatio;
                        else newH = newW / m_aspectRatio;
                    }
                    geom.setTopLeft(QPoint(geom.right() - newW, geom.bottom() - newH));
                }
                else if (m_resizeEdge == TopRight) {
                    int newW = globalPos.x() - geom.left();
                    int newH = geom.bottom() - globalPos.y();
                    if (!shiftPressed) {
                        if (newW / (double)newH > m_aspectRatio) newW = newH * m_aspectRatio;
                        else newH = newW / m_aspectRatio;
                    }
                    geom.setTopRight(QPoint(geom.left() + newW, geom.bottom() - newH));
                }
                else if (m_resizeEdge == BottomLeft) {
                    int newW = geom.right() - globalPos.x();
                    int newH = globalPos.y() - geom.top();
                    if (!shiftPressed) {
                        if (newW / (double)newH > m_aspectRatio) newW = newH * m_aspectRatio;
                        else newH = newW / m_aspectRatio;
                    }
                    geom.setBottomLeft(QPoint(geom.right() - newW, geom.top() + newH));
                }
                else if (m_resizeEdge == RightCenter) {
                    int newW = globalPos.x() - geom.left();
                    geom.setWidth(qMax(20, newW));
                }
                else if (m_resizeEdge == LeftCenter) {
                    int newW = geom.right() - globalPos.x();
                    geom.setLeft(geom.right() - qMax(20, newW));
                }
                else if (m_resizeEdge == TopCenter) {
                    int newH = geom.bottom() - globalPos.y();
                    geom.setTop(geom.bottom() - qMax(20, newH));
                }
                else if (m_resizeEdge == BottomCenter) {
                    int newH = globalPos.y() - geom.top();
                    geom.setHeight(qMax(20, newH));
                }
                ui->logobox->setGeometry(geom);
                updateSpinBoxesFromLogo();
            }
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            if (m_isResizing) {
                if (ui->logobox->height() > 0) {
                    m_aspectRatio = (double)ui->logobox->width() / ui->logobox->height();
                }
            }
            m_isDragging = false;
            m_isResizing = false;
            saveSettings();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void GraphicsEdit::setResolution(int width, int height)
{
    m_targetWidth = width;
    m_targetHeight = height;
    ui->logoXPos->setMaximum(m_targetWidth);
    ui->logoXPos->setMinimum(-m_targetWidth);
    ui->logoYPos->setMaximum(m_targetHeight);
    ui->logoYPos->setMinimum(-m_targetHeight);
    ui->logoWidth->setMaximum(m_targetWidth);
    ui->logoHeight->setMaximum(m_targetHeight);
}

void GraphicsEdit::updateLogoFromSpinBoxes()
{
    double scaleX = (double)ui->LogoDrawingArea->width() / m_targetWidth;
    double scaleY = (double)ui->LogoDrawingArea->height() / m_targetHeight;

    ui->logobox->blockSignals(true);
    ui->logobox->setGeometry(ui->logoXPos->value() * scaleX, 
                             ui->logoYPos->value() * scaleY, 
                             ui->logoWidth->value() * scaleX, 
                             ui->logoHeight->value() * scaleY);
    
    if (!m_logoPath.isEmpty()) {
        QPixmap pix(m_logoPath);
        ui->logobox->setPixmap(pix.scaled(ui->logobox->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    ui->logobox->blockSignals(false);
    update();
}

void GraphicsEdit::updateSpinBoxesFromLogo()
{
    double scaleX = (double)m_targetWidth / ui->LogoDrawingArea->width();
    double scaleY = (double)m_targetHeight / ui->LogoDrawingArea->height();

    ui->logoXPos->blockSignals(true);
    ui->logoYPos->blockSignals(true);
    ui->logoWidth->blockSignals(true);
    ui->logoHeight->blockSignals(true);
    
    ui->logoXPos->setValue(qRound(ui->logobox->x() * scaleX));
    ui->logoYPos->setValue(qRound(ui->logobox->y() * scaleY));
    ui->logoWidth->setValue(qRound(ui->logobox->width() * scaleX));
    ui->logoHeight->setValue(qRound(ui->logobox->height() * scaleY));
    
    if (!m_logoPath.isEmpty()) {
        QPixmap pix(m_logoPath);
        ui->logobox->setPixmap(pix.scaled(ui->logobox->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    
    ui->logoXPos->blockSignals(false);
    ui->logoYPos->blockSignals(false);
    ui->logoWidth->blockSignals(false);
    ui->logoHeight->blockSignals(false);
}
