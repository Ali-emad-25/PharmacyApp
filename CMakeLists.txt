cmake_minimum_required(VERSION 3.19)
project(PharmacyApp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

qt_add_executable(PharmacyApp
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    style.qss
    database.h
    database.cpp
    medicine.h
    medicine.cpp
    actionwindow.h
    actionwindow.cpp
    action.cpp
    action.h
    homewindow.cpp
    homewindow.h
    purchaseswindow.cpp
    purchaseswindow.h
    purchases.h
    purchases.cpp
    purchaseitem.h
    purchaseitem.cpp




)

target_link_libraries(PharmacyApp
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

install(TARGETS PharmacyApp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET PharmacyApp
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
