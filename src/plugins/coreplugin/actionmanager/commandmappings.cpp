/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "commandmappings.h"
#include "commandsfile.h"

#include <coreplugin/dialogs/shortcutsettings.h>

#include <utils/hostosinfo.h>
#include <utils/headerviewstretcher.h>
#include <utils/fancylineedit.h>
#include <utils/qtcassert.h>

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

Q_DECLARE_METATYPE(Core::Internal::ShortcutItem*)

using namespace Utils;

namespace Core {
namespace Internal {

class CommandMappingsPrivate
{
public:
    CommandMappingsPrivate(CommandMappings *parent)
        : q(parent)
    {
        groupBox = new QGroupBox(parent);
        groupBox->setTitle(CommandMappings::tr("Command Mappings"));

        filterEdit = new FancyLineEdit(groupBox);
        filterEdit->setFiltering(true);

        commandList = new QTreeWidget(groupBox);
        commandList->setRootIsDecorated(false);
        commandList->setUniformRowHeights(true);
        commandList->setSortingEnabled(true);
        commandList->setColumnCount(3);

        QTreeWidgetItem *item = commandList->headerItem();
        item->setText(2, CommandMappings::tr("Target"));
        item->setText(1, CommandMappings::tr("Label"));
        item->setText(0, CommandMappings::tr("Command"));

        defaultButton = new QPushButton(CommandMappings::tr("Reset All"), groupBox);
        defaultButton->setToolTip(CommandMappings::tr("Reset all to default."));

        importButton = new QPushButton(CommandMappings::tr("Import..."), groupBox);
        exportButton = new QPushButton(CommandMappings::tr("Export..."), groupBox);

        targetEditGroup = new QGroupBox(CommandMappings::tr("Target Identifier"), parent);
        targetEditGroup->setEnabled(false);

        targetEdit = new FancyLineEdit(targetEditGroup);
        targetEdit->setAutoHideButton(FancyLineEdit::Right, true);
        targetEdit->setPlaceholderText(QString());
        targetEdit->setFiltering(true);
        targetEdit->setValidationFunction([this](FancyLineEdit *, QString *) {
            return !q->hasConflicts();
        });

        resetButton = new QPushButton(targetEditGroup);
        resetButton->setToolTip(CommandMappings::tr("Reset to default."));
        resetButton->setText(CommandMappings::tr("Reset"));

        QLabel *infoLabel = new QLabel(targetEditGroup);
        infoLabel->setTextFormat(Qt::RichText);

        QHBoxLayout *hboxLayout1 = new QHBoxLayout();
        hboxLayout1->addWidget(defaultButton);
        hboxLayout1->addStretch();
        hboxLayout1->addWidget(importButton);
        hboxLayout1->addWidget(exportButton);

        QHBoxLayout *hboxLayout = new QHBoxLayout();
        hboxLayout->addWidget(filterEdit);

        QVBoxLayout *vboxLayout1 = new QVBoxLayout(groupBox);
        vboxLayout1->addLayout(hboxLayout);
        vboxLayout1->addWidget(commandList);
        vboxLayout1->addLayout(hboxLayout1);

        targetLabel = new QLabel(CommandMappings::tr("Target:"));

        QHBoxLayout *hboxLayout2 = new QHBoxLayout();
        hboxLayout2->addWidget(targetLabel);
        hboxLayout2->addWidget(targetEdit);
        hboxLayout2->addWidget(resetButton);

        QVBoxLayout *vboxLayout2 = new QVBoxLayout(targetEditGroup);
        vboxLayout2->addLayout(hboxLayout2);
        vboxLayout2->addWidget(infoLabel);

        QVBoxLayout *vboxLayout = new QVBoxLayout(parent);
        vboxLayout->addWidget(groupBox);
        vboxLayout->addWidget(targetEditGroup);

        q->connect(targetEdit, &FancyLineEdit::buttonClicked,
                   q, &CommandMappings::removeTargetIdentifier);
        q->connect(resetButton, &QPushButton::clicked,
                   q, &CommandMappings::resetTargetIdentifier);
        q->connect(exportButton, &QPushButton::clicked,
                   q, &CommandMappings::exportAction);
        q->connect(importButton, &QPushButton::clicked,
                   q, &CommandMappings::importAction);
        q->connect(defaultButton, &QPushButton::clicked,
                   q, &CommandMappings::defaultAction);

        commandList->sortByColumn(0, Qt::AscendingOrder);

        q->connect(filterEdit, &FancyLineEdit::textChanged,
                   q, &CommandMappings::filterChanged);
        q->connect(commandList, &QTreeWidget::currentItemChanged,
                   q, &CommandMappings::commandChanged);
        q->connect(targetEdit, &FancyLineEdit::textChanged,
                   q, &CommandMappings::targetIdentifierChanged);

        new HeaderViewStretcher(commandList->header(), 1);
    }

