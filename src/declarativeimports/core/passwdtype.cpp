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


