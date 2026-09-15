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
 * QPlainTextEdit Markdown highlighter
 */

#pragma once

#include <entities/note.h>
#include <libraries/qmarkdowntextedit/markdownhighlighter.h>
#include <services/markdownlspclient.h>

#include <QHash>

QT_BEGIN_NAMESPACE
class QTextDocument;

QT_END_NAMESPACE

namespace Sonnet {
class WordTokenizer;
class LanguageFilter;
}    // namespace Sonnet
class QOwnSpellChecker;

class QOwnNotesMarkdownHighlighter : public MarkdownHighlighter {
    Q_OBJECT

   public:
    QOwnNotesMarkdownHighlighter(
        QTextDocument *parent = nullptr,
        HighlightingOptions highlightingOptions = HighlightingOption::None);

    void updateCurrentNote(Note *note);

    void setMarkdownLspDiagnostics(const QVector<MarkdownLspClient::Diagnostic> &diagnostics);
    void clearMarkdownLspDiagnostics();

    struct ScriptingHighlightingRule {
        explicit ScriptingHighlightingRule(const HighlighterState state_) : state(state_) {}
        ScriptingHighlightingRule() = default;

        QRegularExpression pattern;
        QString shouldContain;
        HighlighterState state = NoState;
        uint8_t capturingGroup = 0;
        uint8_t maskedGroup = 0;

        // Custom format fields for script-defined colors and styles
        bool hasCustomFormat = false;
        QString foregroundColor;
        QString backgroundColor;
        bool bold = false;
        bool italic = false;
        bool underline = false;
        qreal fontSize = 0;
    };

   protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

   private:
    void highlightBrokenNotesLink(const QString &text);
    void highlightWikiLinks(const QString &text);
    void clearWikiLinkCache();

    void highlightMarkdownLsp(const QString &text);
    void setMarkdownLspUnderline(int start, int count, const QColor &color, const QString &toolTip);

    // Set the format of a word as misspelled i.e., red wavy underline
    void setMisspelled(const int start, const int count);
    void highlightSpellChecking(const QString &text);
#ifdef LANGUAGETOOL_ENABLED
    void highlightLanguageTool(const QString &text);
    void setLanguageToolUnderline(int start, int count, const QColor &color,
                                  const QString &toolTip);
#endif
#ifdef HARPER_ENABLED
    void highlightHarper(const QString &text);
    void setHarperUnderline(int start, int count, const QColor &color, const QString &toolTip);
#endif

    void updateCachedRegexes(const QString &newExt);

   private:
    Note *_currentNote = nullptr;
    bool _hasEncrypted = false;
    bool _highlightEncrypted = false;

    QString _defaultNoteFileExt;
    QRegularExpression _regexTagStyleLink;
    QRegularExpression _regexBracketLink;
    QHash<QString, bool> _wikiLinkCache;

    // Cache of LSP diagnostics keyed by block (line) number.
    // Each entry holds one or more diagnostics that touch that block.
    struct LspBlockDiagnostic {
        int startCharacter = 0;
        int endCharacter = 0;
        QColor color;
        QString toolTip;
    };
    QHash<int, QVector<LspBlockDiagnostic>> _lspDiagnosticsCache;
    void highlightScriptingRules(const QVector<ScriptingHighlightingRule> &rules,
                                 const QString &text);
    void highlightScriptingHook(const QString &text);
};
