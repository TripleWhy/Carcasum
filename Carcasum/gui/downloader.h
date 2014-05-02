#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "guiIncludes.h"
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace Ui {
class Downloader;
}

class Downloader : public QDialog
{
	Q_OBJECT

private:
	QNetworkAccessManager m_WebCtrl;
	QNetworkReply * reply;
	QByteArray m_DownloadedData;
	bool done = false;

public:
	explicit Downloader(QWidget *parent = 0);
	~Downloader();
	void setLabel(QString text);

	void download(QString url, QString label);
	QByteArray downloadedData() const;

private slots:
	void setDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void fileDownloaded(QNetworkReply* pReply);

private:
	Ui::Downloader *ui;
};

#endif // DOWNLOADER_H
