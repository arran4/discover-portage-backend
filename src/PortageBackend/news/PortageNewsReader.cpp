/*
 * SPDX-FileCopyrightText: 2025 keklick1337 <gentoo@trustcrypt.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "PortageNewsReader.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QProcess>

PortageNewsReader::PortageNewsReader(QObject *parent)
    : QObject(parent)
{
    loadReadStatus();
}

void PortageNewsReader::loadNews()
{
    m_newsItems.clear();
    
    QDir newsDir(m_newsPath);
    if (!newsDir.exists()) {
        qWarning() << "Portage: News directory does not exist:" << m_newsPath;
        Q_EMIT newsLoaded(0);
        return;
    }
    
    // Get all subdirectories (year folders like 2024-01-15-some-news)
    QStringList yearDirs = newsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
    
    for (const QString &yearDir : yearDirs) {
        QString yearPath = newsDir.absoluteFilePath(yearDir);
        QDir year(yearPath);
        
        // Get all .txt files in year directory
        QStringList newsFiles = year.entryList(QStringList() << QStringLiteral("*.txt"), QDir::Files, QDir::Name | QDir::Reversed);
        
        for (const QString &newsFile : newsFiles) {
            QString filePath = year.absoluteFilePath(newsFile);
            parseNewsFile(filePath);
        }
    }
    
    // Also check root news directory for non-dated news
    QStringList rootNews = newsDir.entryList(QStringList() << QStringLiteral("*.txt"), QDir::Files, QDir::Name | QDir::Reversed);
    for (const QString &newsFile : rootNews) {
        QString filePath = newsDir.absoluteFilePath(newsFile);
        parseNewsFile(filePath);
    }
    
    // Sort by date (newest first)
    std::sort(m_newsItems.begin(), m_newsItems.end(), [](const GentooNewsItem &a, const GentooNewsItem &b) {
        return a.posted > b.posted;
    });

    qDebug() << "Portage: Loaded" << m_newsItems.size() << "news items";

    // Try to get unread list from system 'eselect news' if available.
    // This lets us show the same unread/read state as the system
    // (eselect stores read state centrally).
    QProcess p;
    p.start(QStringLiteral("eselect"), QStringList() << QStringLiteral("news") << QStringLiteral("list") << QStringLiteral("new"));
    bool haveEselect = p.waitForFinished(2000);
    if (haveEselect && p.exitCode() == 0) {
        const QByteArray out = p.readAllStandardOutput();
        const QStringList lines = QString::fromUtf8(out).split(QLatin1Char('\n'), Qt::SkipEmptyParts);

        QSet<QString> unreadTitles;
        QRegularExpression re(QStringLiteral(R"(^\s*\[\d+\]\s+\S+\s+\[.*?\]\s+(.*)$)"));
        for (const QString &ln : lines) {
            QRegularExpressionMatch m = re.match(ln);
            if (m.hasMatch()) {
                QString title = m.captured(1).trimmed();
                if (!title.isEmpty())
                    unreadTitles.insert(title);
            }
        }

        // Update read flags according to eselect output; fallback to local read file only if eselect didn't find anything
        for (GentooNewsItem &item : m_newsItems) {
            if (!unreadTitles.isEmpty()) {
                item.read = !unreadTitles.contains(item.title);
            } else {
                // No unread info; fallback to local read status
                item.read = m_readNewsIds.contains(item.id);
            }
        }
    } else {
        // No eselect available or it failed; fallback to local read status file
        for (GentooNewsItem &item : m_newsItems) {
            item.read = m_readNewsIds.contains(item.id);
        }
    }

    Q_EMIT newsLoaded(m_newsItems.size());
    Q_EMIT unreadCountChanged(unreadCount());
}

void PortageNewsReader::parseNewsFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Portage: Cannot open news file:" << filePath;
        return;
    }
    
    GentooNewsItem item;
    item.id = QFileInfo(filePath).baseName(); // filename without .txt
    
    QTextStream in(&file);
    QString content;
    QString currentHeader;
    bool inContent = false;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        
        // Parse headers
        if (line.startsWith(QLatin1String("Title:"))) {
            item.title = line.mid(6).trimmed();
        } else if (line.startsWith(QLatin1String("Author:"))) {
            item.author = line.mid(7).trimmed();
        } else if (line.startsWith(QLatin1String("Posted:"))) {
            QString dateStr = line.mid(7).trimmed();
            item.posted = QDateTime::fromString(dateStr, Qt::ISODate);
        } else if (line.startsWith(QLatin1String("Display-If-Installed:"))) {
            item.displayIfInstalled = line.mid(21).trimmed();
        } else if (line.startsWith(QLatin1String("Display-If-Keyword:"))) {
            item.displayIfKeyword = line.mid(19).trimmed();
        } else if (line.startsWith(QLatin1String("Display-If-Profile:"))) {
            item.displayIfProfile = line.mid(19).trimmed();
        } else if (line.isEmpty() && !item.title.isEmpty()) {
            // Empty line after headers = start of content
            inContent = true;
        } else if (inContent) {
            content += line + QLatin1Char('\n');
        }
    }
    
    item.content = content.trimmed();
    item.read = m_readNewsIds.contains(item.id);
    
    // Only add if we have at least title
    if (!item.title.isEmpty()) {
        m_newsItems.append(item);
    }
}

void PortageNewsReader::loadReadStatus()
{
    m_readNewsIds.clear();
    
    QFile file(m_readStatusPath);
    if (!file.exists()) {
        return; // No read status yet
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Portage: Cannot open read status file:" << m_readStatusPath;
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            m_readNewsIds.append(line);
        }
    }
    
    qDebug() << "Portage: Loaded" << m_readNewsIds.size() << "read news IDs";
}

void PortageNewsReader::saveReadStatus()
{
    QFileInfo fileInfo(m_readStatusPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(QLatin1String("."));
    }
    
    QFile file(m_readStatusPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Portage: Cannot write read status file:" << m_readStatusPath;
        return;
    }
    
    QTextStream out(&file);
    for (const QString &id : m_readNewsIds) {
        out << id << '\n';
    }
    
    qDebug() << "Portage: Saved" << m_readNewsIds.size() << "read news IDs";
}

int PortageNewsReader::unreadCount() const
{
    int count = 0;
    for (const GentooNewsItem &item : m_newsItems) {
        if (!item.read && shouldDisplay(item)) {
            count++;
        }
    }
    return count;
}

void PortageNewsReader::markAsRead(const QString &newsId)
{
    // Try to tell the system via eselect first so state is shared
    QProcess p;
    p.start(QStringLiteral("eselect"), QStringList() << QStringLiteral("news") << QStringLiteral("read") << newsId);
    bool ok = p.waitForFinished(2000) && p.exitCode() == 0;

    if (!ok) {
        // Fallback to local state file
        if (m_readNewsIds.contains(newsId)) {
            return;
        }
        m_readNewsIds.append(newsId);
        saveReadStatus();
    } else {
        // If eselect succeeded, ensure our local cache also reflects it
        if (!m_readNewsIds.contains(newsId)) {
            m_readNewsIds.append(newsId);
            saveReadStatus();
        }
    }

    // Update item status in memory
    for (GentooNewsItem &item : m_newsItems) {
        if (item.id == newsId) {
            item.read = true;
            break;
        }
    }

    Q_EMIT unreadCountChanged(unreadCount());
}

void PortageNewsReader::markAllAsRead()
{
    for (GentooNewsItem &item : m_newsItems) {
        if (!item.read) {
            item.read = true;
            if (!m_readNewsIds.contains(item.id)) {
                m_readNewsIds.append(item.id);
            }
        }
    }
    
    saveReadStatus();
    Q_EMIT unreadCountChanged(0);
}

bool PortageNewsReader::shouldDisplay(const GentooNewsItem &item) const
{
    Q_UNUSED(item);
    
    // TODO: Implement proper filtering based on:
    // - Display-If-Installed: check if packages are installed
    // - Display-If-Keyword: check current system keywords
    // - Display-If-Profile: check current system profile
    
    // For now, show all news
    return true;
}
