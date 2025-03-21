/*
 * SPDX-FileCopyrightText: 2011 Peter Penz <peter.penz19@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KITEMLISTVIEWLAYOUTER_H
#define KITEMLISTVIEWLAYOUTER_H

#include "dolphin_export.h"

#include <QObject>
#include <QRectF>
#include <QSet>
#include <QSizeF>
#include <QVector>

class KItemModelBase;
class KItemListSizeHintResolver;

/**
 * @brief Internal helper class for KItemListView to layout the items.
 *
 * The layouter is capable to align the items within a grid. If the
 * scroll-direction is horizontal the column-width of the grid can be
 * variable. If the scroll-direction is vertical the row-height of
 * the grid can be variable.
 *
 * The layouter is implemented in a way that it postpones the expensive
 * layout operation until a property is read the first time after
 * marking the layouter as dirty (see markAsDirty()). This means that
 * changing properties of the layouter is not expensive, only the
 * first read of a property can get expensive.
 */
class DOLPHIN_EXPORT KItemListViewLayouter : public QObject
{
    Q_OBJECT

public:
    explicit KItemListViewLayouter(KItemListSizeHintResolver *sizeHintResolver, QObject *parent = nullptr);
    ~KItemListViewLayouter() override;

    void setScrollOrientation(Qt::Orientation orientation);
    Qt::Orientation scrollOrientation() const;

    void setSize(const QSizeF &size);
    QSizeF size() const;

    void setItemSize(const QSizeF &size);
    QSizeF itemSize() const;

    /**
     * Margin between the rows and columns of items.
     */
    void setItemMargin(const QSizeF &margin);
    QSizeF itemMargin() const;

    /**
     * Sets the height of the header that is always aligned
     * at the top. A height of <= 0.0 means that no header is
     * used.
     */
    void setHeaderHeight(qreal height);
    qreal headerHeight() const;

    /**
     * Sets the height of the group header that is used
     * to indicate a new item group.
     */
    void setGroupHeaderHeight(qreal height);
    qreal groupHeaderHeight() const;

    /**
     * Sets the margin between the last items of the group n and
     * the group header for the group n + 1.
     */
    void setGroupHeaderMargin(qreal margin);
    qreal groupHeaderMargin() const;

    void setScrollOffset(qreal scrollOffset);
    qreal scrollOffset() const;

    qreal maximumScrollOffset() const;

    void setItemOffset(qreal scrollOffset);
    qreal itemOffset() const;

    qreal maximumItemOffset() const;

    void setModel(const KItemModelBase *model);
    const KItemModelBase *model() const;

    /**
     * @return The first (at least partly) visible index. -1 is returned
     *         if the item count is 0.
     */
    int firstVisibleIndex() const;

    /**
     * @return The last (at least partly) visible index. -1 is returned
     *         if the item count is 0.
     */
    int lastVisibleIndex() const;

    /**
     * @return Rectangle of the item with the index \a index.
     *         The top/left of the bounding rectangle is related to
     *         the top/left of the KItemListView. An empty rectangle
     *         is returned if an invalid index is given.
     */
    QRectF itemRect(int index) const;

    /**
     * @return Rectangle of the group header for the item with the
     *         index \a index. Note that the layouter does not check
     *         whether the item really has a header: Usually only
     *         the first item of a group gets a header (see
     *         isFirstGroupItem()).
     */
    QRectF groupHeaderRect(int index) const;

    /**
     * @return Column of the item with the index \a index.
     *         -1 is returned if an invalid index is given.
     */
    int itemColumn(int index) const;

    /**
     * @return Row of the item with the index \a index.
     *         -1 is returned if an invalid index is given.
     */
    int itemRow(int index) const;

    /**
     * @return Maximum number of (at least partly) visible items for
     *         the given size.
     */
    int maximumVisibleItems() const;

    /**
     * @return True if the item with the index \p itemIndex
     *         is the first item within a group.
     */
    bool isFirstGroupItem(int itemIndex) const;

    /**
     * Marks the layouter as dirty. This means as soon as a property of
     * the layouter gets read, an expensive relayout will be done.
     */
    void markAsDirty();

    inline int columnCount() const
    {
        return m_columnCount;
    }

    /**
     * Set the bottom offset for moving the view so that the small overlayed statusbar
     * won't cover any items by accident.
     */
    void setStatusBarOffset(int offset);

#ifndef QT_NO_DEBUG
    /**
     * @return True if the layouter has been marked as dirty and hence has
     *         not called yet doLayout(). Is enabled only in the debugging
     *         mode, as it is not useful to check the dirty state otherwise.
     */
    bool isDirty();
#endif

private:
    void doLayout();
    void updateVisibleIndexes();
    bool createGroupHeaders();

    /**
     * @return Minimum width of group headers when grouping is enabled in the horizontal
     *         alignment mode. The header alignment is done like this:
     *         Header-1 Header-2 Header-3
     *         Item 1   Item 4   Item 7
     *         Item 2   Item 5   Item 8
     *         Item 3   Item 6   Item 9
     */
    qreal minimumGroupHeaderWidth() const;

private:
    bool m_dirty;
    bool m_visibleIndexesDirty;

    Qt::Orientation m_scrollOrientation;
    QSizeF m_size;

    QSizeF m_itemSize;
    QSizeF m_itemMargin;
    qreal m_headerHeight;
    const KItemModelBase *m_model;
    KItemListSizeHintResolver *m_sizeHintResolver;

    qreal m_scrollOffset;
    qreal m_maximumScrollOffset;

    qreal m_itemOffset;
    qreal m_maximumItemOffset;

    int m_firstVisibleIndex;
    int m_lastVisibleIndex;

    qreal m_columnWidth;
    qreal m_xPosInc;
    int m_columnCount;

    QVector<qreal> m_rowOffsets;
    QVector<qreal> m_columnOffsets;

    // Stores all item indexes that are the first item of a group.
    // Assures fast access for KItemListViewLayouter::isFirstGroupItem().
    QSet<int> m_groupItemIndexes;
    qreal m_groupHeaderHeight;
    qreal m_groupHeaderMargin;

    struct ItemInfo {
        int column;
        int row;
    };
    QVector<ItemInfo> m_itemInfos;

    int m_statusBarOffset;

    friend class KItemListControllerTest;
};

#endif
