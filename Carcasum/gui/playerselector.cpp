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

#include "playerselector.h"
#include "ui_playerselector.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/montecarloplayer2.h"
#include "player/montecarloplayeruct.h"
#include "player/mctsplayer.h"
#include "player/simpleplayer.h"
#include "player/simpleplayer2.h"
#include "player/simpleplayer3.h"
#include "jcz/jczplayer.h"

PlayerSelector::PlayerSelector(jcz::TileFactory * tileFactory, QWidget *parent)
    : QDialog(parent),
      tileFactory(tileFactory),
      ui(new Ui::PlayerSelector)
{
	ui->setupUi(this);

	ui->typeList->clear();
	for (PlayerData const & d : playerData)
		ui->typeList->addItem(d.displayName);

	ui->utilityBox->clear();
	for (auto const & d : utilityData)
		ui->utilityBox->addItem(d.displayName);

	ui->playoutBox->clear();
	for (auto const & d : playoutData)
		ui->playoutBox->addItem(d.displayName);
}

PlayerSelector::~PlayerSelector()
{
	delete ui;
}

Player *PlayerSelector::createPlayer()
{
	PlayerType player = playerData[ui->typeList->currentRow()].type;
	UtilityType utility = utilityData[ui->utilityBox->currentIndex()].type;
	PlayoutType playout = playoutData[ui->playoutBox->currentIndex()].type;
	bool useTimeout = ui->timeLimitRB->isChecked();
	int limit = ui->limitSpinBox->value();
	double Cp = ui->CpSpinBox->value();

	switch (player)
	{
		case PlayerTypeRandom:
			return new RandomPlayer();
		case PlayerSimple:
			return new SimplePlayer();
		case PlayerSimple2:
			return new SimplePlayer2();
		case PlayerSimple3:
			return new SimplePlayer3();
		case PlayerTypeJCZ:
			return new jcz::JCZPlayer(tileFactory);
		case PlayerTypeMonteCarlo:
		{
			switch (utility)
			{
				case UtilityTypeSimple:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::SimpleUtility, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeScoreDiff:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::ScoreDifferenceUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::ScoreDifferenceUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::ComplexUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::ComplexUtilityNormalizedEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypePortion:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
			}
			break;
		}
		case PlayerTypeMonteCarlo2:
		{
			switch (utility)
			{
				case UtilityTypeSimple:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::SimpleUtility, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeScoreDiff:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::ScoreDifferenceUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::ScoreDifferenceUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::ComplexUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::ComplexUtilityNormalizedEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypePortion:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
			}
			break;
		}
		case PlayerTypeMonteCarloUCT:
		{
			switch (utility)
			{
				case UtilityTypeSimple:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::SimpleUtility, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeScoreDiff:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::ScoreDifferenceUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::ScoreDifferenceUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtilityNormalizedEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypePortion:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
			}
			break;
		}
		case PlayerTypeMCTS:
		{
			switch (utility)
			{
				case UtilityTypeSimple:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, false, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::SimpleUtility, Playouts::EarlyCutoff<>>(tileFactory, false, limit, useTimeout, Cp);
					}
				}
				case UtilityTypeScoreDiff:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ScoreDifferenceUtility, Playouts::RandomPlayout>(tileFactory, false, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ScoreDifferenceUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, false, limit, useTimeout, Cp);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory, false, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ComplexUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, false, limit, useTimeout, Cp);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory, false, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ComplexUtilityNormalizedEC, Playouts::EarlyCutoff<>>(tileFactory, false, limit, useTimeout, Cp);
					}
				}
				case UtilityTypePortion:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::PortionUtility, Playouts::RandomPlayout>(tileFactory, false, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::EC<Utilities::PortionUtility>, Playouts::EarlyCutoff<>>(tileFactory, false, limit, useTimeout, Cp);
					}
				}
			}
			break;
		}
	}
	return 0;
}

QString PlayerSelector::playerDisplayName()
{
	return playerData[ui->typeList->currentRow()].displayName;
}

void PlayerSelector::on_typeList_currentRowChanged(int currentRow)
{
	switch (playerData[currentRow].type)
	{
		case PlayerTypeRandom:
		case PlayerSimple:
		case PlayerSimple2:
		case PlayerSimple3:
		case PlayerTypeJCZ:
			ui->optionsWidget->setVisible(false);
			break;
		case PlayerTypeMCTS:
			ui->optionsWidget->setVisible(true);
			ui->cpWidget->setVisible(true);
			break;
		default:
			ui->optionsWidget->setVisible(true);
			ui->cpWidget->setVisible(false);
			break;
	}
}

void PlayerSelector::on_timeLimitRB_toggled(bool checked)
{
	if (checked)
		ui->limitUnitLabel->setText(tr("ms", "player resource limit: unit milliseconds"));
	else
		ui->limitUnitLabel->setText(tr("playouts", "player resource limit: unit playouts"));
}

void PlayerSelector::on_typeList_itemSelectionChanged()
{
	ui->okBox->setEnabled(ui->typeList->selectedItems().size() > 0);
}
