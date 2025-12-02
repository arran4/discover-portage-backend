/*
 * SPDX-FileCopyrightText: 2025 keklick1337 <gentoo@trustcrypt.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QVariantList>

class PortageNewsReader;

class PortageNewsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged)
    
public:
    static PortageNewsManager* instance();
    
    Q_INVOKABLE void loadNews();
    Q_INVOKABLE QVariantList getNewsItems() const;
    Q_INVOKABLE void markAsRead(const QString &newsId);
    Q_INVOKABLE void markAllAsRead();
    
    int unreadCount() const;

Q_SIGNALS:
    void unreadCountChanged(int count);
    void newsLoaded(int count);

private:
    explicit PortageNewsManager(QObject *parent = nullptr);
    static PortageNewsManager *s_instance;
    
    PortageNewsReader *m_reader;
};
