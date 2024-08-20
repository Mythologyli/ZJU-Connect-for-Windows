#include "loginwindow.h"

#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::LoginWindow)
{
	ui->setupUi(this);

	setWindowModality(Qt::WindowModal);
	setAttribute(Qt::WA_DeleteOnClose);

	connect(ui->buttonBox, &QDialogButtonBox::accepted,
		[&]()
		{
			if (ui->usernameLineEdit->text().isEmpty())
			{
				QMessageBox::warning(this, "警告", "账号不应为空！");
				return;
			}
			else if (ui->passwordLineEdit->text().isEmpty())
			{
				QMessageBox::warning(this, "警告", "密码不应为空！");
				return;
			}
			emit login(ui->usernameLineEdit->text(), ui->passwordLineEdit->text(), ui->saveLoginDetailCheckBox->isChecked());
			emit accept();
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
