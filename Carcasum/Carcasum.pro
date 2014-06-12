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
	TARGET = carcasum_tournament

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
		gui/playerselector.cpp \
		gui/renderoptionsdialog.cpp

	HEADERS  += gui/mainwindow.h \
		gui/tileimagefactory.h \
		gui/boardgraphicsview.h \
		gui/boardgraphicsscene.h \
		gui/playerinfoview.h \
		gui/remainingtilesview.h \
		gui/remainingtileview.h \
		gui/guiIncludes.h \
		gui/downloader.h \
		gui/playerselector.h \
		gui/renderoptionsdialog.h

	FORMS    += gui/mainwindow.ui \
		gui/playerinfoview.ui \
		gui/remainingtilesview.ui \
		gui/remainingtileview.ui \
		gui/downloader.ui \
		gui/playerselector.ui \
		gui/renderoptionsdialog.ui

	classicTiles {
		DEFINES += CLASSIC_TILES
		RESOURCES += jcz/jczTilesClassic.qrc
	} else {
		RESOURCES += gui/tilesJczf.qrc

		win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../quazip/release/ -lquazip
		else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../quazip/debug/ -lquazip
		else:unix: LIBS += -L$$OUT_PWD/../quazip/ -lquazip

		INCLUDEPATH += $$PWD/../
		DEPENDPATH += $$PWD/../quazip
	}

	RESOURCES += \
		gui/graphics.qrc

	TRANSLATIONS = carcasum_en.ts \
		carcasum_de.ts \
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
    player/montecarloplayeruct.tpp \
    player/mctsplayer.tpp \
#    player/mctsplayer1.tpp \
    player/playouts.cpp \
    player/utilities.cpp \
    jcz/jczplayer.cpp \
    player/simpleplayer.cpp \
    player/simpleplayer2.cpp \
    player/simpleplayer3.cpp \
    player/specialplayers.cpp

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
#    player/mctsplayer1.h \
    player/montecarloplayer2.h \
    player/montecarloplayeruct.h \
    player/playouts.h \
    player/utilities.h \
    jcz/jczplayer.h \
    player/simpleplayer.h \
    player/simpleplayer2.h \
    player/simpleplayer3.h \
    player/specialplayers.h

RESOURCES += \
	jcz/jcz.qrc

REVISION = $$system(git rev-parse HEAD)
DEFINES += APP_REVISION=$$REVISION

CONFIG += c++11
QMAKE_CXXFLAGS_WARN_ON += -Wextra -Werror=switch -Werror=return-type -Werror=delete-non-virtual-dtor -Wconversion
QMAKE_CFLAGS_RELEASE   += -O3 -march=native -mtune=native -funroll-loops
QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -mtune=native -funroll-loops
QMAKE_LFLAGS_RELEASE   += -O3 -march=native -mtune=native -funroll-loops
#QMAKE_CFLAGS_RELEASE   += -O3 -march=native -mtune=native -funroll-loops -fprofile-generate
#QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -mtune=native -funroll-loops -fprofile-generate
#QMAKE_LFLAGS_RELEASE   += -O3 -march=native -mtune=native -funroll-loops -fprofile-generate
#QMAKE_CFLAGS_RELEASE   += -O3 -march=native -mtune=native -funroll-loops -fprofile-use
#QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -mtune=native -funroll-loops -fprofile-use
#QMAKE_LFLAGS_RELEASE   += -O3 -march=native -mtune=native -funroll-loops -fprofile-use

#DEFINES += QT_FORCE_ASSERTS
#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g


##This builds qm files. TODO: It also deletes them again on make clean.
#isEmpty(QMAKE_LRELEASE) {
#	win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
#	else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
##	else:QMAKE_LRELEASE = lrelease-qt4 # Yaay, fedora!
#}
#updateqm.input = TRANSLATIONS
##updateqm.output = $$OUT_PWD/${QMAKE_FILE_BASE}.qm
##updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm $$OUT_PWD/${QMAKE_FILE_BASE}.qm
#updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
#updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
#updateqm.CONFIG += no_link
#
#QMAKE_EXTRA_COMPILERS += updateqm
#PRE_TARGETDEPS += compiler_updateqm_make_all

unix|win32: LIBS += -lboost_system -lboost_timer -lboost_chrono
