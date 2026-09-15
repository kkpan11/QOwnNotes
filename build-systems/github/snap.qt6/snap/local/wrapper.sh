#!/bin/sh

arch="${SNAP_LAUNCHER_ARCH_TRIPLET:-x86_64-linux-gnu}"
qt6_plugin_dir="$SNAP/usr/lib/$arch/qt6/plugins"
qt6_qml_dir="$SNAP/usr/lib/$arch/qt6/qml"

export QT_PLUGIN_PATH="$qt6_plugin_dir${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
export QT_QPA_PLATFORM_PLUGIN_PATH="$qt6_plugin_dir/platforms"
export QML2_IMPORT_PATH="$qt6_qml_dir:$SNAP/lib/$arch${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"
export QML_IMPORT_PATH="$qt6_qml_dir${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"

exec "$SNAP/usr/bin/QOwnNotes" --snap "$@"
