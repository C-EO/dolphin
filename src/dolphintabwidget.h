/*
 * SPDX-FileCopyrightText: 2014 Emmanuel Pescosta <emmanuelpescosta099@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DOLPHIN_TAB_WIDGET_H
#define DOLPHIN_TAB_WIDGET_H

#include "dolphinnavigatorswidgetaction.h"
#include "dolphintabpage.h"

#include <QTabWidget>
#include <QUrl>

#include <optional>

class DolphinViewContainer;
class KConfigGroup;

class DolphinTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * @param navigatorsWidget The navigatorsWidget which is always going to be connected
     *                         to the active tabPage.
     */
    explicit DolphinTabWidget(DolphinNavigatorsWidgetAction *navigatorsWidget, QWidget *parent);

    /**
     * Where a newly opened tab should be placed.
     */
    enum class NewTabPosition {
        FollowSetting, ///< Honor openNewTabAfterLastTab setting
        AfterCurrent, ///< After the current tab
        AtEnd, ///< At the end of the tab bar
    };

    /**
     * @return Tab page at the current index (can be 0 if tabs count is smaller than 1)
     */
    DolphinTabPage *currentTabPage() const;

    /**
     * @return the next tab page. If the current active tab is the last tab,
     * it returns the first tab. If there is only one tab, returns nullptr
     */
    DolphinTabPage *nextTabPage() const;

    /**
     * @return the previous tab page. If the current active tab is the first tab,
     * it returns the last tab. If there is only one tab, returns nullptr
     */
    DolphinTabPage *prevTabPage() const;

    /**
     * @return Tab page at the given \a index (can be 0 if the index is out-of-range)
     */
    DolphinTabPage *tabPageAt(const int index) const;

    void saveProperties(KConfigGroup &group) const;
    void readProperties(const KConfigGroup &group);

    /**
     * Refreshes the views of the main window by recreating them according to
     * the given Dolphin settings.
     */
    void refreshViews();

    /**
     * Update the name of the tab with the index \a index.
     */
    void updateTabName(int index);

    /**
     * @return Whether any of the tab pages has @p url opened
     * in their primary or secondary view.
     */
    bool isUrlOpen(const QUrl &url) const;

    /**
     * @return Whether the item with @p url can be found in any view only by switching
     * between already open tabs and scrolling in their primary or secondary view.
     */
    bool isItemVisibleInAnyView(const QUrl &urlOfItem) const;

Q_SIGNALS:
    /**
     * Is emitted when the active view has been changed, by changing the current
     * tab or by activating another view when split view is enabled in the current
     * tab.
     */
    void activeViewChanged(DolphinViewContainer *viewContainer);

    /**
     * Is emitted when the number of open tabs has changed (e.g. by opening or
     * closing a tab)
     */
    void tabCountChanged(int count);

    /**
     * Is emitted when a tab has been closed.
     */
    void rememberClosedTab(const QUrl &url, const QByteArray &state);

    /**
     * Is emitted when the url of the current tab has been changed. This signal
     * is also emitted when the active view has been changed.
     */
    void currentUrlChanged(const QUrl &url);

    /**
     * Is emitted when the url of any tab has been changed (including the current tab).
     */
    void urlChanged(const QUrl &url);

