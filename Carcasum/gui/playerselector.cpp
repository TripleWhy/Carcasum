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
	double Cp = ui->CpSpinBox->value();

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
							return new MonteCarloPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::SimpleUtility, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
					}
				}
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer<Utilities::HeydensUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
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
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayer2<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayer2<Utilities::HeydensUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
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
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MonteCarloPlayerUCT<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout);
						case PlayoutTypeEarlyCutoff:
							return new MonteCarloPlayerUCT<Utilities::HeydensUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout);
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
							return new MCTSPlayer<Utilities::SimpleUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::SimpleUtility, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout, Cp);
					}
				}
				case UtilityTypeHeyden:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::HeydensUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::HeydensUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout, Cp);
					}
				}
				case UtilityTypeComplex:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ComplexUtility, Playouts::RandomPlayout>(tileFactory, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ComplexUtilityEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout, Cp);
					}
				}
				case UtilityTypeComplexNormalized:
				{
					switch (playout)
					{
						case PlayoutTypeRandom:
							return new MCTSPlayer<Utilities::ComplexUtilityNormalized, Playouts::RandomPlayout>(tileFactory, limit, useTimeout, Cp);
						case PlayoutTypeEarlyCutoff:
							return new MCTSPlayer<Utilities::ComplexUtilityNormalizedEC, Playouts::EarlyCutoff<>>(tileFactory, limit, useTimeout, Cp);
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
		ui->limitUnitLabel->setText(tr("ms", "player resource limit: unit milliseconds"));
	else
		ui->limitUnitLabel->setText(tr("playouts", "player resource limit: unit playouts"));
}

void PlayerSelector::on_typeList_itemSelectionChanged()
{
	ui->okBox->setEnabled(ui->typeList->selectedItems().size() > 0);
}
