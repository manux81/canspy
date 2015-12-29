#
#  canspy - A simple tool for users who need to interface with a device based on
#           CAN (CAN/CANopen/J1939/NMEA2000/DeviceNet) such as motors,
#           sensors and many other devices.
#  Copyright (C) 2015-2016  Manuele Conti (manuele.conti@gmail.com)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
# This code is made available on the understanding that it will not be
# used in safety-critical situations without a full and competent review.
#


QT += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = canspy
TEMPLATE = app

INCLUDEPATH = ./include

BUILD_DIRECTORY = ./build

# Define the preprocessor macro to get the application version in our application.
DEFINES += __STDC_FORMAT_MACROS

RCC_DIR = $$BUILD_DIRECTORY/RCCFiles
UI_DIR = $$BUILD_DIRECTORY/UICFiles
MOC_DIR = $$BUILD_DIRECTORY/MOCFiles
OBJECTS_DIR = $$BUILD_DIRECTORY/ObjFiles


SOURCES += src/main.cxx \
           src/mainwindow.cxx \
           src/qcanbuffer.cxx \
           src/qcanrecvthread.cxx \
           src/qcansendthread.cxx \
           src/qcansocket.cxx \
           src/configdialog.cxx \
           src/qappsettings.cxx \
           src/qdelegatecolor.cxx \
           src/logmodel.cxx \
           src/qcanpkgabstractmodel.cxx \
           src/qcanmonitor.cxx \
           src/can_drv.cxx \
           src/msgseq.cxx \
           src/drivers/linux/net_ops.cxx \
           src/drivers/general/simulation_ops.cxx

HEADERS  += include/mainwindow.h \
            include/canbus/can_drv.h \
            include/canbus/can_state.h \
            include/canbus/can_packet.h \
            include/canbus/simulation_ops.h \
            include/canbus/net_ops.h \
            include/qcanbuffer.h \
            include/qcanrecvthread.h \
            include/qcansendthread.h \
            include/qcansocket.h \
            include/configdialog.h \
            include/qappsettings.h \
            include/qdelegatecolor.h \
            include/logmodel.h \
            include/qcanpkgabstractmodel.h \
            include/qcanpacketconsumer.h \
            include/qcanmonitor.h \
            include/utils.h \
            include/msgseq.h


FORMS    += forms/mainwindow.ui \
            forms/configdialog.ui \
            forms/msgseq.ui

linux-g++ {
SOURCES += \
        src/osdep/linux/can_socket_ops.cxx \
        src/osdep/linux/utils_linux.cxx

HEADERS += \
        include/canbus/can_socket_ops.h

LIBS += -lsocketcan
SCRIPT_NAME = version.sh

version.target = include/version.h
version.commands = echo "Creating version header..."; \
                   $$PWD/script/$$SCRIPT_NAME $$PWD
version.depends = .

QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += include/version.h
}

RESOURCES += \
    images/canspy.qrc


win32 {
SOURCES += \
        src/osdep/windows/utils_windows.cxx \
        src/drivers/windows/ixxat_ops.cxx \
        src/drivers/windows/usb2can_ops.cxx

HEADERS += \
        include/osdep/canal.h \
        include/canbus/ixxat_ops.h \
        include/canbus/usb2can_ops.h

LIBS += -lusb2can -lvcisdk -lWs2_32 -L./lib/win -L$(IXXAT_VCI_SDK)/lib/ia32

RC_FILE = canspy.rc

INCLUDEPATH += $(IXXAT_VCI_SDK)/inc

DEFINES +=_CRT_SECURE_NO_WARNINGS \
          "WINVER=0x0501" \
          "_WIN32_WINNT=0x0501"

QMAKE_PRE_BUILD = call $(SolutionDir)\script\version.bat
QMAKE_PRE_LINK = call $(SolutionDir)\script\version.bat
}








