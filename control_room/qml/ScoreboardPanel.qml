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
        Item{implicitHeight:10}
    }
    }
}
