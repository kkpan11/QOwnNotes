/*
 * Copyright (c) 2014-2026 Patrizio Bekerle -- <patrizio@bekerle.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#pragma once

#include <QFutureWatcher>
#include <QHash>
#include <QSet>
#include <QTreeWidget>

class QTextDocument;
class QTextCursor;
class QTextBlock;
class QMouseEvent;
class QTreeWidgetItem;

struct Node {
    QString text;
    int pos;
    int elementType;

    bool operator==(const Node &node) const {
        return text == node.text && pos == node.pos && elementType == node.elementType;
    }
};

class NavigationWidget : public QTreeWidget {
    Q_OBJECT

   public:
    explicit NavigationWidget(QWidget *parent = 0);

    void parse(const QTextDocument *document, int textCursorPosition, int noteId = -1);
    static QVector<Node> parseDocument(const QTextDocument *const document);
    static int headingPositionForCursor(const QTextCursor &cursor);
    void clearCollapsedStateForNote(int noteId);

    void selectItemForCursorPosition(int position);

    int getNextHeadingPosition(int currentPosition) const;
    int getPreviousHeadingPosition(int currentPosition) const;

   private slots:
    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void onItemClicked(QTreeWidgetItem *current, int column);
    void onItemChanged(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &pos);
    void renameHeadingTriggered();
    void onItemExpandedOrCollapsed(QTreeWidgetItem *item);

   private:
    void buildNavTree(const QVector<Node> &nodes);
    void doParse();
    static bool isSetextUnderlineBlock(const QString &text);
    static bool isAtxHeadingBlock(const QString &text);
    static bool isNavigationHeadingBlock(const QTextBlock &block);
    void mousePressEvent(QMouseEvent *event) override;

   signals:
    void positionClicked(int position);
    void headingRenamed(int position, const QString &oldText, const QString &newText);

   private:
    const QTextDocument *_doc;
    QHash<int, QTreeWidgetItem *> _lastHeadingItemList;
    QVector<Node> _navigationTreeNodes;
    int _cursorPosition;
    int _currentNoteId = -1;
    bool _restoringExpandState = false;

    // Per-note collapsed heading state, keyed by note ID.
    // The value is a set of collapsed heading keys ("level:text").
    // Only notes where the user actively changed expand/collapse state are stored.
    QHash<int, QSet<QString>> _collapsedHeadingsPerNote;

    QTreeWidgetItem *findSuitableParentItem(int elementType) const;
    int findItemIndexForCursorPosition(int position) const;
    static QString stripMarkdown(const QString &input);
    void saveExpandState();
    void restoreExpandState();
    static QString itemKey(const QTreeWidgetItem *item);
};
