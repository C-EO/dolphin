/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2022 Felix Ernst <felixernst@zohomail.eu>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bottombarcontentscontainer.h"

#include "dolphin_generalsettings.h"
#include "dolphincontextmenu.h"
#include "dolphinmainwindow.h"
#include "dolphinremoveaction.h"
#include "kitemviews/kfileitemlisttostring.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include <unordered_set>

using namespace SelectionMode;

BottomBarContentsContainer::BottomBarContentsContainer(KActionCollection *actionCollection, QWidget *parent) :
    QWidget{parent},
    m_actionCollection{actionCollection}
{
    // We will mostly interact with m_layout when changing the contents and not care about the other internal hierarchy.
    m_layout = new QHBoxLayout(this);
}

void BottomBarContentsContainer::resetContents(BottomBar::Contents contents)
{
    emptyBarContents();

    // A label is added in many of the methods below. We only know its size a bit later and if it should be hidden.
    QTimer::singleShot(10, this, [this](){ updateExplanatoryLabelVisibility(); });

    Q_CHECK_PTR(m_actionCollection);
    m_contents = contents;
    switch (contents) {
    case BottomBar::CopyContents:
        return addCopyContents();
    case BottomBar::CopyLocationContents:
        return addCopyLocationContents();
    case BottomBar::CopyToOtherViewContents:
        return addCopyToOtherViewContents();
    case BottomBar::CutContents:
        return addCutContents();
    case BottomBar::DeleteContents:
        return addDeleteContents();
    case BottomBar::DuplicateContents:
        return addDuplicateContents();
    case BottomBar::GeneralContents:
        return addGeneralContents();
    case BottomBar::PasteContents:
        return addPasteContents();
    case BottomBar::MoveToOtherViewContents:
        return addMoveToOtherViewContents();
    case BottomBar::MoveToTrashContents:
        return addMoveToTrashContents();
    case BottomBar::RenameContents:
        return addRenameContents();
    }
}

void BottomBarContentsContainer::updateForNewWidth()
{
    if (m_contents == BottomBar::GeneralContents) {
        Q_ASSERT(m_overflowButton);
        if (unusedSpace() < 0) {
            // The bottom bar is overflowing! We need to hide some of the widgets.
            for (auto i = m_generalBarActions.rbegin(); i != m_generalBarActions.rend(); ++i) {
                if (!i->isWidgetVisible()) {
                    continue;
                }
                i->widget()->setVisible(false);

                // Add the action to the overflow.
                auto overflowMenu = m_overflowButton->menu();
                if (overflowMenu->actions().isEmpty()) {
                    overflowMenu->addAction(i->action());
                } else {
                    overflowMenu->insertAction(overflowMenu->actions().at(0), i->action());
                }
                m_overflowButton->setVisible(true);
                if (unusedSpace() >= 0) {
                    break; // All widgets fit now.
                }
            }
        } else {
            // We have some unusedSpace(). Let's check if we can maybe add more of the contextual action's widgets.
            for (auto i = m_generalBarActions.begin(); i != m_generalBarActions.end(); ++i) {
                if (i->isWidgetVisible()) {
                    continue;
                }
                if (!i->widget()) {
                    i->newWidget(this);
                    i->widget()->setVisible(false);
                    m_layout->insertWidget(m_layout->count() - 1, i->widget()); // Insert before m_overflowButton
                }
                if (unusedSpace() < i->widget()->sizeHint().width()) {
                    // It doesn't fit. We keep it invisible.
                    break;
                }
                i->widget()->setVisible(true);

                // Remove the action from the overflow.
                auto overflowMenu = m_overflowButton->menu();
                overflowMenu->removeAction(i->action());
                if (overflowMenu->isEmpty()) {
                    m_overflowButton->setVisible(false);
                }
            }
        }
    }

    // Hide the leading explanation if it doesn't fit. The buttons are labeled clear enough that this shouldn't be a big UX problem.
    updateExplanatoryLabelVisibility();
}

