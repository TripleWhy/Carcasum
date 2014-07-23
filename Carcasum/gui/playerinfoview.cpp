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
		meepleLabels[i] = new PIVLabel();
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

#if PLAYERINFOVIEW_SCALEABLE
	int constexpr scaleBonus = 60;
	QPixmap icon = imgFactory->generateMeepleStanding(scaleBonus * PINFO_ICON_SIZE, imgFactory->getPlayerColor(player));
	QPixmap meeple = imgFactory->generateMeepleStanding(scaleBonus * PINFO_MEEPLE_SIZE, imgFactory->getPlayerColor(player));
	ui->iconLabel->setPixmap(icon);
	ui->iconLabel->setFixedSize(icon.size() / scaleBonus);
	ui->iconLabel->setScaledContents(true);

	for (int i = 0; i < MEEPLE_COUNT; ++i)
	{
		meepleLabels[i]->setFixedSize(meeple.size() / scaleBonus);
		meepleLabels[i]->setScaledContents(true);
		meepleLabels[i]->setVisible(true);
		meepleLabels[i]->setPixmap(meeple);
	}
#else
	QPixmap icon = imgFactory->generateMeepleStanding(PINFO_ICON_SIZE, imgFactory->getPlayerColor(player));
	QPixmap meeple = imgFactory->generateMeepleStanding(PINFO_MEEPLE_SIZE, imgFactory->getPlayerColor(player));
	ui->iconLabel->setPixmap(icon);
	ui->iconLabel->setFixedSize(icon.size());

	for (int i = 0; i < MEEPLE_COUNT; ++i)
	{
		meepleLabels[i]->setPixmap(meeple);
		meepleLabels[i]->setFixedSize(meeple.size());
		meepleLabels[i]->setVisible(true);
	}
#endif

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
#if PLAYERINFOVIEW_SCALEABLE
		ui->tileLabel->setPixmap(p);
		ui->tileLabel->setFixedSize(RTILE_TILE_SIZE, RTILE_TILE_SIZE);
		ui->tileLabel->setScaledContents(true);
#else
		ui->tileLabel->setPixmap( p.scaled(RTILE_TILE_SIZE, RTILE_TILE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation) );
#endif
	}
	else
		ui->tileLabel->clear();
}
