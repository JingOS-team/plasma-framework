#ifndef PASSWDTYPE_H
#define PASSWDTYPE_H

#include <QObject>
#include <QObject>

class PassWDType : public QObject
{
    Q_OBJECT
public:
    explicit PassWDType(QObject *parent = nullptr);
    Q_INVOKABLE QString readPassWdTyp();
};


#endif // PASSWDTYPE_H
