#ifndef RANDCODEDIALOG_H
#define RANDCODEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

class randcodedialog : public QDialog
{
    Q_OBJECT

public:
    explicit randcodedialog(const QByteArray& imgData,
                            int displayWidth = 800,
                            QWidget* parent = nullptr);

    QString getCode() const;

private:
    QLabel* imageLabel;
    QLineEdit* codeEdit;
};

#endif //RANDCODEDIALOG_H
