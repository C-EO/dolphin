/*
 * SPDX-FileCopyrightText: 2006 Peter Penz (peter.penz@gmx.at) and Patrice Tremblay
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef STATUSBARSPACEINFO_H
#define STATUSBARSPACEINFO_H

#include <KMessageWidget>

#include <QUrl>
#include <QWidget>

class QHideEvent;
class QShowEvent;
class QMenu;
class QMouseEvent;
class QToolButton;
class QWidgetAction;

class KCapacityBar;

class SpaceInfoObserver;

/**
 * @short Shows the available space for the volume represented
 *        by the given URL as part of the status bar.
 */
class StatusBarSpaceInfo : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarSpaceInfo(QWidget *parent = nullptr);
    ~StatusBarSpaceInfo() override;

    /**
     * Works similar to QWidget::setVisible() except that this will postpone showing the widget until space info has been retrieved. Hiding happens instantly.
     *
     * @param shown Whether this widget is supposed to be visible long-term
     */
    void setShown(bool shown);
    void setUrl(const QUrl &url);
    QUrl url() const;

    void update();

Q_SIGNALS:
    /**
     * Requests for @p message with the given @p messageType to be shown to the user in a non-modal way.
     */
    void showMessage(const QString &message, KMessageWidget::MessageType messageType);

    /**
     * Requests for a progress update to be shown to the user in a non-modal way.
     * @param currentlyRunningTaskTitle     The task that is currently progressing.
     * @param installationProgressPercent   The current percentage of completion.
     */
    void showInstallationProgress(const QString &currentlyRunningTaskTitle, int installationProgressPercent);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    QSize minimumSizeHint() const override;

private Q_SLOTS:
    void slotValuesChanged();

private:
    // The following three methods are only for private use.
    using QWidget::hide; // Use StatusBarSpaceInfo::setShown() instead.
    using QWidget::setVisible; // Use StatusBarSpaceInfo::setShown() instead.
    using QWidget::show; // Use StatusBarSpaceInfo::setShown() instead.

private:
    QScopedPointer<SpaceInfoObserver> m_observer;
    KCapacityBar *m_capacityBar;
    QToolButton *m_textInfoButton;
    QUrl m_url;
    /** Whether m_observer has already retrieved space information for the current url. */
    bool m_hasSpaceInfo;
    /** Whether this widget is supposed to be visible long-term. @see StatusBarSpaceInfo::setShown(). */
    bool m_shown;
};

#endif
