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

#ifndef PACKAGESTRUCTURE_H
#define PACKAGESTRUCTURE_H

#include <QtCore/QStringList>

#include <plasma/plasma_export.h>

class KConfigBase;

namespace Plasma
{

/**
 * @brief A description of the expected file structure of a given package type
 *
 * PackageStructure defines what is in a package. This information is used
 * to create packages and provides a way to programatically refer to contents.
 *
 * An example usage of this class might be:
 *
 @code
    PackageStructure structure;

    structure.addDirectoryDefinition("images", "pics/", i18n("Images"));
    QStringList mimetypes;
    mimetypes << "image/svg" << "image/png" << "image/jpeg";
    structure.setMimetypes("images", mimetypes);

    structure.addDirectoryDefinition("scripts", "code/", i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << "text/\*";
    structure.setMimetypes("scripts", mimetypes);

    structure.addFileDefinition("mainscript", "code/main.js", i18n("Main Script File"));
    structure.setRequired("mainscript", true);
 @endcode
 * One may also choose to create a subclass of PackageStructure and include the setup
 * in the constructor.
 *
 * Either way, PackageStructure creates a sort of "contract" between the packager and
 * the application which is also self-documenting.
 **/
class PLASMA_EXPORT PackageStructure
{
public:
    /**
     * Default constructor for a package structure definition
     *
     * @arg type the type of package. This is often application specific.
     **/
    PackageStructure(const QString &type);

    /**
     * Type of package this structure describes
     **/
    QString type() const;

    /**
     * The directories defined for this package
     **/
    QList<const char*> directories() const;

    /**
     * The required directories defined for this package
     **/
    QList<const char*> requiredDirectories() const;

    /**
     * The individual files, by key, that are defined for this package
     **/
    QList<const char*> files() const;

    /**
     * The individual required files, by key, that are defined for this package
     **/
    QList<const char*> requiredFiles() const;

    /**
     * Adds a directory to the structure of the package. It is added as
     * a not-required element with no associated mimetypes.
     *
     * @param key  used as an internal label for this directory
     * @param path the path within the the package for this directory
     * @param name the user visible (translated) name for the directory
     **/
    void addDirectoryDefinition(const char* key, const QString& path, const QString& name);

    /**
     * Adds a file to the structure of the package. It is added as
     * a not-required element with no associated mimetypes.
     *
     * @param key  used as an internal label for this file
     * @param path the path within the the package for this file
     * @param name the user visible (translated) name for the file
     **/
    void addFileDefinition(const char* key, const QString& path, const QString& name);

    /**
     * @return path relative to the package root for the given entry
     **/
    QString path(const char* key) const;

    /**
     * @return user visible name for the given entry
     **/
    QString name(const char* key) const;

    /**
     * Sets whether or not a given part of the structure is required or not.
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param path the path of the entry within the package
     * @param required true if this entry is required, false if not
     */
    void setRequired(const char* key, bool required);

    /**
     * @return true if the item at path exists and is required
     **/
    bool required(const char* key) const;

    /**
     * Defines the default mimetypes for any definitions that do not have
     * associated mimetypes. Handy for packages with only one or predominantly
     * one file type.
     *
     * @param mimetypes a list of mimetypes
     **/
    void setDefaultMimetypes(QStringList mimetypes);

    /**
     * Define mimetypes for a given part of the structure
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     * 
     * @param path the path of the entry within the package
     * @param mimetypes a list of mimetypes
     **/
    void setMimetypes(const char* key, QStringList mimetypes);

    /**
     * @return the mimetypes associated with the path, if any
     **/
    QStringList mimetypes(const char* key) const;

    /**
     * Copy constructor
     **/
    PackageStructure(const PackageStructure& rhs);

    /**
     * Destructor
     **/
    virtual ~PackageStructure();

    /**
     * Assignment operator
     **/
    PackageStructure& operator=(const PackageStructure& rhs);
    
    /**
      * Read a package structure from a config file.
      */
    static PackageStructure read(const KConfigBase *config);
    
    /**
      * Write this package structure to a config file.
      */
    void write(KConfigBase *config) const;

private:
     class Private;
     Private * const d;
};

} // Plasma namespace
#endif
