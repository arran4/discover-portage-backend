/*
 * SPDX-FileCopyrightText: 2025 keklick1337 <gentoo@trustcrypt.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QDateTime>
#include <QVector>

struct GentooNewsItem {
    QString id;           // e.g., "2024-01-15-some-important-update"
    QString title;        // Parsed from Display-If-Installed
    QString content;      // Full news text
    QDateTime posted;     // Date posted
    QString author;       // Author email
    bool read = false;    // Has user read this?
    
    // Metadata from news file
    QString displayIfInstalled;  // Display-If-Installed condition
    QString displayIfKeyword;    // Display-If-Keyword condition
    QString displayIfProfile;    // Display-If-Profile condition
};

class PortageNewsReader : public QObject
{
    Q_OBJECT
public:
    explicit PortageNewsReader(QObject *parent = nullptr);
    
    // Load all news items from /usr/portage/metadata/news/
    void loadNews();
    
    // Get all news items (sorted by date, newest first)
    QVector<GentooNewsItem> newsItems() const { return m_newsItems; }
    
    // Get unread news count
    int unreadCount() const;
    
    // Mark news item as read
    void markAsRead(const QString &newsId);
    
    // Mark all as read
    void markAllAsRead();
    
    // Check if news item should be displayed based on system config
    bool shouldDisplay(const GentooNewsItem &item) const;

Q_SIGNALS:
    void newsLoaded(int count);
    void unreadCountChanged(int count);

private:
    void parseNewsFile(const QString &filePath);
    void loadReadStatus();
    void saveReadStatus();
    
    QVector<GentooNewsItem> m_newsItems;
    QStringList m_readNewsIds;  // List of read news IDs
    QString m_newsPath = QStringLiteral("/var/db/repos/gentoo/metadata/news");
    QString m_readStatusPath = QStringLiteral("/var/lib/gentoo/news/news-gentoo.read");
};
