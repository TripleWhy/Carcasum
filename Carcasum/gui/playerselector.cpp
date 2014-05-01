#include "playerselector.h"
#include "ui_playerselector.h"
#include "player/randomplayer.h"
#include "player/montecarloplayer.h"
#include "player/montecarloplayer2.h"
#include "player/montecarloplayeruct.h"
#include "player/mctsplayer.h"

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

	switch (player)
	{
		case PlayerTypeRandom:
			return new RandomPlayer();
		case PlayerTypeMonteCarlo:
		{
			switch (utility)
			{
				case UtilityTypeSimple:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::SimpleUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::HeydensUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::ComplexUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::ComplexUtilityNormalized, Playouts::EarlyCutoff<10>>(tileFactory);
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
							return new MonteCarloPlayer2<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::SimpleUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::HeydensUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::ComplexUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::ComplexUtilityNormalized, Playouts::EarlyCutoff<10>>(tileFactory);
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
							return new MonteCarloPlayerUCT<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::SimpleUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::HeydensUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtility, Playouts::EarlyCutoff<10>>(tileFactory);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::ComplexUtilityNormalized, Playouts::EarlyCutoff<10>>(tileFactory);
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
							return new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::SimpleUtility, Playouts::EarlyCutoff<10>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::HeydensUtility, Playouts::EarlyCutoff<10>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ComplexUtility, Playouts::EarlyCutoff<10>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ComplexUtilityNormalized, Playouts::EarlyCutoff<10>>(tileFactory, limit, useTimeout);
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
		ui->limitUnitLabel->setText("ms");
	else
		ui->limitUnitLabel->setText("playouts");
}

void PlayerSelector::on_typeList_itemSelectionChanged()
{
	ui->okBox->setEnabled(ui->typeList->selectedItems().size() > 0);
}
