/*
 * SPDX-FileCopyrightText: 2025 keklick1337 <gentoo@trustcrypt.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "PortageNewsManager.h"
#include "PortageNewsReader.h"
#include <QDebug>

PortageNewsManager *PortageNewsManager::s_instance = nullptr;

PortageNewsManager::PortageNewsManager(QObject *parent)
    : QObject(parent)
    , m_reader(new PortageNewsReader(this))
{
    connect(m_reader, &PortageNewsReader::unreadCountChanged,
            this, &PortageNewsManager::unreadCountChanged);
    connect(m_reader, &PortageNewsReader::newsLoaded,
            this, &PortageNewsManager::newsLoaded);
}

PortageNewsManager *PortageNewsManager::instance()
{
    if (!s_instance) {
        s_instance = new PortageNewsManager();
    }
    return s_instance;
}

void PortageNewsManager::loadNews()
{
    qDebug() << "PortageNewsManager: Loading Gentoo news";
    m_reader->loadNews();
}

QVariantList PortageNewsManager::getNewsItems() const
{
    QVariantList result;
    const auto items = m_reader->newsItems();
    
    for (const GentooNewsItem &item : items) {
        QVariantMap map;
        map[QStringLiteral("id")] = item.id;
        map[QStringLiteral("title")] = item.title;
        map[QStringLiteral("content")] = item.content;
        map[QStringLiteral("posted")] = item.posted;
        map[QStringLiteral("author")] = item.author;
        map[QStringLiteral("read")] = item.read;
        result.append(map);
    }
    
    return result;
}

void PortageNewsManager::markAsRead(const QString &newsId)
{
    qDebug() << "PortageNewsManager: Marking news as read:" << newsId;
    m_reader->markAsRead(newsId);
}

void PortageNewsManager::markAllAsRead()
{
    qDebug() << "PortageNewsManager: Marking all news as read";
    m_reader->markAllAsRead();
}

int PortageNewsManager::unreadCount() const
{
    return m_reader->unreadCount();
}
