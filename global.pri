
!include("user.pri") { }

CONFIG(SAS_BACKTRACE) {
    QMAKE_CXXFLAGS = -g1
}
