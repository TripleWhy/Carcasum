#-------------------------------------------------
#
# Project created by QtCreator 2014-02-09T23:35:48
#
#-------------------------------------------------

QT       += core

core {
	QT       -= gui

	TARGET = carcassonne_core

	CONFIG   += console
	CONFIG   -= app_bundle

	TEMPLATE = app

} else {
	QT       += gui

	greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

	TARGET = carcassonne_gui
	TEMPLATE = app


	SOURCES += gui/main.cpp\
		gui/mainwindow.cpp \
		gui/tileimagefactory.cpp \
		gui/boardgraphicsview.cpp \
		gui/boardgraphicsscene.cpp

	HEADERS  += gui/mainwindow.h \
		gui/tileimagefactory.h \
		gui/boardgraphicsview.h \
		gui/boardgraphicsscene.h

	FORMS    += gui/mainwindow.ui

	classicTiles {
		RESOURCES += jcz/jczTilesClassic.qrc
	} else {
		RESOURCES += gui/tilesJczf.qrc
	}


}

SOURCES += core/tile.cpp \
	core/game.cpp \
	core/board.cpp \
	core/util.cpp \
    player/randomplayer.cpp \
	jcz/jczutils.cpp

HEADERS += \
	core/tile.h \
	core/game.h \
	core/board.h \
	core/player.h \
	core/util.h \
    player/randomplayer.h \
	jcz/jczutils.h

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_WARN_ON += -Wextra -Werror=switch -Werror=return-type -Werror=delete-non-virtual-dtor

RESOURCES += \
	jcz/jcz.qrc