public Q_SLOTS:
    /**
     * Opens a new view with the current URL that is part of a tab and activates
     * the tab.
     */
    void openNewActivatedTab();

    /**
     * Opens a new tab showing the  URL \a primaryUrl and the optional URL
     * \a secondaryUrl and activates the tab.
     */
    void openNewActivatedTab(const QUrl &primaryUrl, const QUrl &secondaryUrl = QUrl());

    /**
     * Opens a new tab in the background showing the URL \a primaryUrl and the
     * optional URL \a secondaryUrl.
     * @return A pointer to the opened DolphinTabPage.
     */
    DolphinTabPage *openNewTab(const QUrl &primaryUrl,
                               const QUrl &secondaryUrl = QUrl(),
                               DolphinTabWidget::NewTabPosition position = DolphinTabWidget::NewTabPosition::FollowSetting);

    /**
     * Opens each directory in \p dirs in a separate tab unless it is already open.
     * If \a splitView is set, 2 directories are collected within one tab.
     * \pre \a dirs must contain at least one url.
     */
    void openDirectories(const QList<QUrl> &dirs, bool splitView);

    /**
     * Opens the directories which contain the files \p files and selects all files.
     * If \a splitView is set, 2 directories are collected within one tab.
     * \pre \a files must contain at least one url.
     */
    void openFiles(const QList<QUrl> &files, bool splitView);

    /**
     * Closes the currently active tab.
     */
    void closeTab();

    /**
     * Closes the tab with the index \a index and activates the tab with index - 1.
     */
    void closeTab(const int index);

    /**
     * Activates the tab with the index \a index.
     */
    void activateTab(const int index);

    /**
     * Activates the last tab in the tab bar.
     */
    void activateLastTab();

    /**
     * Activates the next tab in the tab bar.
     * If the current active tab is the last tab, it activates the first tab.
     */
    void activateNextTab();

    /**
     * Activates the previous tab in the tab bar.
     * If the current active tab is the first tab, it activates the last tab.
     */
    void activatePrevTab();

    /**
     * Is called when the user wants to reopen a previously closed tab from
     * the recent tabs menu.
     */
    void restoreClosedTab(const QByteArray &state);

    /** Copies all selected items to the inactive view. */
    void copyToInactiveSplitView();

    /** Moves all selected items to the inactive view. */
    void moveToInactiveSplitView();

private Q_SLOTS:
    /**
     * Opens the tab with the index \a index in a new Dolphin instance and closes
     * this tab.
     */
    void detachTab(int index);

    /**
     * Opens a new tab showing the url from tab at the given \a index and
     * activates the tab.
     */
    void openNewActivatedTab(int index);

    /**
     * Is connected to the KTabBar signal receivedDragMoveEvent.
     * Allows dragging and dropping files onto tabs.
     */
    void tabDragMoveEvent(int tab, QDragMoveEvent *event);

    /**
     * Is connected to the KTabBar signal receivedDropEvent.
     * Allows dragging and dropping files onto tabs.
     */
    void tabDropEvent(int tab, QDropEvent *event);

    /**
     * The active view url of a tab has been changed so update the text and the
     * icon of the corresponding tab.
     */
    void tabUrlChanged(const QUrl &url);

    void currentTabChanged(int index);

    /**
     * Calls DolphinTabPage::setCustomLabel(label) for the tab at @p index
     * and propagates that change to the DolphinTabBar.
     * @see DolphinTabPage::setCustomLabel().
     */
    void renameTab(int index, const QString &label);

protected:
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

private:
    /**
     * @param tabPage The tab page to get the name of
     * @return The name of the tab page
     */
    QString tabName(DolphinTabPage *tabPage) const;

    struct ViewIndex {
        const int tabIndex;
        const bool isInPrimaryView;
    };

    /**
     * Getter for a view container.
     * @param viewIndex specifies the tab and the view within that tab.
     * @return the view container specified in @p viewIndex or nullptr if it doesn't exist.
     */
    DolphinViewContainer *viewContainerAt(ViewIndex viewIndex) const;

    /**
     * Makes the view container specified in @p viewIndex become the active view container within this tab widget.
     * @param viewIndex Specifies the tab to activate and the view container within the tab to activate.
     * @return the freshly activated view container or nullptr if there is no view container at @p viewIndex.
     */
    DolphinViewContainer *activateViewContainerAt(ViewIndex viewIndex);

    /**
     * Get the position of the view within this widget that is open at @p directory.
     * @param directory The URL of the directory we want to find.
     * @return a small struct containing the tab index of the view and whether it is
     * in the primary view. A std::nullopt is returned if there is no view open for @p directory.
     */
    const std::optional<const ViewIndex> viewOpenAtDirectory(const QUrl &directory) const;

    /**
     * Get the position of the view within this widget that has @p item in the view.
     * This means that the item can be seen by the user in that view when scrolled to the right position.
     * If the view has folders expanded and @p item is one of them, the view will also be returned.
     * @param item The URL of the item we want to find.
     * @return a small struct containing the tab index of the view and whether it is
     * in the primary view. A std::nullopt is returned if there is no view open that has @p item visible anywhere.
     */
    const std::optional<const ViewIndex> viewShowingItem(const QUrl &item) const;

private:
    QPointer<DolphinTabPage> m_lastViewedTab;
    QPointer<DolphinNavigatorsWidgetAction> m_navigatorsWidget;
};

#endif
