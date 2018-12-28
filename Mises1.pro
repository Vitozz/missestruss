#-------------------------------------------------
#
# Project created by QtCreator 2015-12-07T14:44:02
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets svg

TARGET = qmisestruss
TEMPLATE = app

PREFIX = /usr
BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share

CONFIG(debug, debug|release) : DEFINES += IS_DEBUG

win32 {
	QWTLIB = -L$$QWTDIR/lib -lqwt
	CONFIG(debug, debug|release) : QWTLIB = -L$$QWTDIR/lib -lqwtd
	INCLUDEPATH += $$QWTDIR \
			$$QWTDIR/include \
			$$QWTDIR/lib
	LIBS += $$QWTLIB
	RC_FILE = Mises.rc
}

CONFIG += qwt.prf

SOURCES += main.cpp\
            core/misescalcelast.cpp\
            core/misescalcnelast.cpp\
            gui/mainwindow.cpp

HEADERS  += gui/mainwindow.h \
            core/misescalcelast.h\
            core/misescalcnelast.h\
            defines.h

FORMS    += gui/mainwindow.ui

RESOURCES += mises1.qrc

LANG_PATH = langs
TRANSLATIONS = $$LANG_PATH/qmisestruss_ru.ts \
		$$LANG_PATH/qmisestruss_uk.ts
unix {
	target.path = $$BINDIR
	INSTALLS += target
	df.path = $$DATADIR/applications/
	df.files = misestruss.desktop
	translations.path = $$DATADIR/qmisestruss/langs
	translations.extra = lrelease Mises1.pro && cp -f $$LANG_PATH/qmisestruss_*.qm  $(INSTALL_ROOT)$$translations.path
	icon.path =$$DATADIR/qmisestruss/icons
	icon.files = icons/f(x)48.png
	INSTALLS += df \
		    translations \
		    icon
}
