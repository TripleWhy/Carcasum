#include "playerinfoview.h"
#include "ui_playerinfoview.h"

#include "core/game.h"

PlayerInfoView::PlayerInfoView(QWidget * parent)
    : QWidget(parent),
ui(new Ui::PlayerInfoView)
{
	ui->setupUi(this);
	ui->meepleSpacer->changeSize(0, PINFO_MEEPLE_SIZE, QSizePolicy::Expanding, QSizePolicy::Fixed);
	setAutoFillBackground(true);
	normPalette = palette();
	hlPalette = normPalette;
	hlPalette.setColor(QPalette::Background, QColor::fromRgb(255, 255, 255, 96));
	
	for (int i = 0; i < MEEPLE_COUNT; ++i)
	{
		meepleLabels[i] = new QLabel();
		ui->meepleLayout->insertWidget(i, meepleLabels[i]);
	}
}

PlayerInfoView::PlayerInfoView(int player, Game const * game, TileImageFactory * tif, QWidget *parent)
    : PlayerInfoView(parent)
{
	setPlayer(player, game, tif);
}

PlayerInfoView::~PlayerInfoView()
{
	delete ui;
}

void PlayerInfoView::setPlayer(int player, Game const * g, TileImageFactory * tif)
{
	playerIndex = player;
	game = g;
	imgFactory = tif;

	ui->nameLabel->setText(imgFactory->getPlayerName(player));

	QPixmap icon = imgFactory->generateMeepleStanding(PINFO_ICON_SIZE, imgFactory->getPlayerColor(player));
	QPixmap meeple = imgFactory->generateMeepleStanding(PINFO_MEEPLE_SIZE, imgFactory->getPlayerColor(player));
	
	ui->iconLabel->setPixmap(icon);
	ui->iconLabel->setMinimumSize(icon.size());
	ui->iconLabel->setMaximumSize(icon.size());
	for (int i = 0; i < MEEPLE_COUNT; ++i)
	{
		meepleLabels[i]->setPixmap(meeple);
		meepleLabels[i]->setMinimumSize(meeple.size());
		meepleLabels[i]->setMaximumSize(meeple.size());
		meepleLabels[i]->setVisible(true);
	}

	if (g->getNextPlayer() == playerIndex)
		setPalette(hlPalette);
	else
		setPalette(normPalette);
}

void PlayerInfoView::updateView()
{
	int meeples = game->getPlayerMeeples(playerIndex);
	for (int i = 0; i < meeples; ++i)
		meepleLabels[i]->setVisible(true);
	for (int i = meeples; i < MEEPLE_COUNT; ++i)
		meepleLabels[i]->setVisible(false);
	ui->pointsLabel->setText(QString::number(game->getPlayerScore(playerIndex)));
	ui->meeplesLabel->setText(QString::number(meeples));

	if (game->getNextPlayer() == playerIndex)
		setPalette(hlPalette);
	else
		setPalette(normPalette);
}

void PlayerInfoView::displayTile(int player, int tileType)
{
	if (playerIndex == player)
	{
		QPixmap const & p = imgFactory->getImage((TileTypeType)tileType);
		ui->tileLabel->setPixmap( p.scaled(RTILE_TILE_SIZE, RTILE_TILE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation) );
	}
	else
		ui->tileLabel->clear();
}
