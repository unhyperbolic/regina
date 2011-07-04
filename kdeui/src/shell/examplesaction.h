
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  KDE User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2009, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

/*! \file examplesaction.h
 *  \brief Provides an action class that offers access to sample data files.
 */

#ifndef __EXAMPLESACTION_H
#define __EXAMPLESACTION_H

#include <kselectaction.h>
#include <qmap.h>

class KUrl;

/**
 * An action offering a selection of sample data files that can be
 * opened.
 *
 * Much of this class is based upon KRecentFilesAction, as taken from
 * KDE 3.2.3.  KRecentFilesAction was written by Michael Koch and is
 * released under the GNU Library General Public License (v2).
 */
class ExamplesAction : public KSelectAction {
    Q_OBJECT

    private:
        /**
         * Sample data files
         */
        QMap<QAction*, KUrl> urls_;

    public:
        /**
         * Constructor and destructor.
         */
        ExamplesAction(QObject* parent);
        virtual ~ExamplesAction();

        /**
         * Add a sample data file to the list of offerings.
         *
         * The filename should be relative to the Regina examples directory.
         */
        void addUrl(const QString& fileName, const QString& description);

    signals:
        /**
         * Emitted when a sample data file is selected for opening.
         */
        void urlSelected(const KUrl& url);

    protected slots:
        /**
         * All activation events lead here.
         */
        void exampleActivated(QAction*);
};

#endif
