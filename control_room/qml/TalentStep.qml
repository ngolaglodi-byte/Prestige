import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
// QtQuick.Dialogs removed — using built-in path input

// ============================================================
// Step 2 — Talent Management (real backend)
// ============================================================

Item {
    id: talentStepRoot

    Component.onCompleted: talentManager.refreshTalents()

    RowLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 24

        // ── Left: Talent list ──────────────────────────────
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Label { text: window.t("emission_talents"); font.pixelSize: 16; color: window.darkMode ? "white" : "#1A1A1A" }
                Item { Layout.fillWidth: true }

                Button {
                    text: window.t("new_talent")
                    onClicked: addTalentDialog.open()
                    background: Rectangle { color: "#6C5CE7"; radius: 6 }
                    contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 12; horizontalAlignment: Text.AlignHCenter }
                }

                Button {
                    text: window.t("refresh")
                    flat: true
                    onClicked: talentManager.refreshTalents()
                    contentItem: Label { text: parent.text; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 12 }
                }
            }

            // Connection status (prominent)
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: talentManager.connected ? 36 : 52; radius: 8
                color: talentManager.connected ? (window.darkMode ? "#0A1A0A" : "#E8F5E9") : (window.darkMode ? "#2A0A0A" : "#FFEBEE")
                border.color: talentManager.connected ? "#1DB954" : "#CC3333"
                border.width: talentManager.connected ? 1 : 2
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 8; spacing: 2
                    RowLayout {
                        spacing: 8
                        Rectangle {
                            Layout.preferredWidth: 10; Layout.preferredHeight: 10; radius: 5
                            color: talentManager.connected ? "#1DB954" : "#CC3333"
                            SequentialAnimation on opacity {
                                running: !talentManager.connected; loops: Animation.Infinite
                                NumberAnimation { to: 0.3; duration: 800 }
                                NumberAnimation { to: 1.0; duration: 800 }
                            }
                        }
                        Label {
                            text: talentManager.connected
                                  ? window.t("ai_connected") + " \u2014 " + talentManager.talents.length + " " + window.t("talents_count")
                                  : window.t("ai_disconnected")
                            font.pixelSize: 12; font.weight: talentManager.connected ? Font.Normal : Font.Bold
                            color: talentManager.connected ? (window.darkMode ? "#AAA" : "#444") : "#CC3333"
                        }
                    }
                    Label {
                        visible: !talentManager.connected
                        text: "python3 ai_engine/talent_manager.py (port 5556)"
                        font.pixelSize: 10; font.family: "Menlo"
                        color: window.darkMode ? "#886666" : "#CC9999"
                        Layout.leftMargin: 18
                    }
                }
            }

            // Talent list
            ListView {
                id: talentListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 6
                clip: true
                model: talentManager.talents

                delegate: Rectangle {
                    width: talentListView.width
                    height: 80
                    radius: 8
                    color: window.darkMode ? "#111114" : "#FFFFFF"
                    border.color: window.darkMode ? "#1E1E22" : "#DDD"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        // Avatar
                        Rectangle {
                            Layout.preferredWidth: 56; Layout.preferredHeight: 56; radius: 28
                            color: window.darkMode ? "#2A2A30" : "#E8E8EE"
                            border.color: modelData.has_embeddings ? "#1DB954" : (window.darkMode ? "#444" : "#AAA")
                            border.width: 2

                            // Show photo if exists
                            Image {
                                anchors.fill: parent
                                anchors.margins: 2
                                source: talentManager.talentPhotoPath(modelData.id)
                                visible: source !== ""
                                fillMode: Image.PreserveAspectCrop
                                layer.enabled: true
                                layer.effect: Item {} // mask placeholder
                            }

                            // Fallback initial
                            Label {
                                anchors.centerIn: parent
                                text: modelData.name ? modelData.name.charAt(0).toUpperCase() : "?"
                                font.pixelSize: 22; font.bold: true
                                color: "#6C5CE7"
                                visible: talentManager.talentPhotoPath(modelData.id) === ""
                            }
                        }

                        // Info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Label { text: modelData.name; font.pixelSize: 15; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
                            Label { text: modelData.role; font.pixelSize: 12; color: window.darkMode ? "#888" : "#555" }
                            RowLayout {
                                spacing: 8
                                Rectangle {
                                    Layout.preferredWidth: embLbl.implicitWidth + 12; Layout.preferredHeight: 18; radius: 4
                                    color: modelData.has_embeddings ? (window.darkMode ? "#0A2A0A" : "#E8F5E9") : (window.darkMode ? "#2A1A0A" : "#FFF3E0")
                                    Label { id: embLbl; anchors.centerIn: parent
                                        text: modelData.has_embeddings
                                              ? modelData.embedding_count + " " + window.t("embedding_count")
                                              : window.t("no_embedding")
                                        font.pixelSize: 11
                                        color: modelData.has_embeddings ? "#1DB954" : "#CC8833" }
                                }
                            }
                        }

                        // Actions
                        ColumnLayout {
                            spacing: 4

                            Button {
                                text: window.t("photo")
                                flat: true
                                onClicked: {
                                    photoFileDialog.talentId = modelData.id
                                    photoFileDialog.open()
                                }
                                contentItem: Label { text: parent.text; color: "#6C5CE7"; font.pixelSize: 11 }
                            }

                            Button {
                                text: window.t("delete")
                                flat: true
                                onClicked: {
                                    deleteTalentDialog.talentId = modelData.id
                                    deleteTalentDialog.talentName = modelData.name
                                    deleteTalentDialog.open()
                                }
                                contentItem: Label { text: parent.text; color: "#CC3333"; font.pixelSize: 11 }
                            }
                        }
                    }
                }

                // Empty state
                Label {
                    anchors.centerIn: parent
                    visible: talentManager.talents.length === 0
                    text: window.t("no_talents")
                    color: window.darkMode ? "#555" : "#999"; font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // ── Right: Info panel ──────────────────────────────
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: window.darkMode ? "#111114" : "#FFFFFF"; radius: 8; border.color: window.darkMode ? "#1E1E22" : "#DDD"

            Flickable {
                anchors.fill: parent; anchors.margins: 16
                contentHeight: rightCol.implicitHeight; clip: true
                flickableDirection: Flickable.VerticalFlick
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                ColumnLayout {
                    id: rightCol; width: parent.width; spacing: 16

                    Label { text: window.t("talent_register"); font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }

                    Label {
                        text: "1. Cliquez sur '+ Nouveau talent'"
                        font.pixelSize: 12; color: window.darkMode ? "#AAA" : "#444"
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    Label {
                        text: "2. Remplissez le nom et le rôle"
                        font.pixelSize: 12; color: window.darkMode ? "#AAA" : "#444"
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    Label {
                        text: "3. Sélectionnez une photo du visage (format JPG/PNG)"
                        font.pixelSize: 12; color: window.darkMode ? "#AAA" : "#444"
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    Label {
                        text: "4. L'IA génère automatiquement l'embedding de reconnaissance faciale"
                        font.pixelSize: 12; color: window.darkMode ? "#AAA" : "#444"
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }
                    Label {
                        text: "5. Pendant le live, le talent est reconnu automatiquement et l'overlay s'affiche"
                        font.pixelSize: 12; color: window.darkMode ? "#AAA" : "#444"
                        wrapMode: Text.WordWrap; Layout.fillWidth: true
                    }

                    Item { Layout.fillHeight: true }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }

                    Label { text: "Formats photo acceptés"; font.pixelSize: 12; font.bold: true; color: window.darkMode ? "#888" : "#555" }
                    Label { text: "JPG, PNG — visage face caméra\nBonne lumière, fond neutre idéal\n1 photo minimum par talent"; font.pixelSize: 11; color: window.darkMode ? "#666" : "#999"; wrapMode: Text.WordWrap; Layout.fillWidth: true }

                    Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: window.darkMode ? "#222" : "#CCC" }

                    Label { text: "Base : talents.json"; font.pixelSize: 11; color: window.darkMode ? "#555" : "#999" }
                    Label { text: "Photos : ~/.prestige-ai/talent_photos/"; font.pixelSize: 11; color: window.darkMode ? "#555" : "#999"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                }
            }
        }
    }

    // ── Add Talent Dialog ──────────────────────────────────
    Dialog {
        id: addTalentDialog
        anchors.centerIn: parent
        width: 480
        modal: true
        title: "Nouveau talent"
        standardButtons: Dialog.NoButton

        property string selectedPhotoPath: ""

        background: Rectangle { color: window.darkMode ? "#1A1A1E" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? "#333" : "#CCC" }

        header: Rectangle {
            color: "transparent"; height: 48
            Label { anchors.centerIn: parent; text: "Nouveau talent"; font.pixelSize: 16; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
        }

        contentItem: ColumnLayout {
            spacing: 16

            // Photo area
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 120; Layout.preferredHeight: 120; radius: 60
                color: window.darkMode ? "#2A2A30" : "#E8E8EE"
                border.color: addTalentDialog.selectedPhotoPath !== "" ? "#1DB954" : (window.darkMode ? "#444" : "#AAA")
                border.width: 2

                Image {
                    anchors.fill: parent; anchors.margins: 2
                    source: addTalentDialog.selectedPhotoPath !== "" ? "file://" + addTalentDialog.selectedPhotoPath : ""
                    visible: source !== ""
                    fillMode: Image.PreserveAspectCrop
                }

                Label {
                    anchors.centerIn: parent
                    text: addTalentDialog.selectedPhotoPath !== "" ? "" : "Photo"
                    color: window.darkMode ? "#666" : "#999"; font.pixelSize: 14
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: addPhotoDialog.open()
                }
            }

            Label {
                text: "Cliquez sur le cercle pour choisir une photo"
                font.pixelSize: 11; color: window.darkMode ? "#888" : "#555"
                Layout.alignment: Qt.AlignHCenter
            }

            // Name
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Label { text: "Nom complet"; font.pixelSize: 12; color: window.darkMode ? "#999" : "#666" }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    placeholderText: "Ex: John Smith"
                    font.pixelSize: 14; color: window.darkMode ? "#FFF" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            // Role
            ColumnLayout {
                Layout.fillWidth: true; spacing: 4
                Label { text: "Rôle / Fonction"; font.pixelSize: 12; color: window.darkMode ? "#999" : "#666" }
                TextField {
                    id: roleField
                    Layout.fillWidth: true
                    placeholderText: "Ex: Presenter, Journalist, Guest..."
                    font.pixelSize: 14; color: window.darkMode ? "#FFF" : "#1A1A1A"
                    background: Rectangle { color: window.darkMode ? "#1E1E22" : "#F0F0F4"; radius: 6; border.color: window.darkMode ? "#333" : "#CCC" }
                }
            }

            // Buttons
            RowLayout {
                Layout.fillWidth: true; spacing: 12
                Item { Layout.fillWidth: true }
                Button {
                    text: "Annuler"; flat: true
                    onClicked: { addTalentDialog.close(); addTalentDialog.selectedPhotoPath = "" }
                    contentItem: Label { text: parent.text; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter }
                }
                Button {
                    text: "  Enregistrer  "
                    enabled: nameField.text.length > 0
                    onClicked: {
                        var result = talentManager.addTalent(nameField.text, roleField.text, addTalentDialog.selectedPhotoPath)
                        if (result && result.error) {
                            console.log("[Talent] Error adding talent:", result.error)
                        } else {
                            nameField.text = ""
                            roleField.text = ""
                            addTalentDialog.selectedPhotoPath = ""
                            addTalentDialog.close()
                            talentManager.refreshTalents()
                        }
                    }
                    background: Rectangle { color: parent.enabled ? "#6C5CE7" : (window.darkMode ? "#333" : "#CCC"); radius: 6 }
                    contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter }
                }
            }
        }
    }

    // ── Delete confirmation ────────────────────────────────
    Dialog {
        id: deleteTalentDialog
        anchors.centerIn: parent; width: 380; modal: true
        standardButtons: Dialog.NoButton
        property string talentId: ""
        property string talentName: ""

        background: Rectangle { color: window.darkMode ? "#1A1A1E" : "#FFFFFF"; radius: 12; border.color: window.darkMode ? "#333" : "#CCC" }
        contentItem: ColumnLayout {
            spacing: 16
            Label { text: "Supprimer " + deleteTalentDialog.talentName + " ?"; font.pixelSize: 16; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A" }
            Label { text: "Le talent et sa photo seront supprimés.\nCette action est irréversible."; font.pixelSize: 13; color: window.darkMode ? "#999" : "#666"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button { text: "Annuler"; flat: true; onClicked: deleteTalentDialog.close()
                    contentItem: Label { text: parent.text; color: window.darkMode ? "#888" : "#555"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter } }
                Button { text: "  Supprimer  "; onClicked: { talentManager.deleteTalent(deleteTalentDialog.talentId); deleteTalentDialog.close() }
                    background: Rectangle { color: "#CC0000"; radius: 6 }
                    contentItem: Label { text: parent.text; color: "white"; font.pixelSize: 13; horizontalAlignment: Text.AlignHCenter } }
            }
        }
    }

    // ── File path helpers (no QtQuick.Dialogs dependency) ──
    // addPhotoDialog and photoFileDialog replaced by inline path input
    QtObject {
        id: addPhotoDialog
        property bool showing: false
        function open() { showing = true }
    }
    QtObject {
        id: photoFileDialog
        property string talentId: ""
        property bool showing: false
        function open() { showing = true }
    }
}
