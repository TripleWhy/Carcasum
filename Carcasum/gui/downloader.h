/*
	This file is part of Carcasum.

	Carcasum is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Carcasum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with Carcasum.  If not, see <http://www.gnu.org/licenses/>.
*/

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
