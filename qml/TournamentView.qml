import QtQuick 2.0
import com.iskrembilen 1.0

Rectangle {
    Text {
        id: winnerroundsTitle
        anchors {
            top: parent.top
            left: parent.left
        }
        text: "Winners bracket:"
    }

    Flickable {
        anchors {
            top: winnerroundsTitle.bottom
            left: parent.left
            right: parent.right
            bottom: parent.verticalCenter
        }
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: winnerRoundsRow.width
        contentHeight: winnerRoundsRow.height

        Row {
            id: winnerRoundsRow
            Repeater {
                model: TournamentController.winnersRounds
                delegate: roundComponent
            }
        }
    }

    Text {
        id: loserroundsTitle
        anchors {
            top: parent.verticalCenter
            left: parent.left
        }
        text: "Losers bracket:"
    }
    Flickable {
        anchors {
            top: loserroundsTitle.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: losersRoundsRow.width
        contentHeight: losersRoundsRow.height

        Row {
            id: losersRoundsRow
            Repeater {
                model: TournamentController.losersRounds
                delegate: roundComponent
            }
        }
    }

    Component {
        id: roundComponent

        Column {
            Rectangle {
                id: header
                width: 250
                height: 50
                color: "gray"
                Text {
                    anchors.centerIn: parent
                    text: name
                    color: "white"
                }
            }
            Repeater {
                model: matches
                delegate: matchComponent
            }
        }
    }

    Component {
        id: matchComponent

        Item {
            width: 250
            height: 100
            Rectangle {
                anchors {
                    fill: parent
                    margins: 10
                }
                color: "black"
                border.width: 1
                radius: 5

                Text {
                    id: matchName
                    anchors {
                        left: parent.left
                        leftMargin: 10
                        verticalCenter: parent.verticalCenter
                    }

                    text: name
                    color: "white"

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
    }

    Component {
        id: competitorComponent
        Rectangle {
            height: parent.height / 2 - parent.spacing / 2
            width: parent.width
            radius: 10
            color: isValid ? (winner ? "green" : "gray") : "red"

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
            }
        }
    }
}