void BottomBarContentsContainer::slotSelectionChanged(const KFileItemList &selection, const QUrl &baseUrl)
{
    if (m_contents == BottomBar::GeneralContents) {
        auto contextActions = contextActionsFor(selection, baseUrl);
        m_generalBarActions.clear();
        if (contextActions.empty()) {
            Q_ASSERT(qobject_cast<BottomBar *>(parentWidget()->parentWidget()->parentWidget()));
            if (isVisibleTo(parentWidget()->parentWidget()->parentWidget()->parentWidget())) { // is the bar visible
                Q_EMIT barVisibilityChangeRequested(false);
            }
        } else {
            for (auto i = contextActions.begin(); i != contextActions.end(); ++i) {
                m_generalBarActions.emplace_back(ActionWithWidget{*i});
            }
            resetContents(BottomBar::GeneralContents);

            Q_EMIT barVisibilityChangeRequested(true);
        }
    }
    updateMainActionButton(selection);
}

void BottomBarContentsContainer::addCopyContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be copied."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to copy files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort Copying"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *copyButton = new QPushButton(this);
    // We claim to have PasteContents already so triggering the copy action next won't instantly hide the bottom bar.
    connect(copyButton, &QAbstractButton::clicked, [this]() {
        if (GeneralSettings::showPasteBarAfterCopying()) {
            m_contents = BottomBar::Contents::PasteContents;
        }
    });
    // Connect the copy action as a second step.
    m_mainAction = ActionWithWidget(m_actionCollection->action(KStandardAction::name(KStandardAction::Copy)), copyButton);
    // Finally connect the lambda that actually changes the contents to the PasteContents.
    connect(copyButton, &QAbstractButton::clicked, [this]() {
        if (GeneralSettings::showPasteBarAfterCopying()) {
            resetContents(BottomBar::Contents::PasteContents); // resetContents() needs to be connected last because
                // it instantly deletes the button and then the other slots won't be called.
        }
        Q_EMIT leaveSelectionModeRequested();
    });
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(copyButton);
}

void BottomBarContentsContainer::addCopyLocationContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select one file or folder whose location should be copied."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to copy the location of files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort Copying"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *copyLocationButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(QStringLiteral("copy_location")), copyLocationButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(copyLocationButton);
}

void BottomBarContentsContainer::addCopyToOtherViewContents()
{
    // i18n: "Copy over" refers to copying to the other split view area that is currently visible to the user.
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be copied over."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to copy the location of files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort Copying"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *copyToOtherViewButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(QStringLiteral("copy_to_inactive_split_view")), copyToOtherViewButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(copyToOtherViewButton);
}

void BottomBarContentsContainer::addCutContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be cut."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to cut files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort Cutting"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *cutButton = new QPushButton(this);
    // We claim to have PasteContents already so triggering the cut action next won't instantly hide the bottom bar.
    connect(cutButton, &QAbstractButton::clicked, [this]() {
        if (GeneralSettings::showPasteBarAfterCopying()) {
            m_contents = BottomBar::Contents::PasteContents;
        }
    });
    // Connect the cut action as a second step.
    m_mainAction = ActionWithWidget(m_actionCollection->action(KStandardAction::name(KStandardAction::Cut)), cutButton);
    // Finally connect the lambda that actually changes the contents to the PasteContents.
    connect(cutButton, &QAbstractButton::clicked, [this](){
        if (GeneralSettings::showPasteBarAfterCopying()) {
            resetContents(BottomBar::Contents::PasteContents); // resetContents() needs to be connected last because
                // it instantly deletes the button and then the other slots won't be called.
        }
        Q_EMIT leaveSelectionModeRequested();
    });
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(cutButton);
}

void BottomBarContentsContainer::addDeleteContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be permanently deleted."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to delete files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *deleteButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(KStandardAction::name(KStandardAction::DeleteFile)), deleteButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(deleteButton);
}

void BottomBarContentsContainer::addDuplicateContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be duplicated here."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to duplicate files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort Duplicating"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *duplicateButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(QStringLiteral("duplicate")), duplicateButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(duplicateButton);
}

