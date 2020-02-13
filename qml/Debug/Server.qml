/**
 * Copyright (C) Olivier Le Doeuff 2019
 * Contact: olivier.ldff@gmail.com
 */

// Qt
import QtQuick 2.12

// Backend
import Qaterial 1.0 as Qaterial

// Debug
import NetTcp 1.0 as NetTcp
import NetTcp.Debug 1.0 as Debug

Qaterial.DebugObject
{
    id: root
    property NetTcp.AbstractServer object: null
    title: "" + (root.object ? root.object : "Server null")

    content: Debug.ServerContent
    {
        object: root.object
    }
}
