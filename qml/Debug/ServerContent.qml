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
    property NetTcp.AbstractServer object: null
    Qaterial.Label
    {
        text: "isRunning : " + (root.object ? root.object.isRunning : "")
        width: parent.width
        elide: Text.ElideRight
        textType: Qaterial.Style.TextType.Caption
        color: root.object && root.object.isRunning ? Qaterial.Style.green : Qaterial.Style.red
    }

    Qaterial.Label
    {
        text: "isListening : " + (root.object ? root.object.isListening : "")
        width: parent.width
        elide: Text.ElideRight
        textType: Qaterial.Style.TextType.Caption
        color: root.object && root.object.isListening ? Qaterial.Style.green : Qaterial.Style.red
    }

    Qaterial.FlatButton
    {
        topInset: 0
        bottomInset: 0
        textType: Qaterial.Style.TextType.Caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "watchdogPeriod : " + (root.object ? root.object.watchdogPeriod : "")
        onClicked: if(root.object) dialogManager.openTextField({
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
        textType: Qaterial.Style.TextType.Caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "address : " + (root.object ? root.object.address : "")
        onClicked: if(root.object) dialogManager.openTextField({
            acceptedCallback: function(result, acceptableInput)
            {
                if(acceptableInput)
                    root.object.address = result
                else
                    snackbarManager.show({text : result + " isn't an ipv4 address"})
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
        textType: Qaterial.Style.TextType.Caption
        highlighted: false
        backgroundImplicitHeight: 20
        text: "port : " + (root.object ? root.object.port : "")
        onClicked: if(root.object) dialogManager.openTextField({
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

    Row
    {
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            anchors.verticalCenter: parent.verticalCenter
            textType: Qaterial.Style.TextType.Caption
            backgroundImplicitHeight: 20
            text: root.object && root.object.isRunning ? "Stop" : "Start"
            onClicked:
            {
                if(root.object && root.object.isRunning)
                {
                    if(!root.object.stop())
                        snackbarManager.show({text : "Fail to stop server"})
                }
                else if(root.object)
                {
                    if(!root.object.start())
                        snackbarManager.show({text : "Fail to start server"})
                }
            }
        }
        Qaterial.FlatButton
        {
            topInset: 0
            bottomInset: 0
            anchors.verticalCenter: parent.verticalCenter
            textType: Qaterial.Style.TextType.Caption
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
            elide: Text.ElideRight
            textType: Qaterial.Style.TextType.Caption
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