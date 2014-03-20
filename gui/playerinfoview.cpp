#include "playerinfoview.h"
#include "ui_playerinfoview.h"

#include "core/game.h"

PlayerInfoView::PlayerInfoView(QWidget * parent)
    : QWidget(parent),
ui(new Ui::PlayerInfoView)
{
	ui->setupUi(this);
	
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

	QPixmap icon = imgFactory->generateMeepleStanding(ICON_SIZE, imgFactory->getPlayerColor(player));
	QPixmap meeple = imgFactory->generateMeepleStanding(MEEPLE_SIZE, imgFactory->getPlayerColor(player));
	
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
}
