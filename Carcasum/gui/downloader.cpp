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
