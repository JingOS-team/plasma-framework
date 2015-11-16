/*
    Copyright (C) 2013 Mark Gaiser <markg85@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "daysmodel.h"

#include <QDebug>
#include <QByteArray>
#include <QPluginLoader>
#include <QDir>
#include <QCoreApplication>

DaysModel::DaysModel(QObject *parent) :
    QAbstractListModel(parent),
    m_agendaNeedsUpdate(false)
{
    QString pluginPath;

    const QStringList paths = QCoreApplication::libraryPaths();
    Q_FOREACH (const QString &libraryPath, paths) {
        const QString path(libraryPath + QStringLiteral("/plasmacalendarplugins"));
        QDir dir(path);

        if (!dir.exists()) {
            continue;
        }

        QStringList entryList = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        Q_FOREACH (const QString &fileName, entryList) {
            QPluginLoader loader(dir.absoluteFilePath(fileName));

            if (!loader.load()) {
                qWarning() << "Could not create Plasma Calendar Plugin: " << pluginPath;
                qWarning() << loader.errorString();
                continue;
            }

            QObject *obj = loader.instance();
            if (obj) {
                Plasma::CalendarEventsPlugin *eventsPlugin = qobject_cast<Plasma::CalendarEventsPlugin*>(obj);
                if (eventsPlugin) {
                    qDebug() << "Adding Calendar plugin" << eventsPlugin;
                    connect(eventsPlugin, &Plasma::CalendarEventsPlugin::dataReady,
                            this, &DaysModel::onDataReady);
                    connect(eventsPlugin, &Plasma::CalendarEventsPlugin::eventModified,
                            this, &DaysModel::onEventModified);
                    connect(eventsPlugin, &Plasma::CalendarEventsPlugin::eventRemoved,
                            this, &DaysModel::onEventRemoved);
                    m_eventPlugins << eventsPlugin;
                } else {
                    // not our/valid plugin, so unload it
                    loader.unload();
                }
            } else {
                loader.unload();
            }
        }
    }

    QHash<int, QByteArray> roleNames;

    roleNames.insert(isCurrent,              "isCurrent");
    roleNames.insert(containsEventItems,     "containsEventItems");
    roleNames.insert(dayNumber,              "dayNumber");
    roleNames.insert(monthNumber,            "monthNumber");
    roleNames.insert(yearNumber,             "yearNumber");

    setRoleNames(roleNames);
}

DaysModel::~DaysModel()
{
    qDeleteAll(m_eventPlugins);
}

void DaysModel::setSourceData(QList<DayData> *data)
{
    if (m_data != data) {
        m_data = data;
        reset();
    }
}

int DaysModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_data->size() <= 0) {
        return 0;
    } else {
        return m_data->size();
    }
}

QVariant DaysModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {

        const DayData &currentData = m_data->at(index.row());
        const QDate currentDate(currentData.yearNumber, currentData.monthNumber, currentData.dayNumber);

        switch (role) {
        case isCurrent:
            return currentData.isCurrent;
        case containsEventItems:
            return m_eventsData.contains(currentDate);
        case dayNumber:
            return currentData.dayNumber;
        case monthNumber:
            return currentData.monthNumber;
        case yearNumber:
            return currentData.yearNumber;
        }
    }
    return QVariant();
}

void DaysModel::update()
{
    if (m_data->size() <= 0) {
        return;
    }

    m_eventsData.clear();

    // We always have 42 items (or weeks * num of days in week) so we only have to tell the view that the data changed.
    layoutChanged();

    const QDate modelFirstDay(m_data->at(0).yearNumber, m_data->at(0).monthNumber, m_data->at(0).dayNumber);

    Q_FOREACH (Plasma::CalendarEventsPlugin *eventsPlugin, m_eventPlugins) {
        eventsPlugin->loadEventsForDateRange(modelFirstDay, modelFirstDay.addDays(42));
    }
}

void DaysModel::onDataReady(const QMultiHash<QDate, Plasma::EventData> &data)
{
    m_eventsData.reserve(m_eventsData.size() + data.size());
    m_eventsData += data;
    layoutChanged();
}

void DaysModel::onEventModified(const Plasma::EventData &data)
{
    QList<QDate> updatesList;
    auto i = m_eventsData.begin();
    while (i != m_eventsData.end()) {
        if (i->uid() == data.uid()) {
            *i = data;
            updatesList << i.key();
        }

        ++i;
    }

    if (!updatesList.isEmpty()) {
        m_agendaNeedsUpdate = true;
    }

    Q_FOREACH (const QDate &date, updatesList) {
        const QModelIndex changedIndex = indexForDate(date);
        if (changedIndex.isValid()) {
            Q_EMIT dataChanged(changedIndex, changedIndex);
        }
        Q_EMIT agendaUpdated(date);
    }
}

void DaysModel::onEventRemoved(const QString &uid)
{
    QList<QDate> updatesList;
    auto i = m_eventsData.begin();
    while (i != m_eventsData.end()) {
        if (i->uid() == uid) {
            updatesList << i.key();
            i = m_eventsData.erase(i);
        } else {
            ++i;
        }
    }

    if (!updatesList.isEmpty()) {
        m_agendaNeedsUpdate = true;
    }

    Q_FOREACH (const QDate &date, updatesList) {
        const QModelIndex changedIndex = indexForDate(date);
        if (changedIndex.isValid()) {
            Q_EMIT dataChanged(changedIndex, changedIndex);
        }
        Q_EMIT agendaUpdated(date);
    }
}

QList<QObject*> DaysModel::eventsForDate(const QDate &date)
{
    if (m_lastRequestedAgendaDate == date && !m_agendaNeedsUpdate) {
        return m_qmlData;
    }

    m_lastRequestedAgendaDate = date;
    qDeleteAll(m_qmlData);
    m_qmlData.clear();

    const QList<Plasma::EventData> events = m_eventsData.values(date);
    m_qmlData.reserve(events.size());

    Q_FOREACH (const Plasma::EventData &event, events) {
        m_qmlData << new EventDataDecorator(event, this);
    }

    // sort events by their time
    std::sort(m_qmlData.begin(), m_qmlData.end(), [](QObject *a, QObject *b) {
        return qobject_cast<EventDataDecorator*>(b)->startDateTime() > qobject_cast<EventDataDecorator*>(a)->startDateTime();
    });

    m_agendaNeedsUpdate = false;
    return m_qmlData;
}

QModelIndex DaysModel::indexForDate(const QDate &date)
{
    if (!m_data) {
        return QModelIndex();
    }

    const DayData &firstDay = m_data->at(0);
    const QDate firstDate(firstDay.yearNumber, firstDay.monthNumber, firstDay.dayNumber);

    qint64 daysTo = firstDate.daysTo(date);

    return createIndex(daysTo, 0);
}