void BottomBarContentsContainer::addGeneralContents()
{
    if (!m_overflowButton) {
        m_overflowButton = new QToolButton{this};
        // i18n: This button appears in a bar if there isn't enough horizontal space to fit all the other buttons.
        // The small icon-only button opens a menu that contains the actions that didn't fit on the bar.
        // Since this is an icon-only button this text will only appear as a tooltip and as accessibility text.
        m_overflowButton->setToolTip(i18nc("@action", "More"));
        m_overflowButton->setAccessibleName(m_overflowButton->toolTip());
        m_overflowButton->setIcon(QIcon::fromTheme(QStringLiteral("view-more-horizontal-symbolic")));
        m_overflowButton->setMenu(new QMenu{m_overflowButton});
        m_overflowButton->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
        m_overflowButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding); // Makes sure it has the same height as the labeled buttons.
        m_layout->addWidget(m_overflowButton);
    } else {
        m_overflowButton->menu()->actions().clear();
        // The overflowButton should be part of the calculation for needed space so we set it visible in regards to unusedSpace().
        m_overflowButton->setVisible(true);
    }

    // We first add all the m_generalBarActions to the bar until the bar is full.
    auto i = m_generalBarActions.begin();
    for (; i != m_generalBarActions.end(); ++i) {
        if (i->action()->isVisible()) {
            if (i->widget()) {
                i->widget()->setEnabled(i->action()->isEnabled());
            } else {
                i->newWidget(this);
                i->widget()->setVisible(false);
                m_layout->insertWidget(m_layout->count() - 1, i->widget()); // Insert before m_overflowButton
            }
            if (unusedSpace() < i->widget()->sizeHint().width()) {
                break; // The bar is too full already. We keep it invisible.
            } else {
                i->widget()->setVisible(true);
            }
        }
    }
    // We are done adding widgets to the bar so either we were able to fit all the actions in there
    m_overflowButton->setVisible(false);
    // …or there are more actions left which need to be put into m_overflowButton.
    for (; i != m_generalBarActions.end(); ++i) {
        m_overflowButton->menu()->addAction(i->action());

        // The overflowButton is set visible if there is actually an action in it.
        if (!m_overflowButton->isVisible() && i->action()->isVisible() && !i->action()->isSeparator()) {
            m_overflowButton->setVisible(true);
        }
    }
}

void BottomBarContentsContainer::addMoveToOtherViewContents()
{
    // i18n: "Move over" refers to moving to the other split view area that is currently visible to the user.
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be moved over."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to copy the location of files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort Moving"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *moveToOtherViewButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(QStringLiteral("move_to_inactive_split_view")), moveToOtherViewButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(moveToOtherViewButton);
}

void BottomBarContentsContainer::addMoveToTrashContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explaining the next step in a process", "Select the files and folders that should be moved to the Trash."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process of moving files to the trash by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Abort"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *moveToTrashButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(KStandardAction::name(KStandardAction::MoveToTrash)), moveToTrashButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(moveToTrashButton);
}

void BottomBarContentsContainer::addPasteContents()
{
    m_explanatoryLabel = new QLabel(xi18n("<para>The selected files and folders were added to the Clipboard. "
            "Now the <emphasis>Paste</emphasis> action can be used to transfer them from the Clipboard "
            "to any other location. They can even be transferred to other applications by using their "
            "respective <emphasis>Paste</emphasis> actions.</para>"), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    auto *vBoxLayout = new QVBoxLayout(this);
    m_layout->addLayout(vBoxLayout);

    /** We are in "PasteContents" mode which means hiding the bottom bar is impossible.
     * So we first have to claim that we have different contents before requesting to leave selection mode. */
    auto actuallyLeaveSelectionMode = [this]() {
        m_contents = BottomBar::Contents::CopyLocationContents;
        Q_EMIT leaveSelectionModeRequested();
    };

    auto *pasteButton = new QPushButton(this);
    copyActionDataToButton(pasteButton, m_actionCollection->action(KStandardAction::name(KStandardAction::Paste)));
    pasteButton->setText(i18nc("@action A more elaborate and clearly worded version of the Paste action", "Paste from Clipboard"));
    connect(pasteButton, &QAbstractButton::clicked, this, actuallyLeaveSelectionMode);
    vBoxLayout->addWidget(pasteButton);

    auto *dismissButton = new QToolButton(this);
    dismissButton->setText(i18nc("@action Dismisses a bar explaining how to use the Paste action", "Dismiss this Reminder"));
    connect(dismissButton, &QAbstractButton::clicked, this, actuallyLeaveSelectionMode);
    auto *dontRemindAgainAction = new QAction(i18nc("@action Dismisses an explanatory area and never shows it again", "Don't remind me again"), this);
    connect(dontRemindAgainAction, &QAction::triggered, this, []() {
        GeneralSettings::setShowPasteBarAfterCopying(false);
    });
    connect(dontRemindAgainAction, &QAction::triggered, this, actuallyLeaveSelectionMode);
    auto *dismissButtonMenu = new QMenu(dismissButton);
    dismissButtonMenu->addAction(dontRemindAgainAction);
    dismissButton->setMenu(dismissButtonMenu);
    dismissButton->setPopupMode(QToolButton::MenuButtonPopup);
    vBoxLayout->addWidget(dismissButton);

    m_explanatoryLabel->setMaximumHeight(pasteButton->sizeHint().height() + dismissButton->sizeHint().height() + m_explanatoryLabel->fontMetrics().height());
}

void BottomBarContentsContainer::addRenameContents()
{
    m_explanatoryLabel = new QLabel(i18nc("@info explains the next step in a process", "Select the file or folder that should be renamed.\nBulk renaming is possible when multiple items are selected."), this);
    m_explanatoryLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_explanatoryLabel->setWordWrap(true);
    m_layout->addWidget(m_explanatoryLabel);

    // i18n: Aborts the current step-by-step process to delete files by leaving the selection mode.
    auto *cancelButton = new QPushButton(i18nc("@action:button", "Stop Renaming"), this);
    connect(cancelButton, &QAbstractButton::clicked, this, &BottomBarContentsContainer::leaveSelectionModeRequested);
    m_layout->addWidget(cancelButton);

    auto *renameButton = new QPushButton(this);
    m_mainAction = ActionWithWidget(m_actionCollection->action(KStandardAction::name(KStandardAction::RenameFile)), renameButton);
    updateMainActionButton(KFileItemList());
    m_layout->addWidget(renameButton);
}

void BottomBarContentsContainer::emptyBarContents()
{
    QLayoutItem *child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        if (auto *childLayout = child->layout()) {
            QLayoutItem *grandChild;
            while ((grandChild = childLayout->takeAt(0)) != nullptr) {
                delete grandChild->widget(); // delete the widget
                delete grandChild;   // delete the layout item
            }
        }
        delete child->widget(); // delete the widget
        delete child;   // delete the layout item
    }
}

