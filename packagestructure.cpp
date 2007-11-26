/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "packagestructure.h"

#include <QMap>

namespace Plasma
{

class ContentStructure
{
    public:
        ContentStructure()
            : directory(false),
              required(false)
        {
        }

        ContentStructure(const ContentStructure& other)
        {
            path = other.path;
            name = other.name;
            mimetypes = other.mimetypes;
            directory = other.directory;
            required = other.required;
        }

        QString path;
        QString name;
        QStringList mimetypes;
        bool directory;
        bool required;
};

class PackageStructure::Private
{
    public:
        QString type;
        QMap<QByteArray, ContentStructure> contents;
        QStringList mimetypes;
};

PackageStructure::PackageStructure(const QString &type)
    : d(new Private)
{
    d->type = type;
}

PackageStructure::PackageStructure(const PackageStructure& rhs)
    : d(new Private)
{
    *d = *rhs.d;
}

PackageStructure::~PackageStructure()
{
    delete d;
}

PackageStructure& PackageStructure::operator=(const PackageStructure& rhs)
{
    if (this == &rhs) {
        return *this;
    }

    *d = *rhs.d;
    return *this;
}

QString PackageStructure::type() const
{
    return d->type;
}

QList<const char*> PackageStructure::directories() const
{
    QList<const char*> dirs;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory) {
            dirs << it.key().constData();
        }
        ++it;
    }
    return dirs;
}

QList<const char*> PackageStructure::requiredDirectories() const
{
    QList<const char*> dirs;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (it.value().directory &&
            it.value().required) {
            dirs << it.key();
        }
        ++it;
    }
    return dirs;
}

QList<const char*> PackageStructure::files() const
{
    QList<const char*> files;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory) {
            files << it.key();
        }
        ++it;
    }
    return files;
}

QList<const char*> PackageStructure::requiredFiles() const
{
    QList<const char*> files;
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.constBegin();
    while (it != d->contents.constEnd()) {
        if (!it.value().directory && it.value().required) {
            files << it.key();
        }
        ++it;
    }
    return files;
}

void PackageStructure::addDirectoryDefinition(const char* key, const QString& path, const QString& name)
{
    ContentStructure s;
    s.name = name;
    s.path = path;
    s.directory = true;

    d->contents[key] = s;
}

void PackageStructure::addFileDefinition(const char* key, const QString& path, const QString& name)
{
    ContentStructure s;
    s.name = name;
    s.path = path;
    s.directory = false;

    d->contents[key] = s;
}

QString PackageStructure::path(const char* key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.find(key);
    if (it == d->contents.constEnd()) {
        return QString();
    }

    return it.value().path;
}

QString PackageStructure::name(const char* key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.find(key);
    if (it == d->contents.constEnd()) {
        return QString();
    }

    return it.value().name;
}

void PackageStructure::setRequired(const char* key, bool required)
{
    QMap<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    it.value().required = required;
}

bool PackageStructure::required(const char* key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.find(key);
    if (it == d->contents.constEnd()) {
        return false;
    }

    return it.value().required;
}

void PackageStructure::setDefaultMimetypes(QStringList mimetypes)
{
    d->mimetypes = mimetypes;
}

void PackageStructure::setMimetypes(const char* key, QStringList mimetypes)
{
    QMap<QByteArray, ContentStructure>::iterator it = d->contents.find(key);
    if (it == d->contents.end()) {
        return;
    }

    it.value().mimetypes = mimetypes;
}

QStringList PackageStructure::mimetypes(const char* key) const
{
    QMap<QByteArray, ContentStructure>::const_iterator it = d->contents.find(key);
    if (it == d->contents.constEnd()) {
        return QStringList();
    }

    if (it.value().mimetypes.isEmpty()) {
        return d->mimetypes;
    }

    return it.value().mimetypes;
}

} // Plasma namespace

