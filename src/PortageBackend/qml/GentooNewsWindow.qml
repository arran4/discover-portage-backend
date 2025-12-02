/*
 * SPDX-FileCopyrightText: 2025 keklick1337 <gentoo@trustcrypt.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Window
import org.kde.discover.portage 1.0

Window {
    id: newsWindow
    title: i18n("Gentoo News")
    width: 800
    height: 600
    visible: true
    
    GentooNewsPage {
        anchors.fill: parent
    }
}
