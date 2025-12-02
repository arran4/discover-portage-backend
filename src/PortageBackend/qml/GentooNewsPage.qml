/*
 * SPDX-FileCopyrightText: 2025 keklick1337 <gentoo@trustcrypt.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.discover.portage 1.0

Kirigami.ScrollablePage {
    id: newsPage
    title: i18n("Gentoo News")
    
    property var newsModel: []
    
    Component.onCompleted: {
        // Load news from backend
        if (typeof PortageNewsManager !== 'undefined') {
            PortageNewsManager.loadNews();
            updateNewsList();
        }
    }
    
    function updateNewsList() {
        if (typeof PortageNewsManager === 'undefined') return;
        
        let items = PortageNewsManager.getNewsItems();
        newsModel = items;
        newsListView.model = items;
    }
    
    actions: [
        Kirigami.Action {
            text: i18n("Mark All as Read")
            icon.name: "mail-mark-read"
            onTriggered: {
                if (typeof PortageNewsManager !== 'undefined') {
                    PortageNewsManager.markAllAsRead();
                    updateNewsList();
                }
            }
        },
        Kirigami.Action {
            text: i18n("Refresh")
            icon.name: "view-refresh"
            onTriggered: {
                if (typeof PortageNewsManager !== 'undefined') {
                    PortageNewsManager.loadNews();
                    updateNewsList();
                }
            }
        }
    ]
    
    ListView {
        id: newsListView
        
        delegate: Kirigami.AbstractCard {
            id: newsCard
            
            property bool isRead: modelData.read || false
            
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        level: 3
                        text: modelData.title || "Untitled News"
                        font.bold: !newsCard.isRead
                    }
                    
                    QQC2.Label {
                        text: Qt.formatDateTime(modelData.posted, "yyyy-MM-dd")
                        opacity: 0.7
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                    
                    Kirigami.Icon {
                        source: newsCard.isRead ? "mail-read" : "mail-unread"
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }
                }
                
                QQC2.Label {
                    Layout.fillWidth: true
                    text: i18n("Author: %1", modelData.author || "Unknown")
                    opacity: 0.7
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                }
                
                Kirigami.Separator {
                    Layout.fillWidth: true
                }
                
                QQC2.ScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(contentTextArea.implicitHeight + 20, 300)
                    
                    QQC2.TextArea {
                        id: contentTextArea
                        text: modelData.content || ""
                        readOnly: true
                        wrapMode: Text.Wrap
                        selectByMouse: true
                        background: null
                    }
                }
                
                RowLayout {
                    Layout.fillWidth: true
                    
                    QQC2.Button {
                        text: newsCard.isRead ? i18n("Mark as Unread") : i18n("Mark as Read")
                        icon.name: newsCard.isRead ? "mail-unread" : "mail-read"
                        onClicked: {
                            if (typeof PortageNewsManager !== 'undefined') {
                                if (newsCard.isRead) {
                                    // TODO: implement mark as unread
                                } else {
                                    PortageNewsManager.markAsRead(modelData.id);
                                    newsCard.isRead = true;
                                    updateNewsList();
                                }
                            }
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                }
            }
        }
        
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: newsListView.count === 0
            text: i18n("No Gentoo news items found")
            explanation: i18n("News items are located in /var/db/repos/gentoo/metadata/news/")
            icon.name: "news-subscribe"
        }
    }
}