    CommandMappings *q;

    QGroupBox *groupBox;
    FancyLineEdit *filterEdit;
    QTreeWidget *commandList;
    QPushButton *defaultButton;
    QPushButton *importButton;
    QPushButton *exportButton;
    QGroupBox *targetEditGroup;
    QLabel *targetLabel;
    FancyLineEdit *targetEdit;
    QPushButton *resetButton;
};

} // namespace Internal

CommandMappings::CommandMappings(QWidget *parent)
    : QWidget(parent), d(new Internal::CommandMappingsPrivate(this))
{
}

CommandMappings::~CommandMappings()
{
   delete d;
}

void CommandMappings::setImportExportEnabled(bool enabled)
{
    d->importButton->setVisible(enabled);
    d->exportButton->setVisible(enabled);
}

QTreeWidget *CommandMappings::commandList() const
{
    return d->commandList;
}

QLineEdit *CommandMappings::targetEdit() const
{
    return d->targetEdit;
}

void CommandMappings::setPageTitle(const QString &s)
{
    d->groupBox->setTitle(s);
}

void CommandMappings::setTargetLabelText(const QString &s)
{
    d->targetLabel->setText(s);
}

void CommandMappings::setTargetEditTitle(const QString &s)
{
    d->targetEditGroup->setTitle(s);
}

void CommandMappings::setTargetHeader(const QString &s)
{
    d->commandList->setHeaderLabels(QStringList() << tr("Command") << tr("Label") << s);
}

void CommandMappings::commandChanged(QTreeWidgetItem *current)
{
    if (!current || !current->data(0, Qt::UserRole).isValid()) {
        d->targetEdit->clear();
        d->targetEditGroup->setEnabled(false);
        return;
    }
    d->targetEditGroup->setEnabled(true);
}

void CommandMappings::filterChanged(const QString &f)
{
    for (int i = 0; i < d->commandList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = d->commandList->topLevelItem(i);
        filter(f, item);
    }
}

bool CommandMappings::hasConflicts() const
{
    return true;
}

bool CommandMappings::filter(const QString &filterString, QTreeWidgetItem *item)
{
    bool visible = filterString.isEmpty();
    int columnCount = item->columnCount();
    for (int i = 0; !visible && i < columnCount; ++i) {
        QString text = item->text(i);
        if (HostOsInfo::isMacHost()) {
            // accept e.g. Cmd+E in the filter. the text shows special fancy characters for Cmd
            if (i == columnCount - 1) {
                QKeySequence key = QKeySequence::fromString(text, QKeySequence::NativeText);
                if (!key.isEmpty()) {
                    text = key.toString(QKeySequence::PortableText);
                    text.replace(QLatin1String("Ctrl"), QLatin1String("Cmd"));
                    text.replace(QLatin1String("Meta"), QLatin1String("Ctrl"));
                    text.replace(QLatin1String("Alt"), QLatin1String("Opt"));
                }
            }
        }
        visible |= (bool)text.contains(filterString, Qt::CaseInsensitive);
    }

    int childCount = item->childCount();
    if (childCount > 0) {
        // force visibility if this item matches
        QString leafFilterString = visible ? QString() : filterString;
        for (int i = 0; i < childCount; ++i) {
            QTreeWidgetItem *citem = item->child(i);
            visible |= !filter(leafFilterString, citem); // parent visible if any child visible
        }
    }
    item->setHidden(!visible);
    return !visible;
}

void CommandMappings::setModified(QTreeWidgetItem *item , bool modified)
{
    QFont f = item->font(0);
    f.setItalic(modified);
    item->setFont(0, f);
    item->setFont(1, f);
    f.setBold(modified);
    item->setFont(2, f);
}

QString CommandMappings::filterText() const
{
    return d->filterEdit ? d->filterEdit->text() : QString();
}

} // namespace Core
