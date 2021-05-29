/*
    SPDX-FileCopyrightText: 2021 Jiashu Yu <yujiashu@jingos.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "passwdtype.h"
#include <KConfigGroup>
#include <KSharedConfig>

PassWDType::PassWDType(QObject *parent) : QObject(parent)
{

}
QString PassWDType::readPassWdTyp(){
    auto kdeglobals = KSharedConfig::openConfig(QString(QStringLiteral("kdeglobals")));
    KConfigGroup cfg(kdeglobals, QString(QStringLiteral("LockScreen")));

    return cfg.readEntry(QString(QStringLiteral("passwordType")), QString());
}


