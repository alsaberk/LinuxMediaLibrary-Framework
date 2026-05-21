#ifndef GRAPHICSEDIT_H
#define GRAPHICSEDIT_H

#include <QWidget>
#include <QImage>

namespace Ui {
class GraphicsEdit;
}

class GraphicsEdit : public QWidget
{
    Q_OBJECT

public:
    explicit GraphicsEdit(QWidget *parent = nullptr);
    ~GraphicsEdit();

    void setResolution(int width, int height);
    void saveSettings();
    void loadSettings();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_openLogoBtn_clicked();
    void on_emptyLogoBtn_clicked();
    void updateLogoFromSpinBoxes();
    void updateSpinBoxesFromLogo();

private:
    Ui::GraphicsEdit *ui;
    QString m_logoPath;
    bool m_isDragging = false;
    bool m_isResizing = false;
    int m_resizeEdge = 0; 
    QPoint m_dragOffset;
    QSize m_originalSize;
    double m_aspectRatio = 1.0;
    
    int m_targetWidth = 1920;
    int m_targetHeight = 1080;

    enum Edge { 
        None = 0, 
        TopLeft = 1, TopRight = 2, BottomLeft = 3, BottomRight = 4,
        TopCenter = 5, BottomCenter = 6, LeftCenter = 7, RightCenter = 8
    };
    Edge getEdge(const QPoint &pos);
    void updateCursor(Edge edge);
};

#endif // GRAPHICSEDIT_H
