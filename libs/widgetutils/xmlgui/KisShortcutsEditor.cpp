/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "KisShortcutsEditor.h"
#include "KisShortcutsEditor_p.h"
#include "config-xmlgui.h"

// The following is needed for KisShortcutsEditorPrivate and QTreeWidgetHack
// #include "KisShortcutsDialog_p.h"

#include <QAction>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QTextTableFormat>
#include <QPrinter>
#include <QDebug>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include "kactioncollection.h"
#include "kactioncategory.h"
#include <ktreewidgetsearchline.h>

//---------------------------------------------------------------------
// KisShortcutsEditor
//---------------------------------------------------------------------

Q_DECLARE_METATYPE(KisShortcutsEditorItem *)


KisShortcutsEditor::KisShortcutsEditor(KActionCollection *collection, QWidget *parent, ActionTypes actionType,
                                   LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KisShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
    addCollection(collection);
}

KisShortcutsEditor::KisShortcutsEditor(QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KisShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
}

KisShortcutsEditor::~KisShortcutsEditor()
{
    delete d;
}

bool KisShortcutsEditor::isModified() const
{
    // Iterate over all items
    QTreeWidgetItemIterator it(d->ui.list, QTreeWidgetItemIterator::NoChildren);

    for (; (*it); ++it) {
        KisShortcutsEditorItem *item = dynamic_cast<KisShortcutsEditorItem *>(*it);
        if (item && item->isModified()) {
            return true;
        }
    }
    return false;
}

void KisShortcutsEditor::clearCollections()
{
    d->delegate->contractAll();
    d->ui.list->clear();
    d->actionCollections.clear();
    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KisShortcutsEditor::addCollection(KActionCollection *collection, const QString &title)
{
    // KXmlGui add action collections unconditionally. If some plugin doesn't
    // provide actions we don't want to create empty subgroups.
    if (collection->isEmpty()) {
        return;
    }

    // Pause updating.
    setUpdatesEnabled(false);


    /**
     * Forward this actioncollection to the delegate which will do conflict checking.
     * TODO this seems to _replace_ any existing collections in the delegate. So, is that cool?
     */
    d->actionCollections.append(collection);
    d->delegate->setCheckActionCollections(d->actionCollections);


    // Determine how we should label this collection in the widget.
    QString collectionTitle;
    if (!title.isEmpty()) {
      collectionTitle = title;
    } else {
      // Use the programName (Translated).
      collectionTitle = collection->componentDisplayName();
    }

    // Create the collection root node.
    QTreeWidgetItem *hierarchy[3];
    hierarchy[KisShortcutsEditorPrivate::Root] = d->ui.list->invisibleRootItem();
    hierarchy[KisShortcutsEditorPrivate::Program] =
      d->findOrMakeItem(hierarchy[KisShortcutsEditorPrivate::Root], collectionTitle);
    hierarchy[KisShortcutsEditorPrivate::Action] = NULL;

    // Remember which actions we have seen. We will be adding categorized
    // actions first, so this will help us keep track of which actions haven't
    // been categorized yet, so we can add them as uncategorized at the end.
    QSet<QAction *> actionsSeen;

    // Add a subtree for each category? Perhaps easier to think that this
    // doesn't exist. Basically you add KActionCategory as a QObject child of
    // KActionCollection, and then tag objects as belonging to the category.
    QList<KActionCategory *> categories = collection->findChildren<KActionCategory *>();
    foreach (KActionCategory *category, categories) {
        hierarchy[KisShortcutsEditorPrivate::Action] =
          d->findOrMakeItem(hierarchy[KisShortcutsEditorPrivate::Program], category->text());

        // Add every item from each category.
        foreach (QAction *action, category->actions()) {
            actionsSeen.insert(action);
            d->addAction(action, hierarchy, KisShortcutsEditorPrivate::Action);
        }
    }

    // Finally, tack on any uncategorized actions.
    foreach (QAction *action, collection->actions()) {
        if (!actionsSeen.contains(action)) {
            d->addAction(action, hierarchy, KisShortcutsEditorPrivate::Program);
        }
    }

    // sort the list
    d->ui.list->sortItems(Name, Qt::AscendingOrder);

    // Now turn on updating again.
    setUpdatesEnabled(true);

    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KisShortcutsEditor::clearConfiguration()
{
    d->clearConfiguration();
}

void KisShortcutsEditor::importConfiguration(KConfigBase *config)
{
    d->importConfiguration(config);
}

void KisShortcutsEditor::exportConfiguration(KConfigBase *config) const
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    if (d->actionTypes) {
        QString groupName(QStringLiteral("Shortcuts"));
        KConfigGroup group(config, groupName);
        foreach (KActionCollection *collection, d->actionCollections) {
            collection->writeSettings(&group, true);
        }
    }
}

void KisShortcutsEditor::writeConfiguration(KConfigGroup *config) const
{
    foreach (KActionCollection *collection, d->actionCollections) {
        collection->writeSettings(config);
    }
}

//slot
void KisShortcutsEditor::resizeColumns()
{
    for (int i = 0; i < d->ui.list->columnCount(); i++) {
        d->ui.list->resizeColumnToContents(i);
    }
}




void KisShortcutsEditor::commit()
{
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KisShortcutsEditorItem *item = dynamic_cast<KisShortcutsEditorItem *>(*it)) {
            item->commit();
        }
    }
}

void KisShortcutsEditor::save()
{
    writeConfiguration();
    // we have to call commit. If we wouldn't do that the changes would be
    // undone on deletion! That would lead to weird problems. Changes to
    // Global Shortcuts would vanish completely. Changes to local shortcuts
    // would vanish for this session.
    commit();
}

//From 2007-ish:
// There was once a crash where these items were deleted too early by Qt.
void KisShortcutsEditor::undo()
{
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KisShortcutsEditorItem *item = dynamic_cast<KisShortcutsEditorItem *>(*it)) {
            item->undo();
        }
    }
}

// WTF?
//
// "We ask the user here if there are any conflicts, as opposed to undo(). They
//  don't do the same thing anyway, this just not to confuse any readers. slot"
void KisShortcutsEditor::allDefault()
{
    d->allDefault();
}

void KisShortcutsEditor::printShortcuts() const
{
    d->printShortcuts();
}

KisShortcutsEditor::ActionTypes KisShortcutsEditor::actionTypes() const
{
    return d->actionTypes;
}

void KisShortcutsEditor::setActionTypes(ActionTypes actionTypes)
{
    d->setActionTypes(actionTypes);
}


#include "moc_KisShortcutsEditor.cpp"