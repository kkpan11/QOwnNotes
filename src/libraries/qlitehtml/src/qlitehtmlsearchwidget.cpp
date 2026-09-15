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
 *
 */

#include "qlitehtmlsearchwidget.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBoundaryFinder>

#include "qlitehtmlwidget.h"
#include "ui_qlitehtmlsearchwidget.h"

namespace {
bool isEmojiCodePoint(unsigned int codePoint)
{
    return codePoint == 0x20E3 || codePoint == 0x00A9 || codePoint == 0x00AE || codePoint == 0x203C
           || codePoint == 0x2049 || codePoint == 0x2122 || codePoint == 0x2139
           || (codePoint >= 0x2194 && codePoint <= 0x21AA)
           || (codePoint >= 0x231A && codePoint <= 0x2328) || codePoint == 0x23CF
           || (codePoint >= 0x23E9 && codePoint <= 0x23FA) || codePoint == 0x24C2
           || (codePoint >= 0x25AA && codePoint <= 0x25AB) || codePoint == 0x25B6
           || codePoint == 0x25C0 || (codePoint >= 0x25FB && codePoint <= 0x25FE)
           || (codePoint >= 0x2600 && codePoint <= 0x27BF)
           || (codePoint >= 0x2934 && codePoint <= 0x2935)
           || (codePoint >= 0x2B05 && codePoint <= 0x2B55) || codePoint == 0x3030
           || codePoint == 0x303D || codePoint == 0x3297 || codePoint == 0x3299
           || (codePoint >= 0x1F000 && codePoint <= 0x1FAFF);
}

int graphemeCount(const QString &text, int maxCount)
{
    if (text.isEmpty()) {
        return 0;
    }

    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    finder.toStart();

    int count = 0;
    while (finder.toNextBoundary() != -1) {
        ++count;

        if (count >= maxCount) {
            break;
        }
    }

    return count;
}

bool shouldStartSearch(const QString &text)
{
    const int minimumSearchLength = 2;
    const int count = graphemeCount(text, minimumSearchLength);
    if (count >= minimumSearchLength) {
        return true;
    }

    if (count != 1) {
        return false;
    }

    const auto codePoints = text.toUcs4();
    for (unsigned int codePoint : codePoints) {
        if (isEmojiCodePoint(codePoint)) {
            return true;
        }
    }

    return false;
}
} // namespace

QLiteHtmlSearchWidget::QLiteHtmlSearchWidget(QLiteHtmlWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QLiteHtmlSearchWidget)
{
    ui->setupUi(this);
    _liteHtmlWidget = parent;
    hide();

    QObject::connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(deactivate()));
    QObject::connect(ui->searchLineEdit,
                     SIGNAL(textChanged(QString)),
                     this,
                     SLOT(searchLineEditTextChanged(QString)));
    QObject::connect(ui->searchDownButton, SIGNAL(clicked()), this, SLOT(doSearchDown()));
    QObject::connect(ui->searchUpButton, SIGNAL(clicked()), this, SLOT(doSearchUp()));

    // Set up debounce timer so the search is delayed while the user is still
    // typing
    _debounceTimer.setSingleShot(true);
    _debounceTimer.setInterval(300);
    QObject::connect(&_debounceTimer, &QTimer::timeout, this, &QLiteHtmlSearchWidget::doSearchDown);

    installEventFilter(this);
    ui->searchLineEdit->installEventFilter(this);

#ifdef Q_OS_MAC
    layout()->setSpacing(8);
    QString buttonStyle = "QPushButton {margin: 0}";
    ui->closeButton->setStyleSheet(buttonStyle);
    ui->searchDownButton->setStyleSheet(buttonStyle);
    ui->searchUpButton->setStyleSheet(buttonStyle);
    ui->matchCaseSensitiveButton->setStyleSheet(buttonStyle);
#endif
}

QLiteHtmlSearchWidget::~QLiteHtmlSearchWidget()
{
    delete ui;
}

