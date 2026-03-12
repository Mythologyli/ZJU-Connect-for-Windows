#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "randcodedialog.h"

randcodedialog::randcodedialog(const QByteArray& imgData,
                               int displayWidth,
                               QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("请输入图片中的验证码");

    QImage origImage;
    origImage.loadFromData(reinterpret_cast<const uchar*>(imgData.constData()), imgData.size());

    QPixmap imagePixmap;
    imagePixmap = QPixmap::fromImage(
        origImage.scaledToWidth(displayWidth, Qt::SmoothTransformation)
    );

    QVBoxLayout* mainLay = new QVBoxLayout(this);
    imageLabel = new QLabel;
    imageLabel->setPixmap(imagePixmap);
    imageLabel->setFixedSize(imagePixmap.size());
    mainLay->addWidget(imageLabel, 0, Qt::AlignCenter);

    codeEdit = new QLineEdit;
    codeEdit->setPlaceholderText("请输入字符串验证码");
    mainLay->addWidget(codeEdit);

    QHBoxLayout* btnLay = new QHBoxLayout;
    QPushButton* btnConfirm = new QPushButton("确定");
    btnLay->addStretch();
    btnLay->addWidget(btnConfirm);
    mainLay->addLayout(btnLay);

    connect(btnConfirm, &QPushButton::clicked, this, &randcodedialog::accept);
}

QString randcodedialog::getCode() const
{
    return codeEdit->text();
}
