import QtQuick 2.0
import QtQuick.Controls 2.1
import com.iskrembilen 1.0

Rectangle {
    id: tournamentView
    property string hoveredName: ""

    color: "black"
    property color foreground: Qt.rgba(0, 1, 0, 1)
    property color lightBackground: Qt.rgba(0, 0.25, 0, 1)

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: tournamentView.hoveredName = ""
    }

    Rectangle {
        id: header
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 5
        }

        color: lightBackground
        height: winnerroundsTitle.height + 20
        border.color: foreground
        border.width: 1

        Text {
            id: winnerroundsTitle
            anchors {
                left: parent.left
                leftMargin: 10
                verticalCenter: parent.verticalCenter
            }
            text: "Winners bracket:"
            font.bold: true
            color: foreground
        }
    }

    Flickable {
        id: winnerFlickable
        anchors {
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.verticalCenter
            margins: 5
        }
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { }


        contentWidth: winnerRoundsRow.width
        contentHeight: winnerRoundsRow.height

        Row {
            id: winnerRoundsRow
            spacing: 30
            Repeater {
                model: TournamentController.winnersRounds
                delegate: roundComponent
            }
        }
    }

    Rectangle {
        id: separator
        anchors {
            top: parent.verticalCenter
            left: parent.left
            right: parent.right
            margins: 5
        }

        color: lightBackground
        height: loserroundsTitle.height + 20
        border.color: foreground
        border.width: 1

        Text {
            id: loserroundsTitle
            anchors {
                left: parent.left
                leftMargin: 10
                verticalCenter: parent.verticalCenter
            }

            text: "Losers bracket:"
            font.bold: true
            color: foreground
        }
    }

    Flickable {
        anchors {
            top: separator.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 5
        }
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: losersRoundsRow.width
        contentHeight: losersRoundsRow.height

        Row {
            id: losersRoundsRow
            spacing: 30
            Repeater {
                model: TournamentController.losersRounds
                delegate: roundComponent
            }
        }
    }

    Component {
        id: roundComponent

        Rectangle {
            width: roundColumn.width
            height: roundColumn.height
            color: "transparent"
            border.width: 2
            border.color: lightBackground
            opacity: matches.length > 0 ? 1 : 0

            Column {
                id: roundColumn

                Rectangle {
                    id: header
                    width: 250
                    height: 30
                    color: lightBackground

                    Text {
                        anchors.centerIn: parent
                        text: name
                        color: foreground
                        font.bold: true
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

        Rectangle {
            width: 250
            height: 100
            color: "black"
            border.width: 1
            border.color: lightBackground

            Text {
                id: matchName
                anchors {
                    left: parent.left
                    leftMargin: 10
                    verticalCenter: parent.verticalCenter
                }

                text: name
                color: "green"
            }

            Column {
                anchors {
                    top: parent.top
                    left: matchName.right
                    margins: 10
                    right: parent.right
                    bottom: parent.bottom
                }
                spacing: 5

                Repeater {
                    model: competitors
                    delegate: competitorComponent

                }
            }
        }
    }

    Component {
        id: competitorComponent
        Rectangle {
            height: parent.height / 2 - parent.spacing / 2
            width: parent.width
            color: {
                if (!isValid) {
                    return "red"
                }

                if (winner) {
                    return lightBackground
                } else {
                    return "black";
                }
            }

            border.width: 2
            border.color: hovered ? foreground : "black"
            property bool hovered: tournamentView.hoveredName === name

            Behavior on border.color { ColorAnimation { duration: 100 } }

            Text {
                id: nameText
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: 10
                    right: scoreRect.left
                    bottom: parent.bottom
                    margins: 5
                }

                verticalAlignment: Text.AlignVCenter
                text: name
                color: foreground
                Behavior on color { ColorAnimation { duration: 100 } }
                font.bold: winner
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
                Behavior on color { ColorAnimation { duration: 100 } }
                color: foreground
                font.bold: winner
            }


            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: tournamentView.hoveredName = name
            }
        }
    }
}
