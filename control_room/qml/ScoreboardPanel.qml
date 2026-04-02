import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: sb
    property string teamA: setupController.scoreboardTeamA
    property string teamB: setupController.scoreboardTeamB
    property int scoreA: setupController.scoreboardScoreA
    property int scoreB: setupController.scoreboardScoreB
    property string colorA: setupController.scoreboardColorA
    property string colorB: setupController.scoreboardColorB
    property bool timerRunning: false
    property int elapsedSec: 0

    Timer { interval: 1000; running: sb.timerRunning; repeat: true; onTriggered: { sb.elapsedSec = sb.elapsedSec + 1; setupController.scoreboardMatchTime = fmt(sb.elapsedSec) } }
    function fmt(s) { var m=Math.floor(s/60); var sec=s%60; return(m<10?"0":"")+m+":"+(sec<10?"0":"")+sec }

    Flickable {
        anchors.fill: parent; contentHeight: col.implicitHeight; clip: true
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
    ColumnLayout {
        id: col; width: parent.width; spacing: 8
        Item { implicitHeight: 4 }
        Label { text: "Score Board"; font.pixelSize: 14; font.bold: true; color: window.darkMode ? "white" : "#1A1A1A"; leftPadding: 8 }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label { text: "Sport:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 11 }
            ComboBox { model: ["Football","Basketball","Tennis","Rugby","Handball"]; Layout.fillWidth: true;
                onActivated: console.log("[Scoreboard] Sport:", currentText)
                background: Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"} }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            ColorPickerButton { Layout.preferredWidth:22;Layout.preferredHeight:22; currentColor: sb.colorA; onColorSelected: function(c) { sb.colorA=c; setupController.scoreboardColorA=c } }
            TextField {
                Layout.fillWidth:true;text:sb.teamA
                onTextChanged: { sb.teamA=text; setupController.scoreboardTeamA=text }
                font.pixelSize:10;color:window.darkMode?"#FFF":"#1A1A1A"
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
            Label{text:"vs";color:window.darkMode?"#555":"#999";font.pixelSize:10}
            TextField {
                Layout.fillWidth:true;text:sb.teamB
                onTextChanged: { sb.teamB=text; setupController.scoreboardTeamB=text }
                font.pixelSize:10;color:window.darkMode?"#FFF":"#1A1A1A"
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
            ColorPickerButton { Layout.preferredWidth:22;Layout.preferredHeight:22; currentColor: sb.colorB; onColorSelected: function(c) { sb.colorB=c; setupController.scoreboardColorB=c } }
        }
        RowLayout { spacing: 6; Layout.alignment: Qt.AlignHCenter
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#CC0000";Label{anchors.centerIn:parent;text:"-";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:if(sb.scoreA>0){sb.scoreA--;setupController.scoreboardScoreA=sb.scoreA}}}
            Label{text:sb.scoreA.toString();font.pixelSize:28;font.bold:true;color:window.darkMode?"white":"#1A1A1A";Layout.preferredWidth:36;horizontalAlignment:Text.AlignHCenter}
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#1DB954";Label{anchors.centerIn:parent;text:"+";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{sb.scoreA++;setupController.scoreboardScoreA=sb.scoreA}}}
            Label{text:"—";color:window.darkMode?"#444":"#AAA";font.pixelSize:18}
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#CC0000";Label{anchors.centerIn:parent;text:"-";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:if(sb.scoreB>0){sb.scoreB--;setupController.scoreboardScoreB=sb.scoreB}}}
            Label{text:sb.scoreB.toString();font.pixelSize:28;font.bold:true;color:window.darkMode?"white":"#1A1A1A";Layout.preferredWidth:36;horizontalAlignment:Text.AlignHCenter}
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#1DB954";Label{anchors.centerIn:parent;text:"+";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{sb.scoreB++;setupController.scoreboardScoreB=sb.scoreB}}}
        }
        RowLayout { spacing: 4; Layout.alignment: Qt.AlignHCenter
            Label{text:setupController.scoreboardMatchTime;font.pixelSize:20;font.family:"Menlo";font.bold:true;color:"#00E5FF"}
            Rectangle{Layout.preferredWidth:50;Layout.preferredHeight:22;radius:4;color:sb.timerRunning?"#CC0000":"#1DB954";Label{anchors.centerIn:parent;text:sb.timerRunning?"STOP":"START";color:"white";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.timerRunning=!sb.timerRunning}}
            Rectangle{Layout.preferredWidth:44;Layout.preferredHeight:22;radius:4;color:window.darkMode?Qt.rgba(1,1,1,0.06):Qt.rgba(0,0,0,0.08);Label{anchors.centerIn:parent;text:"RESET";color:window.darkMode?"#888":"#555";font.pixelSize:8}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{sb.elapsedSec=0;setupController.scoreboardMatchTime="00:00";sb.timerRunning=false}}}
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Période:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            Repeater{model:[1,2,3,4];Rectangle{width:24;height:20;radius:4;color:setupController.scoreboardPeriod===modelData?"#5B4FDB":(window.darkMode?Qt.rgba(1,1,1,0.04):Qt.rgba(0,0,0,0.06));Label{anchors.centerIn:parent;text:modelData.toString();color:setupController.scoreboardPeriod===modelData?"white":(window.darkMode?"#888":"#555");font.pixelSize:9}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:setupController.scoreboardPeriod=modelData}}}
        }
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        Label{text:"Cartons";font.pixelSize:10;color:window.darkMode?"#AAA":"#444";leftPadding:8}
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"J:";color:window.darkMode?"#999":"#666";font.pixelSize:9}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#FFCC00";Label{anchors.centerIn:parent;text:setupController.scoreboardYellowA.toString();color:"#000";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:setupController.scoreboardYellowA=setupController.scoreboardYellowA+1}}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#CC0000";Label{anchors.centerIn:parent;text:setupController.scoreboardRedA.toString();color:"white";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:setupController.scoreboardRedA=setupController.scoreboardRedA+1}}
            Item{Layout.fillWidth:true}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#FFCC00";Label{anchors.centerIn:parent;text:setupController.scoreboardYellowB.toString();color:"#000";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:setupController.scoreboardYellowB=setupController.scoreboardYellowB+1}}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#CC0000";Label{anchors.centerIn:parent;text:setupController.scoreboardRedB.toString();color:"white";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:setupController.scoreboardRedB=setupController.scoreboardRedB+1}}
            Label{text:":R";color:window.darkMode?"#999":"#666";font.pixelSize:9}
        }
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Position:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            ComboBox{id:sbPosCb;model:["Haut gauche","Haut droite","Bas gauche","Bas droite"];currentIndex:0;Layout.fillWidth:true
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
                onCurrentIndexChanged:{var pos=["top_left","top_right","bottom_left","bottom_right"];setupController.scoreboardPosition=pos[currentIndex]}}
        }
        Switch{text:"Afficher";checked:setupController.scoreboardVisible;onToggled:setupController.scoreboardVisible=checked;leftPadding:8}

        // ── Design Template ────────────────────────────
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Design:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            ComboBox{model:["Glass Dark","Solid Dark","Sport Green","FIFA Style","UEFA Modern","Neon Border","White Clean","Red Accent","Blue Accent","Minimal"];
                property var vals:["sb_glass_dark","sb_solid_dark","sb_sport_green","sb_fifa_style","sb_uefa_modern","sb_neon_border","sb_white_clean","sb_red_accent","sb_blue_accent","sb_minimal"]
                currentIndex:Math.max(0,vals.indexOf(setupController.scoreboardDesign));Layout.fillWidth:true
                onActivated:setupController.scoreboardDesign=vals[currentIndex]
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}}
        }

        // ── Team Logos ──────────────────────────────────
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        Label{text:"Logos des equipes";font.pixelSize:10;color:window.darkMode?"#AAA":"#444";leftPadding:8}
        RowLayout { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8
            Label{text:"Equipe A:";color:window.darkMode?"#999":"#666";font.pixelSize:10;Layout.preferredWidth:60}
            TextField {
                Layout.fillWidth:true; text:setupController.teamLogoA; placeholderText:"Chemin logo (PNG)"
                onTextEdited: setupController.teamLogoA = text
                font.pixelSize:10;color:window.darkMode?"#CCC":"#333"
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8
            Label{text:"Equipe B:";color:window.darkMode?"#999":"#666";font.pixelSize:10;Layout.preferredWidth:60}
            TextField {
                Layout.fillWidth:true; text:setupController.teamLogoB; placeholderText:"Chemin logo (PNG)"
                onTextEdited: setupController.teamLogoB = text
                font.pixelSize:10;color:window.darkMode?"#CCC":"#333"
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
        }

        // ── GOAL Animation ─────────────────────────────
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        Label{text:"Animation GOAL";font.pixelSize:10;font.bold:true;color:window.darkMode?"#AAA":"#444";leftPadding:8}

        RowLayout { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8
            Label{text:"Texte:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            TextField {
                id: goalTextField; Layout.fillWidth:true; text:setupController.goalAnimText; placeholderText:"GOAL!"
                onTextEdited: setupController.goalAnimText = text
                font.pixelSize:10;color:window.darkMode?"#CCC":"#333"
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8
            Label{text:"Joueur:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            TextField {
                id: goalPlayerField; Layout.fillWidth:true; placeholderText:"Nom du buteur"
                onTextEdited: setupController.goalAnimPlayer = text
                font.pixelSize:10;color:window.darkMode?"#CCC":"#333"
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Effet:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            ComboBox {
                model:["Kinetic Pop","Bounce In","Wave","Scale Up"]
                property var vals:["kinetic_pop","bounce_in","wave_text","scale_up_letter"]
                currentIndex:Math.max(0,vals.indexOf(setupController.goalAnimEffect))
                onActivated:setupController.goalAnimEffect=vals[currentIndex]
                Layout.fillWidth:true
                background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}
            }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Duree:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            Slider{from:2;to:15;stepSize:1;value:setupController.goalAnimDuration;Layout.fillWidth:true;onMoved:setupController.goalAnimDuration=value}
            Label{text:setupController.goalAnimDuration+"s";color:window.darkMode?"#888":"#555";font.pixelSize:10;Layout.preferredWidth:24}
        }

        // GOAL buttons (1-click: increment score + trigger animation + auto-reset)
        Timer {
            id: goalResetTimer; interval: setupController.goalAnimDuration * 1000; repeat: false
            onTriggered: setupController.goalAnimActive = false
        }
        Timer {
            id: sportEventResetTimer; interval: 4500; repeat: false
            onTriggered: setupController.sportEvent = ""
        }
        RowLayout { spacing: 8; Layout.alignment: Qt.AlignHCenter; Layout.topMargin: 4
            Rectangle {
                Layout.preferredWidth:120;Layout.preferredHeight:40;radius:8;color:sb.colorA
                Label{anchors.centerIn:parent;text:"\u26BD GOAL "+sb.teamA;color:"white";font.pixelSize:11;font.bold:true}
                MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{
                    sb.scoreA++;setupController.scoreboardScoreA=sb.scoreA;
                    setupController.goalAnimTeam="a";setupController.goalAnimActive=true;
                    goalResetTimer.restart()
                }}
            }
            Rectangle {
                Layout.preferredWidth:120;Layout.preferredHeight:40;radius:8;color:sb.colorB
                Label{anchors.centerIn:parent;text:"\u26BD GOAL "+sb.teamB;color:"white";font.pixelSize:11;font.bold:true}
                MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{
                    sb.scoreB++;setupController.scoreboardScoreB=sb.scoreB;
                    setupController.goalAnimTeam="b";setupController.goalAnimActive=true;
                    goalResetTimer.restart()
                }}
            }
        }

        // ── Sport Events ───────────────────────────────
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        Label{text:"Evenements";font.pixelSize:10;color:window.darkMode?"#AAA":"#444";leftPadding:8}
        Flow { spacing: 4; Layout.leftMargin: 8; Layout.rightMargin: 8; Layout.fillWidth: true
            Repeater {
                model: [
                    {label:"\uD83D\uDFE8 Jaune",evt:"yellow_card",c:"#CCAA00"},
                    {label:"\uD83D\uDFE5 Rouge",evt:"red_card",c:"#CC0000"},
                    {label:"\u23F8 Mi-temps",evt:"halftime",c:"#555"},
                    {label:"\u2194 Remplacement",evt:"substitution",c:"#0088CC"},
                    {label:"PEN Penalty",evt:"penalty",c:"#CC0000"},
                    {label:"CK Corner",evt:"corner",c:"#666"},
                    {label:"VAR",evt:"var",c:"#0055AA"},
                    {label:"\u23F9 Fin",evt:"full_time",c:"#888"}
                ]
                Rectangle {
                    width: evtLbl.implicitWidth+14; height:28; radius:6; color:modelData.c
                    Label{id:evtLbl;anchors.centerIn:parent;text:modelData.label;color:"white";font.pixelSize:9;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{setupController.sportEvent=modelData.evt;sportEventResetTimer.restart()}}
                }
            }
        }

        Item{implicitHeight:10}
    }
    }
}
