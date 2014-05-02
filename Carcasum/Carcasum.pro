#-------------------------------------------------
#
# Project created by QtCreator 2014-02-09T23:35:48
#
#-------------------------------------------------

QT       += core

TEMPLATE = app

core {
#	QT       -= gui  #QTransform needs this

	TARGET = carcasum_core

	CONFIG   += console
	CONFIG   -= app_bundle

	SOURCES += \
		core/main.cpp

#	DEFINES += QT_NO_DEBUG_OUTPUT
#	DEFINES += QT_NO_WARNING_OUTPUT

} else:tournament {
	TARGET = carcasum_turnement

	CONFIG   += console
	CONFIG   -= app_bundle

	SOURCES += \
		tournament/main.cpp

} else {
	QT       += gui svg

	greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

	TARGET = carcasum_gui

	SOURCES += gui/main.cpp\
		gui/mainwindow.cpp \
		gui/tileimagefactory.cpp \
		gui/boardgraphicsview.cpp \
		gui/boardgraphicsscene.cpp \
		gui/playerinfoview.cpp \
		gui/remainingtilesview.cpp \
		gui/remainingtileview.cpp \
		gui/downloader.cpp \
		gui/playerselector.cpp

	HEADERS  += gui/mainwindow.h \
		gui/tileimagefactory.h \
		gui/boardgraphicsview.h \
		gui/boardgraphicsscene.h \
		gui/playerinfoview.h \
		gui/remainingtilesview.h \
		gui/remainingtileview.h \
		gui/guiIncludes.h \
		gui/downloader.h \
		gui/playerselector.h

	FORMS    += gui/mainwindow.ui \
		gui/playerinfoview.ui \
		gui/remainingtilesview.ui \
		gui/remainingtileview.ui \
		gui/downloader.ui \
		gui/playerselector.ui

	classicTiles {
		RESOURCES += jcz/jczTilesClassic.qrc
	} else {
		RESOURCES += gui/tilesJczf.qrc
	}

	RESOURCES += \
		gui/graphics.qrc

	win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../quazip/release/ -lquazip
	else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../quazip/debug/ -lquazip
	else:unix: LIBS += -L$$OUT_PWD/../quazip/ -lquazip

	INCLUDEPATH += $$PWD/../
	DEPENDPATH += $$PWD/../quazip
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
    player/montecarloplayer.tpp \
    core/random.cpp \
    core/nexttileprovider.cpp \
    player/montecarloplayer2.tpp \
    player/montecarloplayeruct.tpp \ #TODO where?
    player/mctsplayer.cpp \
    player/playouts.cpp \
    player/utilities.cpp

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
    core/random.h \
    core/nexttileprovider.h \
    player/mctsplayer.h \
    player/montecarloplayer2.h \
    player/montecarloplayeruct.h \
    player/mctsplayer.tpp \
    player/playouts.h \
    player/utilities.h

RESOURCES += \
	jcz/jcz.qrc

REVISION = $$system(git rev-parse HEAD)
DEFINES += APP_REVISION=$$REVISION

CONFIG += c++11
QMAKE_CXXFLAGS_WARN_ON += -Wextra -Werror=switch -Werror=return-type -Werror=delete-non-virtual-dtor -Wconversion

#DEFINES += QT_FORCE_ASSERTS
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g