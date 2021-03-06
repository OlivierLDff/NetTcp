/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

// Qt
import QtQuick 2.12

// Backend
import Qaterial 1.0 as Qaterial
import Stringify.Validator 1.0 as StringifyValidator

// Debug
import NetTcp 1.0 as NetTcp
import NetTcp.Debug 1.0 as Debug

Column
{
    id: root
    property NetTcp.Server object: null
    Qaterial.LabelCaption
    {
        text: "isRunning : " + (root.object ? root.object.isRunning : "")
        width: parent.width
        elide: Text.ElideRight
        color: root.object && root.object.isRunning ? Qaterial.Style.green : Qaterial.Style.red
    }

    Qaterial.LabelCaption
    {
        text: "isListening : " + (root.object ? root.object.isListening : "")
        width: parent.width
        elide: Text.ElideRight
        color: root.object && root.object.isListening ? Qaterial.Style.green : Qaterial.Style.red
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
        text: "address : " + (root.object ? root.object.address : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                if(acceptableInput)
                    root.object.address = result
                else
                    Qaterial.SnackbarManager.show({text : result + " isn't an ipv4 address"})
            },
            text: root.object.address,
            title: qsTr("Enter Bind Ip Address"),
            textTitle: qsTr("Ip"),
            helperText: "Should be 0.0.0.0 for any iface",
            validator: StringifyValidator.Ipv4,
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
        text: "port : " + (root.object ? root.object.port : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.port = result
            },
            text: root.object.port,
            title: qsTr("Enter Bind Listening port"),
            textTitle: qsTr("Listening port"),
            helperText: "Between 0 and 65535",
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            validator: StringifyValidator.SocketPort,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

    Qaterial.LabelCaption
    {
        text: "clients : " + (root.object ? root.object.count : "")
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
        text: "maxClientCount : " + (root.object ? root.object.maxClientCount : "")
        onClicked: if(root.object) Qaterial.DialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                root.object.maxClientCount = result
            },
            text: root.object.maxClientCount,
            title: qsTr("Enter Max Client Count"),
            textTitle: qsTr("maxClientCount"),
            inputMethodHints: Qt.ImhFormattedNumbersOnly,
            selectAllText: true,
            standardButtons: Qaterial.Dialog.Cancel | Qaterial.Dialog.Yes
        })
    }

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
                        Qaterial.SnackbarManager.show({text : "Fail to stop server"})
                }
                else if(root.object)
                {
                    if(!root.object.start())
                        Qaterial.SnackbarManager.show({text : "Fail to start server"})
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
    }

    Repeater
    {
        width: parent.width
        model: root.object
        delegate: Debug.Socket
        {
            width: parent.width
            object: model.qtObject
        }
    } // Repeater
}