void QLiteHtmlSearchWidget::activate()
{
    show();

    const int verticalScrollBarValue = _liteHtmlWidget->verticalScrollBar()->value();
    const int horizontalScrollBarValue = _liteHtmlWidget->horizontalScrollBar()->value();

    QString selectedText = _liteHtmlWidget->selectedText();
    // Preset the selected text as search text if there is any, replacing any
    // existing search text. Set _searchFromSelection BEFORE calling setText()
    // so the textChanged signal triggers doSearch() with incremental=true,
    // which makes the litehtml engine start from the beginning of the current
    // selection instead of after it — keeping the first result on the
    // originally selected word (for #3538).
    bool searchAlreadyDone = false;
    if (!selectedText.isEmpty()) {
        _searchFromSelection = true;
        ui->searchLineEdit->setText(selectedText);
        // If the text actually changed, textChanged fired synchronously:
        // searchLineEditTextChanged -> doSearch() already ran with incremental=true
        // and found the correct occurrence. _searchFromSelection was consumed
        // (set to false) inside doSearch(). Skip the doSearchDown() below to
        // avoid advancing to the next occurrence.
        // If the text did NOT change (same search term as before), _searchFromSelection
        // is still true; we fall through so doSearchDown() uses it.
        searchAlreadyDone = !_searchFromSelection;
    }

    ui->searchLineEdit->setFocus();
    ui->searchLineEdit->selectAll();

    if (!searchAlreadyDone) {
        doSearchDown();
    }

    _liteHtmlWidget->verticalScrollBar()->setValue(verticalScrollBarValue);
    _liteHtmlWidget->horizontalScrollBar()->setValue(horizontalScrollBarValue);
}

void QLiteHtmlSearchWidget::deactivate()
{
    hide();
    _liteHtmlWidget->setFocus();
}

bool QLiteHtmlSearchWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            deactivate();
            return true;
        } else if ((keyEvent->modifiers().testFlag(Qt::ShiftModifier)
                    && (keyEvent->key() == Qt::Key_Return))
                   || (keyEvent->key() == Qt::Key_Up)) {
            doSearchUp();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() == Qt::Key_Down)) {
            doSearchDown();
            return true;
        } else if (keyEvent->key() == Qt::Key_F3) {
            doSearch(!keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            return true;
        }

        return false;
    }

    return QWidget::eventFilter(obj, event);
}

void QLiteHtmlSearchWidget::searchLineEditTextChanged(const QString &arg1)
{
    // If the search term is too short, just clear the style without jumping to
    // the top of the document
    if (!shouldStartSearch(arg1)) {
        _debounceTimer.stop();
        ui->searchLineEdit->setStyleSheet(QString());
        return;
    }

    // Debounce: delay the search while the user is still typing
    _debounceTimer.start();
}

void QLiteHtmlSearchWidget::doSearchUp()
{
    doSearch(false);
}

void QLiteHtmlSearchWidget::doSearchDown()
{
    doSearch(true);
}

/**
 * @brief Searches for text in the LiteHtml widget
 * @returns true if found
 */
bool QLiteHtmlSearchWidget::doSearch(bool searchDown, bool allowRestartAtTop)
{
    QString text = ui->searchLineEdit->text();

    if (!shouldStartSearch(text)) {
        ui->searchLineEdit->setStyleSheet(QString());
        return false;
    }

    int searchMode = ui->modeComboBox->currentIndex();

    QFlags<QTextDocument::FindFlag> options = searchDown ? QTextDocument::FindFlag(0)
                                                         : QTextDocument::FindBackward;
    if (searchMode == WholeWordsMode) {
        options |= QTextDocument::FindWholeWords;
    }

    if (ui->matchCaseSensitiveButton->isChecked()) {
        options |= QTextDocument::FindCaseSensitively;
    }

    bool wrapped = false;
    // Use incremental=true when searching from a preset selection so that the
    // search starts at the beginning of the selection, keeping the first result
    // at the originally selected word (for #3541)
    const bool incremental = _searchFromSelection;
    _searchFromSelection = false;
    bool found = _liteHtmlWidget->findText(text, options, incremental, &wrapped);

    if (!found && allowRestartAtTop) {
        _liteHtmlWidget->findText(text, options, false, &wrapped);
    }

    // Add background and foreground colors according to whether we found the text or not
    const QString bgColorCode = _darkMode ? (found ? QStringLiteral("#135a13")
                                                   : QStringLiteral("#8d2b36"))
                                : found   ? QStringLiteral("#D5FAE2")
                                          : QStringLiteral("#FAE9EB");
    const QString fgColorCode = _darkMode ? QStringLiteral("#cccccc") : QStringLiteral("#404040");

    ui->searchLineEdit->setStyleSheet(QStringLiteral("* { background: ") + bgColorCode
                                      + QStringLiteral("; color: ") + fgColorCode
                                      + QStringLiteral("; }"));

    return found;
}

void QLiteHtmlSearchWidget::setDarkMode(bool enabled)
{
    _darkMode = enabled;

    // Apply dark mode styling to buttons to ensure good contrast
    if (_darkMode) {
        // Set button icon colors to light color for visibility in dark mode
        QString buttonStyle = QStringLiteral("QPushButton { color: #cccccc; }");
        ui->matchCaseSensitiveButton->setStyleSheet(buttonStyle);
    } else {
        // Reset to default styling in light mode
        ui->matchCaseSensitiveButton->setStyleSheet(QString());
    }
}
