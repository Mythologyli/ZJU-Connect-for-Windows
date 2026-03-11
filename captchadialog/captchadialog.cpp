#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QPushButton>

#include "captchadialog.h"

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
    setMouseTracking(true);
}

void ClickableLabel::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        emit clicked(ev->position().toPoint());
    }
    QLabel::mousePressEvent(ev);
}

captchadialog::captchadialog(const QByteArray& imgData,
                             int displayWidth,
                             QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("请按顺序点击图片上的文字");

    origImage.loadFromData(reinterpret_cast<const uchar*>(imgData.constData()), imgData.size());
    origWidth = origImage.width();
    origHeight = origImage.height();

    if (origWidth > displayWidth)
    {
        scaleFactor = static_cast<double>(origWidth) / displayWidth;
        scaledPixmap = QPixmap::fromImage(
            origImage.scaledToWidth(displayWidth, Qt::SmoothTransformation)
        );
    }
    else
    {
        scaleFactor = 1.0;
        scaledPixmap = QPixmap::fromImage(origImage);
    }
    displayedPixmap = scaledPixmap;

    QVBoxLayout* mainLay = new QVBoxLayout(this);
    imageLabel = new ClickableLabel;
    imageLabel->setPixmap(displayedPixmap);
    imageLabel->setFixedSize(displayedPixmap.size());
    mainLay->addWidget(imageLabel, 0, Qt::AlignCenter);
    QHBoxLayout* btnLay = new QHBoxLayout;
    btnReset = new QPushButton("重置");
    btnConfirm = new QPushButton("确定");
    btnLay->addWidget(btnReset);
    btnLay->addStretch();
    btnLay->addWidget(btnConfirm);
    mainLay->addLayout(btnLay);

    connect(imageLabel, &ClickableLabel::clicked,
            this, &captchadialog::onImageClicked);
    connect(btnReset, &QPushButton::clicked,
            this, &captchadialog::onResetClicked);
    connect(btnConfirm, &QPushButton::clicked,
            this, &captchadialog::accept);
}

void captchadialog::onImageClicked(const QPoint& pt)
{
    int ox = int(pt.x() * scaleFactor);
    int oy = int(pt.y() * scaleFactor);
    points.append(QPoint(ox, oy));
    QPainter p(&displayedPixmap);
    p.setPen(Qt::red);
    p.setBrush(Qt::red);
    constexpr int R = 5;
    p.drawEllipse(pt, R, R);
    p.end();
    imageLabel->setPixmap(displayedPixmap);
}

void captchadialog::onResetClicked()
{
    points.clear();
    displayedPixmap = scaledPixmap;
    imageLabel->setPixmap(displayedPixmap);
}

QString captchadialog::getJson() const
{
    QJsonObject root;
    QJsonArray arr;
    for (auto& pt : points)
    {
        QJsonArray xy;
        xy.append(pt.x());
        xy.append(pt.y());
        arr.append(xy);
    }
    root["coordinates"] = arr;
    root["width"] = origWidth;
    root["height"] = origHeight;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}
