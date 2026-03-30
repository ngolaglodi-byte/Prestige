import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: sb
    property bool showOnOutput: false
    property string teamA: "ÉQUIPE A"
    property string teamB: "ÉQUIPE B"
    property int scoreA: 0
    property int scoreB: 0
    property string colorA: "#CC0000"
    property string colorB: "#0066CC"
    property int period: 1
    property int yellowA: 0; property int redA: 0
    property int yellowB: 0; property int redB: 0
    property bool timerRunning: false
    property int elapsedSec: 0
    property string matchTime: "00:00"

    Timer { interval: 1000; running: sb.timerRunning; repeat: true; onTriggered: { sb.elapsedSec = sb.elapsedSec + 1; sb.matchTime = fmt(sb.elapsedSec) } }
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
            Label { text: "Sport:"; color: window.darkMode ? "#999" : "#666"; font.pixelSize: 10 }
            ComboBox { model: ["Football","Basketball","Tennis","Rugby","Handball"]; Layout.fillWidth: true; background: Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"} }
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Rectangle { Layout.preferredWidth:18;Layout.preferredHeight:18;radius:4;color:sb.colorA; MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{sb.colorA=sb.colorA==="#CC0000"?"#FF6B00":"#CC0000"}} }
            TextField { Layout.fillWidth:true;text:sb.teamA;onTextChanged:sb.teamA=text;font.pixelSize:10;color:window.darkMode?"#FFF":"#1A1A1A";background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"} }
            Label{text:"vs";color:window.darkMode?"#555":"#999";font.pixelSize:10}
            TextField { Layout.fillWidth:true;text:sb.teamB;onTextChanged:sb.teamB=text;font.pixelSize:10;color:window.darkMode?"#FFF":"#1A1A1A";background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"} }
            Rectangle { Layout.preferredWidth:18;Layout.preferredHeight:18;radius:4;color:sb.colorB; MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{sb.colorB=sb.colorB==="#0066CC"?"#1DB954":"#0066CC"}} }
        }
        RowLayout { spacing: 6; Layout.alignment: Qt.AlignHCenter
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#CC0000";Label{anchors.centerIn:parent;text:"-";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:if(sb.scoreA>0)sb.scoreA--}}
            Label{text:sb.scoreA.toString();font.pixelSize:28;font.bold:true;color:window.darkMode?"white":"#1A1A1A";Layout.preferredWidth:36;horizontalAlignment:Text.AlignHCenter}
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#1DB954";Label{anchors.centerIn:parent;text:"+";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.scoreA++}}
            Label{text:"—";color:window.darkMode?"#444":"#AAA";font.pixelSize:18}
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#CC0000";Label{anchors.centerIn:parent;text:"-";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:if(sb.scoreB>0)sb.scoreB--}}
            Label{text:sb.scoreB.toString();font.pixelSize:28;font.bold:true;color:window.darkMode?"white":"#1A1A1A";Layout.preferredWidth:36;horizontalAlignment:Text.AlignHCenter}
            Rectangle{Layout.preferredWidth:26;Layout.preferredHeight:26;radius:6;color:"#1DB954";Label{anchors.centerIn:parent;text:"+";color:"white";font.pixelSize:14;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.scoreB++}}
        }
        RowLayout { spacing: 4; Layout.alignment: Qt.AlignHCenter
            Label{text:sb.matchTime;font.pixelSize:20;font.family:"Menlo";font.bold:true;color:"#00E5FF"}
            Rectangle{Layout.preferredWidth:50;Layout.preferredHeight:22;radius:4;color:sb.timerRunning?"#CC0000":"#1DB954";Label{anchors.centerIn:parent;text:sb.timerRunning?"STOP":"START";color:"white";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.timerRunning=!sb.timerRunning}}
            Rectangle{Layout.preferredWidth:44;Layout.preferredHeight:22;radius:4;color:window.darkMode?Qt.rgba(1,1,1,0.06):Qt.rgba(0,0,0,0.08);Label{anchors.centerIn:parent;text:"RESET";color:window.darkMode?"#888":"#555";font.pixelSize:8}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:{sb.elapsedSec=0;sb.matchTime="00:00";sb.timerRunning=false}}}
        }
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Période:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            Repeater{model:[1,2,3,4];Rectangle{width:24;height:20;radius:4;color:sb.period===modelData?"#5B4FDB":(window.darkMode?Qt.rgba(1,1,1,0.04):Qt.rgba(0,0,0,0.06));Label{anchors.centerIn:parent;text:modelData.toString();color:sb.period===modelData?"white":(window.darkMode?"#888":"#555");font.pixelSize:9}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.period=modelData}}}
        }
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        Label{text:"Cartons";font.pixelSize:10;color:window.darkMode?"#AAA":"#444";leftPadding:8}
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"J:";color:window.darkMode?"#999":"#666";font.pixelSize:9}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#FFCC00";Label{anchors.centerIn:parent;text:sb.yellowA.toString();color:"#000";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.yellowA++}}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#CC0000";Label{anchors.centerIn:parent;text:sb.redA.toString();color:"white";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.redA++}}
            Item{Layout.fillWidth:true}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#FFCC00";Label{anchors.centerIn:parent;text:sb.yellowB.toString();color:"#000";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.yellowB++}}
            Rectangle{Layout.preferredWidth:16;Layout.preferredHeight:16;radius:2;color:"#CC0000";Label{anchors.centerIn:parent;text:sb.redB.toString();color:"white";font.pixelSize:8;font.bold:true}
                    MouseArea{anchors.fill:parent;cursorShape:Qt.PointingHandCursor;onClicked:sb.redB++}}
            Label{text:":R";color:window.darkMode?"#999":"#666";font.pixelSize:9}
        }
        Rectangle{Layout.fillWidth:true;Layout.preferredHeight:1;color:window.darkMode?"#222":"#CCC";Layout.leftMargin:8;Layout.rightMargin:8}
        RowLayout { spacing: 4; Layout.leftMargin: 8
            Label{text:"Position:";color:window.darkMode?"#999":"#666";font.pixelSize:10}
            ComboBox{model:["Haut gauche","Haut droite","Bas gauche","Bas droite"];currentIndex:1;Layout.fillWidth:true;background:Rectangle{color:window.darkMode?"#1E1E22":"#F0F0F4";radius:4;border.color:window.darkMode?"#333":"#CCC"}}
        }
        Switch{text:"Afficher";checked:sb.showOnOutput;onToggled:sb.showOnOutput=checked;leftPadding:8}
        Item{implicitHeight:10}
    }
    }
}
