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

#include "downloader.h"
#include "ui_downloader.h"

Downloader::Downloader(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Downloader)
{
	ui->setupUi(this);

	connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)), SLOT(fileDownloaded(QNetworkReply*)));
}

Downloader::~Downloader()
{
	delete ui;
}

void Downloader::setLabel(QString text)
{
	ui->label->setText(text);
}

void Downloader::download(QString url, QString label)
{
	done = false;
	QNetworkRequest request(url);
	reply = m_WebCtrl.get(request);
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(setDownloadProgress(qint64,qint64)));

	setLabel(label);
	exec();

//	while (!done)
//	{
//		QCoreApplication::processEvents();
//	}
}

QByteArray Downloader::downloadedData() const
{
	return m_DownloadedData;
}

void Downloader::setDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	ui->progressBar->setMaximum((int)bytesTotal);
	ui->progressBar->setValue((int)bytesReceived);
}

void Downloader::fileDownloaded(QNetworkReply * pReply)
{
	m_DownloadedData = pReply->readAll();
	pReply->deleteLater();
	done = true;
	accept();
}
