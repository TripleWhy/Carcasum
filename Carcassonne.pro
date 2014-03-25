#-------------------------------------------------
#
# Project created by QtCreator 2014-02-09T23:35:48
#
#-------------------------------------------------

QT       += core

core {
#	QT       -= gui  #QTransform needs this

	TARGET = carcassonne_core

	CONFIG   += console
	CONFIG   -= app_bundle

	TEMPLATE = app

	SOURCES += \
		core/main.cpp

#	DEFINES += QT_NO_DEBUG_OUTPUT
#	DEFINES += QT_NO_WARNING_OUTPUT

} else {
	QT       += gui svg

	greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

	TARGET = carcassonne_gui
	TEMPLATE = app


	SOURCES += gui/main.cpp\
		gui/mainwindow.cpp \
		gui/tileimagefactory.cpp \
		gui/boardgraphicsview.cpp \
		gui/boardgraphicsscene.cpp \
		gui/playerinfoview.cpp

	HEADERS  += gui/mainwindow.h \
		gui/tileimagefactory.h \
		gui/boardgraphicsview.h \
		gui/boardgraphicsscene.h \
		gui/playerinfoview.h

	FORMS    += gui/mainwindow.ui \
		gui/playerinfoview.ui

	classicTiles {
		RESOURCES += jcz/jczTilesClassic.qrc
	} else {
		RESOURCES += gui/tilesJczf.qrc
	}

	RESOURCES += \
		gui/graphics.qrc

}

SOURCES += core/tile.cpp \
	core/game.cpp \
	core/board.cpp \
	core/util.cpp \
    player/randomplayer.cpp \
    jcz/xmlparser.cpp \
    jcz/tilefactory.cpp \
    jcz/location.cpp \
    jcz/expansion.cpp \
    player/montecarloplayer.cpp \
    core/random.cpp

HEADERS += \
	core/tile.h \
	core/game.h \
	core/board.h \
	core/player.h \
	core/util.h \
    player/randomplayer.h \
    jcz/tilefactory.h \
    jcz/xmlparser.h \
    jcz/location.h \
    jcz/expansion.h \
    player/montecarloplayer.h \
    static.h \
    core/random.h

RESOURCES += \
	jcz/jcz.qrc

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_WARN_ON += -Wextra -Werror=switch -Werror=return-type -Werror=delete-non-virtual-dtor

#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
