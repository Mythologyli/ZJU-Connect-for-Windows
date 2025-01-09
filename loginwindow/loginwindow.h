#pragma once

#include <QDialog>
#include "ui_loginwindow.h"

class LoginWindow : public QDialog
{
	Q_OBJECT

public:
	LoginWindow(QWidget *parent = nullptr);

	~LoginWindow() override;

	void setDetail(const QString& username, const QString& password);

	void keyPressEvent(QKeyEvent *event) override;

signals:
	void login(const QString& username, const QString& password, bool saveDetail);

private:
	Ui::LoginWindow *ui;
};
