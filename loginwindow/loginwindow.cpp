#include "loginwindow.h"

#include "../utils/utils.h"

#include <QMessageBox>
#include <QPushButton>

LoginWindow::LoginWindow(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::LoginWindow)
{
	ui->setupUi(this);

	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_DeleteOnClose);

	connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
		[&]()
		{
			if (!Utils::credentialCheck(ui->usernameLineEdit->text(), ui->passwordLineEdit->text()))
				return;
			emit login(ui->usernameLineEdit->text(), ui->passwordLineEdit->text(), ui->saveLoginDetailCheckBox->isChecked());
			emit accept();
		}
	);

	connect(ui->passwordVisibleCheckBox, &QCheckBox::checkStateChanged,
		[&](Qt::CheckState state)
		{
			ui->passwordLineEdit->setEchoMode(state == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password);
		}
	);
}

LoginWindow::~LoginWindow()
{
	delete ui;
}

void LoginWindow::setDetail(const QString& username, const QString& password)
{
	ui->usernameLineEdit->setText(username);
	ui->passwordLineEdit->setText(password);
}
