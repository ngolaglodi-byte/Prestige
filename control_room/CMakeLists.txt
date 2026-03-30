project(prestige_control LANGUAGES CXX)

set(CONTROL_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PreviewMonitor.cpp
    src/OverlayController.cpp
    src/ConfigManager.cpp
    src/EmissionProfile.cpp
    src/SetupController.cpp
    src/LiveController.cpp
    src/StyleRegistry.cpp
    src/TalentManager.cpp
    src/RssFetcher.cpp
    src/SubtitleController.cpp
    src/WeatherFetcher.cpp
    src/MacroEngine.cpp
    src/AudioMeter.cpp
    src/SocialChatController.cpp
    src/WebRemoteServer.cpp
    src/LicenseManager.cpp
)

set(CONTROL_HEADERS
    src/MainWindow.h
    src/PreviewMonitor.h
    src/OverlayController.h
    src/ConfigManager.h
    src/EmissionProfile.h
    src/SetupController.h
    src/LiveController.h
    src/StyleRegistry.h
    src/TalentManager.h
    src/RssFetcher.h
    src/SubtitleController.h
    src/WeatherFetcher.h
    src/MacroEngine.h
    src/AudioMeter.h
    src/SocialChatController.h
    src/WebRemoteServer.h
    src/LicenseManager.h
)

qt_add_executable(prestige_control ${CONTROL_SOURCES} ${CONTROL_HEADERS}
    resources/icons.qrc
)

# ── macOS bundle icon ─────────────────────────────────────
if(APPLE)
    set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../resources/prestige.icns)
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(prestige_control PRIVATE ${ICON_FILE})
    set_target_properties(prestige_control PROPERTIES
        MACOSX_BUNDLE_ICON_FILE prestige.icns
        MACOSX_BUNDLE_GUI_IDENTIFIER com.prestigeai.controlroom
        MACOSX_BUNDLE_BUNDLE_NAME "Prestige AI"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
endif()

# ── QML module ──────────────────────────────────────────────
qt_add_qml_module(prestige_control
    URI PrestigeControl
    VERSION 1.0
    QML_FILES
        qml/MainView.qml
        qml/SetupView.qml
        qml/LiveView.qml
        qml/StyleStep.qml
        qml/TalentStep.qml
        qml/SourceStep.qml
        qml/OutputStep.qml
        qml/TestStep.qml
        qml/MonitorPanel.qml
        qml/OverlayPanel.qml
        qml/SettingsPanel.qml
        qml/ScoreboardPanel.qml
        qml/TickerPanel.qml
        qml/WeatherPanel.qml
        qml/SubtitlePanel.qml
        qml/StatsPanel.qml
        qml/TeleprompterPanel.qml
        qml/MacroPanel.qml
        qml/ThemeManager.qml
        qml/SocialChatPanel.qml
)

target_link_libraries(prestige_control PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Network
    prestige_shared
    prestige_broadcast_sdk
)

if(ZMQ_FOUND)
    target_link_libraries(prestige_control PRIVATE PkgConfig::ZMQ)
    target_compile_definitions(prestige_control PRIVATE PRESTIGE_HAVE_ZMQ)
endif()
