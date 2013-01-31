#ifndef LOCALSETTINGSDIALOG_H
#define LOCALSETTINGSDIALOG_H

#pragma warning(push, 3)
#pragma warning(disable:4251)
#include <QDialog>
#include <QMap>
#include "ui_localsettingsdialog.h"
#pragma warning(pop)

class LocalSettingsDialog : public QDialog, private Ui::LocalSettingsDialog
{
	Q_OBJECT

public:
    LocalSettingsDialog(QMap<QString, QVariant> settings, QWidget* parent = 0);
	~LocalSettingsDialog();

signals:
	void settingsChanged(QMap<QString, QVariant> settings);

private slots:
	void showFileDialog();
	virtual void accept();

private:
	// Object used to send the new settings
    QMap<QString, QVariant> mSettings;
};

#endif // LOCALSETTINGSDIALOG_H
