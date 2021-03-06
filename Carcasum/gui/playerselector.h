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

#ifndef PLAYERSELECTOR_H
#define PLAYERSELECTOR_H

#include "static.h"
#include "core/player.h"
#include "jcz/tilefactory.h"
#include "guiIncludes.h"
#include <QListWidgetItem>
#include <array>

namespace Ui {
class PlayerSelector;
}

class PlayerSelector : public QDialog
{
	Q_OBJECT

private:
	enum PlayerType { PlayerTypeRandom, PlayerTypeMonteCarlo, PlayerTypeMonteCarlo2, PlayerTypeMonteCarloUCT, PlayerTypeMCTS, PlayerTypeJCZ, PlayerSimple, PlayerSimple2, PlayerSimple3 };
	enum UtilityType { UtilityTypeSimple, UtilityTypeScoreDiff, UtilityTypeComplex, UtilityTypeComplexNormalized, UtilityTypePortion };
	enum PlayoutType { PlayoutTypeRandom, PlayoutTypeEarlyCutoff };

	struct PlayerData
	{
		PlayerType type;
		QString displayName;
		QString toolTip;
	};
	struct UtilityData
	{
		UtilityType type;
		QString displayName;
		QString toolTip;
	};
	struct PlayoutData
	{
		PlayoutType type;
		QString displayName;
		QString toolTip;
	};

	const std::array<PlayerData, 9> playerData = {{ {PlayerTypeRandom, "Random", ""},
	                                                {PlayerSimple, "SimplePlayer v1", ""},
	                                                {PlayerSimple2, "SimplePlayer v2", ""},
	                                                {PlayerSimple3, "SimplePlayer v3", ""},
	                                                {PlayerTypeJCZ, "JCloisterZone AI", ""},
	                                                {PlayerTypeMonteCarlo, "Monte Carlo", ""},
	                                                {PlayerTypeMonteCarlo2, "Monte Carlo 2", ""},
	                                                {PlayerTypeMonteCarloUCT, "Monte Carlo UCB1", ""},
	                                                {PlayerTypeMCTS, "MCTS", ""}
	                                              }};
	const std::array<UtilityData, 5> utilityData = {{
	                                                    {UtilityTypePortion, "Portion", ""},
	                                                    {UtilityTypeSimple, "Simple", ""},
	                                                    {UtilityTypeScoreDiff, "Score Difference", ""},
	                                                    {UtilityTypeComplex, "Complex", ""},
	                                                    {UtilityTypeComplexNormalized, "Complex Normalized", ""},
	                                                }};
	const std::array<PlayoutData, 2> playoutData = {{ {PlayoutTypeRandom, "Random", ""},
	                                                  {PlayoutTypeEarlyCutoff, "Early Cutoff", ""}
	                                                }};
	jcz::TileFactory * tileFactory;

public:
	explicit PlayerSelector(jcz::TileFactory * tileFactory, QWidget *parent = 0);
	~PlayerSelector();
	Player * createPlayer();
	QString playerDisplayName();

private slots:
	void on_typeList_currentRowChanged(int currentRow);
	void on_timeLimitRB_toggled(bool checked);

	void on_typeList_itemSelectionChanged();

private:
	Ui::PlayerSelector *ui;
};

#endif // PLAYERSELECTOR_H
