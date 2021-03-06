/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

// Qt
import QtQuick 2.12

// Backend
import Qaterial 1.0 as Qaterial
import Stringify.Validator 1.0 as StringifyValidator
import Stringify.Formatter 1.0 as Formatter

// Debug
import NetTcp 1.0 as NetTcp

Column
{
    id: root
    property NetTcp.Socket object: null
    Qaterial.LabelCaption
    {
        text: "isRunning : " + (root.object ? root.object.isRunning : "")
        width: parent.width
        elide: Text.ElideRight
        color: root.object && root.object.isRunning ? Qaterial.Style.green : Qaterial.Style.red
    }

    Qaterial.LabelCaption
    {
        text: "isConnected : " + (root.object ? root.object.isConnected : "")
        width: parent.width
        elide: Text.ElideRight
        color: root.object && root.object.isConnected ? Qaterial.Style.green : Qaterial.Style.red
    }

    Qaterial.LabelCaption
    {
        id: _errorLabel
        Connections
        {
            target: root.object
            function onSocketError(error, description) { _errorLabel.text = description }
            function onIsConnectedChanged() { if(root.object.isConnected) _errorLabel.text = "" }
        }
        color: "red"
        width: parent.width
        elide: Text.ElideRight
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "watchdogPeriod : " + (root.object ? root.object.watchdogPeriod : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.watchdogPeriod = result
            },
            text: root.object.watchdogPeriod,
            title: qsTr("Enter watchdogPeriod"),
            textTitle: qsTr("watchdogPeriod"),
            helperText: "In Ms",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "peerAddress : " + (root.object ? root.object.peerAddress : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                if(acceptableInput)
                    root.object.peerAddress = result
                else
                    Qaterial.SnackbarManager.show({text : result + " isn't an ipv4 peerAddress"})
            },
            text: root.object.peerAddress,
            title: qsTr("Enter Bind Ip Address"),
            textTitle: qsTr("Ip"),
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        font: Qaterial.Style.textTheme.caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "peerPort : " + (root.object ? root.object.peerPort : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.peerPort = result
            },
            text: root.object.peerPort,
            title: qsTr("Enter Bind Listening peerPort"),
            textTitle: qsTr("Listening peerPort"),
            helperText: "Between 0 and 65535",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            validator: StringifyValidator.SocketPort,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.LabelCaption
    {
        text: "local Address : " + (root.object ? root.object.localAddress : "") + ":" + (root.object ? root.object.localPort : "")
    }

    Row
    {
        spacing: 10
        Qaterial.LabelCaption
        {
            text: "rx/s : " + Formatter.Data.formatBytes(root.object ? root.object.rxBytesPerSeconds : "")
        }

        Qaterial.LabelCaption
        {
            text: "tx/s : " + Formatter.Data.formatBytes(root.object ? root.object.txBytesPerSeconds : "")
        }
    } // Row

    Row
    {
        spacing: 10

        Qaterial.LabelCaption
        {
            text: "rx : " + Formatter.Data.formatBytes(root.object ? root.object.rxBytesTotal : "")
        }

        Qaterial.LabelCaption
        {
            text: "tx : " + Formatter.Data.formatBytes(root.object ? root.object.txBytesTotal : "")
        }
    } // Row

    Row
    {
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            anchors.verticalCenter: parent.verticalCenter
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: root.object && root.object.isRunning ? "Stop" : "Start"
            onClicked:
            {
                if(root.object && root.object.isRunning)
                {
                    if(!root.object.stop())
                        Qaterial.SnackbarManager.show({text : "Fail to stop socket"})
                }
                else if(root.object)
                {
                    if(!root.object.start())
                        Qaterial.SnackbarManager.show({text : "Fail to start socket"})
                }
            }
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            anchors.verticalCenter: parent.verticalCenter
            font: Qaterial.Style.textTheme.caption
            backgroundImplicitHeight: 20
            text: root.object && "Restart"
            enabled: root.object && root.object.isRunning
            onClicked:
            {
                if(root.object)
                {
                    root.object.restart()
                }
            }
        }
        Qaterial.SwitchButton
        {
            text: "worker thread"
            implicitHeight: 32
            anchors.verticalCenter: parent.verticalCenter
            checked: root.object && root.object.useWorkerThread
            font: Qaterial.Style.textTheme.caption
            onClicked: if(root.object) root.object.useWorkerThread = checked
        }
    } // Row
}