std::vector<QAction *> BottomBarContentsContainer::contextActionsFor(const KFileItemList& selectedItems, const QUrl& baseUrl)
{
    if (selectedItems.isEmpty()) {
        // There are no contextual actions to show for these items.
        // We might even want to hide this bar in this case. To make this clear, we reset m_internalContextMenu.
        m_internalContextMenu.release()->deleteLater();
        return std::vector<QAction *>{};
    }

    std::vector<QAction *> contextActions;

    // We always want to show the most important actions at the beginning
    contextActions.emplace_back(m_actionCollection->action(KStandardAction::name(KStandardAction::Copy)));
    contextActions.emplace_back(m_actionCollection->action(KStandardAction::name(KStandardAction::Cut)));
    contextActions.emplace_back(m_actionCollection->action(KStandardAction::name(KStandardAction::RenameFile)));
    contextActions.emplace_back(m_actionCollection->action(KStandardAction::name(KStandardAction::MoveToTrash)));

    // We are going to add the actions from the right-click context menu for the selected items.
    auto *dolphinMainWindow = qobject_cast<DolphinMainWindow *>(window());
    Q_CHECK_PTR(dolphinMainWindow);
    if (!m_fileItemActions) {
        m_fileItemActions = new KFileItemActions(this);
        m_fileItemActions->setParentWidget(dolphinMainWindow);
        connect(m_fileItemActions, &KFileItemActions::error, this, &BottomBarContentsContainer::error);
    }
    m_internalContextMenu = std::make_unique<DolphinContextMenu>(dolphinMainWindow, selectedItems.constFirst(), selectedItems, baseUrl, m_fileItemActions);
    auto internalContextMenuActions = m_internalContextMenu->actions();

    // There are some actions which we wouldn't want to add. We remember them in the actionsThatShouldntBeAdded set.
    // We don't want to add the four basic actions again which were already added to the top.
    std::unordered_set<QAction *> actionsThatShouldntBeAdded{contextActions.begin(), contextActions.end()};
    // "Delete" isn't really necessary to add because we have "Move to Trash" already. It is also more dangerous so let's exclude it.
    actionsThatShouldntBeAdded.insert(m_actionCollection->action(KStandardAction::name(KStandardAction::DeleteFile)));

    // KHamburgerMenu would only be visible if there is no menu available anywhere on the user interface. This might be useful for recovery from
    // such a situation in theory but a bar with context dependent actions doesn't really seem like the right place for it.
    Q_ASSERT(internalContextMenuActions.first()->icon().name() == m_actionCollection->action(KStandardAction::name(KStandardAction::HamburgerMenu))->icon().name());
    internalContextMenuActions.removeFirst();

    for (auto it = internalContextMenuActions.constBegin(); it != internalContextMenuActions.constEnd(); ++it) {
        if (actionsThatShouldntBeAdded.count(*it)) {
            continue; // Skip this action.
        }
        if (!qobject_cast<DolphinRemoveAction *>(*it)) { // We already have a "Move to Trash" action so we don't want a DolphinRemoveAction.
            // We filter duplicate separators here so we won't have to deal with them later.
            if (!contextActions.back()->isSeparator() || !(*it)->isSeparator()) {
                contextActions.emplace_back((*it));
            }
        }
    }
    return contextActions;
}

