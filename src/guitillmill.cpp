#include "guitillmill.h"
#include "ui_guitillmill.h"
#include <QInputDialog>
#include <QListWidgetItem>

#

GUITillMill::GUITillMill(DM::Module * m, QWidget *parent) :
QDialog(parent),
m((TileMill*)m),
ui(new Ui::GUITillMill)
{
	ui->setupUi(this);

	std::vector<std::string> viewnames = m->getParameter<std::vector<std::string> >("ViewNames");
	foreach (std::string v, viewnames) {
		ui->listWidget_view->addItem(QString::fromStdString(v));
	}
}

GUITillMill::~GUITillMill()
{
	delete ui;
}

void GUITillMill::on_pushButton_view_clicked()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
										 tr("ViewName"), QLineEdit::Normal,
										 "", &ok);
	ui->listWidget_view->addItem(text);
}

void GUITillMill::accept()
{
	std::vector<std::string> views;
	for (int i = 0; i < ui->listWidget_view->count(); i++) {

		if (ui->listWidget_view->item(i)->text().isEmpty())
			continue;
		views.push_back(ui->listWidget_view->item(i)->text().toStdString());
	}

	this->m->setParameterNative<std::vector<std::string> >("ViewNames", views);

	QDialog::accept();
}


