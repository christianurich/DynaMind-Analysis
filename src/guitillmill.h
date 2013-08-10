#ifndef GUITILLMILL_H
#define GUITILLMILL_H
#include <dmcompilersettings.h>
#include <tile_mill.h>
#include <QDialog>

namespace Ui {
class GUITillMill;
}


class GUITillMill : public QDialog
{
	Q_OBJECT
	
public:
	explicit GUITillMill(DM::Module * m, QWidget *parent = 0);
	~GUITillMill();
	
private:
	Ui::GUITillMill *ui;
	TileMill * m;
protected slots:
	void on_pushButton_view_clicked();
public slots:
	void accept();
};

#endif // GUITILLMILL_H