int BottomBarContentsContainer::unusedSpace() const
{
    int sumOfPreferredWidths = m_layout->contentsMargins().left() + m_layout->contentsMargins().right();
    if (m_overflowButton) {
        sumOfPreferredWidths += m_overflowButton->sizeHint().width();
    }
    for (int i = 0; i < m_layout->count(); ++i) {
        auto widget = m_layout->itemAt(i)->widget();
        if (widget && !widget->isVisibleTo(widget->parentWidget())) {
            continue; // We don't count invisible widgets.
        }
        sumOfPreferredWidths += m_layout->itemAt(i)->sizeHint().width() + m_layout->spacing();
    }
    Q_ASSERT(qobject_cast<BottomBar *>(parentWidget()->parentWidget()->parentWidget()));
    const int totalBarWidth = parentWidget()->parentWidget()->parentWidget()->width();
    return totalBarWidth - sumOfPreferredWidths - 20; // We consider all space used when there are only 20 pixels left
                                                      // so there is some room to breath and not too much wonkyness while resizing.
}

void BottomBarContentsContainer::updateExplanatoryLabelVisibility()
{
    if (!m_explanatoryLabel) {
        return;
    }
    if (m_explanatoryLabel->isVisible()) {
        m_explanatoryLabel->setVisible(unusedSpace() > 0);
    } else {
        // We only want to re-show the label when it fits comfortably so the computation below adds another "+20".
        m_explanatoryLabel->setVisible(unusedSpace() > m_explanatoryLabel->sizeHint().width() + 20);
    }
}

void BottomBarContentsContainer::updateMainActionButton(const KFileItemList& selection)
{
    if (!m_mainAction.widget()) {
        return;
    }
    Q_ASSERT(qobject_cast<QAbstractButton *>(m_mainAction.widget()));

    // Users are nudged towards selecting items by having the button disabled when nothing is selected.
    m_mainAction.widget()->setEnabled(selection.count() > 0 && m_mainAction.action()->isEnabled());
    QFontMetrics fontMetrics = m_mainAction.widget()->fontMetrics();

    QString buttonText;
    switch (m_contents) {
    case BottomBar::CopyContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Copy action",
                            "Copy %2 to the Clipboard", "Copy %2 to the Clipboard", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    case BottomBar::CopyLocationContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Copy Location action",
                            "Copy the Location of %2 to the Clipboard", "Copy the Location of %2 to the Clipboard", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    case BottomBar::CutContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Cut action",
                            "Cut %2 to the Clipboard", "Cut %2 to the Clipboard", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    case BottomBar::DeleteContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Delete action",
                            "Permanently Delete %2", "Permanently Delete %2", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    case BottomBar::DuplicateContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Duplicate action",
                            "Duplicate %2", "Duplicate %2", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    case BottomBar::MoveToTrashContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Trash action",
                            "Move %2 to the Trash", "Move %2 to the Trash", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    case BottomBar::RenameContents:
        buttonText = i18ncp("@action A more elaborate and clearly worded version of the Rename action",
                            "Rename %2", "Rename %2", selection.count(),
                            fileItemListToString(selection, fontMetrics.averageCharWidth() * 20, fontMetrics));
        break;
    default:
        return;
    }
    if (buttonText != QStringLiteral("NULL")) {
        static_cast<QAbstractButton *>(m_mainAction.widget())->setText(buttonText);

        // The width of the button has changed. We might want to hide the label so the full button text fits on the bar.
        updateExplanatoryLabelVisibility();
    }
}