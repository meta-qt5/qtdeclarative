TEMPLATE=app
TARGET=tst_qmltest
CONFIG += qmltestcase
SOURCES += tst_qmltest.cpp


importFiles.files = borderimage  buttonclick  createbenchmark  events  qqmlbinding selftests

importFiles.path = .
DEPLOYMENT += importFiles

mac:CONFIG+=insignificant_test # QTBUG-25306
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
