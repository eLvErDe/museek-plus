# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'privatechat.ui'
#
# Created: Fri Jul  3 17:50:21 2009
#      by: PyQt4 UI code generator 4.5.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_Private(object):
    def setupUi(self, Private):
        Private.setObjectName("Private")
        Private.resize(400, 300)
        self.verticalLayout_2 = QtGui.QVBoxLayout(Private)
        self.verticalLayout_2.setMargin(0)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setObjectName("verticalLayout")
        self.ChatLog = QtGui.QTextEdit(Private)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(2)
        sizePolicy.setHeightForWidth(self.ChatLog.sizePolicy().hasHeightForWidth())
        self.ChatLog.setSizePolicy(sizePolicy)
        self.ChatLog.setMinimumSize(QtCore.QSize(0, 0))
        self.ChatLog.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.ChatLog.setBaseSize(QtCore.QSize(0, 400))
        self.ChatLog.setFocusPolicy(QtCore.Qt.NoFocus)
        self.ChatLog.setReadOnly(True)
        self.ChatLog.setObjectName("ChatLog")
        self.verticalLayout.addWidget(self.ChatLog)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.ChatEntry = QtGui.QLineEdit(Private)
        self.ChatEntry.setObjectName("ChatEntry")
        self.horizontalLayout.addWidget(self.ChatEntry)
        self.Close = QtGui.QPushButton(Private)
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Close.sizePolicy().hasHeightForWidth())
        self.Close.setSizePolicy(sizePolicy)
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap("icons/cancel.png"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.Close.setIcon(icon)
        self.Close.setObjectName("Close")
        self.horizontalLayout.addWidget(self.Close)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.verticalLayout_2.addLayout(self.verticalLayout)

        self.retranslateUi(Private)
        QtCore.QMetaObject.connectSlotsByName(Private)

    def retranslateUi(self, Private):
        Private.setWindowTitle(QtGui.QApplication.translate("Private", "Private", None, QtGui.QApplication.UnicodeUTF8))
        self.Close.setText(QtGui.QApplication.translate("Private", "Close", None, QtGui.QApplication.UnicodeUTF8))

