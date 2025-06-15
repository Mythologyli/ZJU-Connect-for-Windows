#ifndef CAPTCHADIALOG_H
#define CAPTCHADIALOG_H

#include <QDialog>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QPoint>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = nullptr);
signals:
    void clicked(const QPoint& pt);

protected:
    void mousePressEvent(QMouseEvent* ev) override;
};

class captchadialog : public QDialog
{
    Q_OBJECT

public:
    explicit captchadialog(const QByteArray& imgData,
                           int displayWidth = 800,
                           QWidget* parent = nullptr);

    QString getJson() const;

private slots:
    void onImageClicked(const QPoint& pt);
    void onResetClicked();

private:
    ClickableLabel* imageLabel;
    QPushButton* btnReset;
    QPushButton* btnConfirm;

    QImage origImage;
    int origWidth = 0;
    int origHeight = 0;

    double scaleFactor = 1.0;
    QPixmap scaledPixmap;

    QPixmap displayedPixmap;
    QVector<QPoint> points;
};

#endif //CAPTCHADIALOG_H
