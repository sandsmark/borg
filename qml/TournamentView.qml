import QtQuick 2.0
import QtQuick.Controls 2.1
import com.iskrembilen 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: tournamentView
    property string hoveredName: ""

//    color: "#eceae8"
    color: "black"
    property color foreground: "black"
    property color lightBackground: "lightgray"
    property int fontPixelSize: 30

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: tournamentView.hoveredName = ""
    }

//    FastBlur {
//        id: blur
//        anchors.fill: rootitem
//        source: rootitem
//        radius: 8
//        visible: false
//    }

//    ColorOverlay {
//        anchors.fill: blur
//        color: "#00ffff"
//        source: blur
//    }

    Item {
        id: rootitem
        anchors.fill: parent
        Flickable {
            id: winnerFlickable
            anchors.fill: parent
            anchors.margins: 15
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { }
            ScrollBar.horizontal: ScrollBar { }


            contentWidth: contents.width
            contentHeight: contents.height

            Column {
                id: contents
                spacing: 30
                Row {
                    id: winnerRoundsRow
                    spacing: 30
                    Repeater {
                        model: TournamentController.winnersRounds
                        delegate: roundComponent
                    }
                }
                Row {
                    id: losersRoundsRow
                    spacing: 30
                    Repeater {
                        model: TournamentController.losersRounds
                        delegate: roundComponent
                    }
                }
            }
        }
    }

    Component {
        id: roundComponent

        Rectangle {
            width: roundColumn.width
            height: roundColumn.height + 10
            color: "transparent"
//            border.width: 2
//            border.color: lightBackground
            opacity: matches.length > 0 ? 1 : 0

            BorderImage {
                source: "qrc:/corners.svg"
                anchors.fill: parent
                anchors.margins: -10
                opacity: TournamentController.nextRound === name ? 1 : 0.5
                border { left: 30; top: 30; right: 30; bottom: 30 }
            }

            FastBlur {
                id: blur
                anchors.fill: roundColumn
                source: roundColumn
                radius: 32
                visible: false
            }

            ColorOverlay {
                anchors.fill: blur
                color: "#00ffff"
                source: blur
            }


            Column {
                id: roundColumn
                property string roundName: name
                spacing: 5

                Rectangle {
                    id: roundHeader
                    width: 350
                    height: 35
                    color: "transparent"
                    opacity: TournamentController.nextRound === name ? 1 : 0.5

//                    FastBlur {
//                        id: blur
//                        anchors.fill: roundHeader
//                        source: roundHeader
//                        radius: 16
//                        visible: false
//                    }

//                    ColorOverlay {
//                        anchors.fill: blur
//                        color: "#00ffff"
//                        source: blur
//                    }

                    Text {
                        id: roundName
                        anchors.centerIn: parent
                        text: name
                        color: "white"
//                        font.bold: TournamentController.nextRound === name
                        font.pixelSize: fontPixelSize
                        font.family: "Aldrich"
                    }
                }

                Repeater {
                    id: matchesRepeater
                    model: matches
                    delegate: matchComponent
                }
            }
        }
    }

    Component {
        id: matchComponent

        Item {
            width: 350
            height: 100
            property string roundName: parent.roundName

            opacity: {
                if (TournamentController.nextMatch === name && TournamentController.nextRound === parent.roundName){
                    1
                } else if (isDone) {
                    if (parent.roundName === "Final") {
                        return 1;
                    } else {
                        return 0.5;
                    }
                } else {
                    return 0.75;
                }
            }

            BorderImage {
                source: "qrc:/end.svg"
                anchors.fill: competitorColumn
                anchors.margins: -10
                opacity: matchMouse.containsMouse || TournamentController.nextMatch == name ? 1 : 0.75
                Behavior on opacity { NumberAnimation { duration: 50 } }
                border { left: 30; top: 30; right: 30; bottom: 30 }
            }


            MouseArea {
                id: matchMouse
                anchors.fill: competitorColumn
                hoverEnabled: true
                onPositionChanged: {
                    var child = competitorColumn.childAt(mouse.x, mouse.y)
                    if (child) {
                        tournamentView.hoveredName = child.competitorName
                    }
                }
            }

            Column {
                id: competitorColumn
                property bool isUpcoming: (TournamentController.nextMatch === name)
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: 20
                    right: parent.right
                    rightMargin: 20
                    bottom: parent.bottom
                    margins: 10
                }

                Repeater {
                    model: competitors
                    delegate: competitorComponent

                }
            }
        }
    }

    Component {
        id: competitorComponent
        Item {
            id: competitorRect
            height: parent.height / 2 - parent.spacing / 2
            width: parent.width - 20

            property string competitorName: name
            property bool isUpcoming: parent.isUpcoming

            Text {
                id: nameText
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: 10
                    right: scoreRect.left
                }

                text: name
                font.pixelSize: fontPixelSize
                color: "white"
                Behavior on opacity { NumberAnimation { duration: 50 } }
                opacity: {
                    if (tournamentView.hoveredName === name) {
                        return 1;
                    } else if (competitorRect.isUpcoming && tournamentView.hoveredName === "") {
                        return 1;
                    } else {
                        return 0.5;
                    }
                }

                font.bold: winner
                font.family: "Aldrich"
            }

            Text {
                id: scoreRect
                anchors {
                    top: parent.top
                    right: parent.right
                    bottom: parent.bottom
                }
                width: height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: score
                font.pixelSize: fontPixelSize
                color: "white"
                visible: done
                font.family: "Aldrich"
                opacity: nameText.opacity

            }


        }
    }

//    OldSchoolShader {
//        id: retroShader
//        anchors.fill: parent
//        source: ShaderEffectSource {
//            id: mainSource
//            sourceItem: winnerFlickable
//            width: winnerFlickable.width
//            height: winnerFlickable.height
//            hideSource: true
//        }
//        burninSource: burninTexture
//    }

//    // Need to have these parented outside the main shader above, to avoid crashings
//    ShaderEffectSource {
//        id: burninTexture
//        hideSource: true
//        sourceItem: burninShader
//        recursive: true
//    }

//    ShaderEffect {
//        id: burninShader
//        property variant txt_source: retroShader.source
//        property variant blurredSource: burninTexture
//        property real fps: 60
//        property real blurCoefficient: 1.0 / (fps * 0.65)
//        width: winnerFlickable.width
//        height: winnerFlickable.height
//        blending: false

//        fragmentShader:
//            "#ifdef GL_ES
//                    precision mediump float;
//                #endif\n" +

//        "uniform lowp float qt_Opacity;" +
//        "uniform lowp sampler2D txt_source;" +

//        "varying highp vec2 qt_TexCoord0;

//                 uniform lowp sampler2D blurredSource;
//                 uniform highp float blurCoefficient;" +

//        "float rgb2grey(vec3 v){
//                    return dot(v, vec3(0.21, 0.72, 0.04));
//                }" +

//        "void main() {" +
//        "vec2 coords = qt_TexCoord0;" +
//        "vec3 origColor = texture2D(txt_source, coords).rgb;" +
//        "vec3 blur_color = texture2D(blurredSource, coords).rgb - vec3(blurCoefficient);" +
//        "vec3 color = min(origColor + blur_color, max(origColor, blur_color));" +

//        "gl_FragColor = vec4(color, rgb2grey(color - origColor));" +
//        "}"
//    }
}
