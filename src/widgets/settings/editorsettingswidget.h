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

#pragma once

#include <QWidget>

namespace Ui {
class EditorSettingsWidget;
}

class EditorSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit EditorSettingsWidget(QWidget *parent = nullptr);
    ~EditorSettingsWidget();

    void initialize();
    void readSettings();
    void storeSettings();

   signals:
    void needRestart();
    void wikiLinkSupportToggled(bool checked);

   private slots:
    void on_cursorWidthResetButton_clicked();
    void on_markdownHighlightingCheckBox_toggled(bool checked);
    void on_enableWikiLinkSupportCheckBox_toggled(bool checked);
    void on_showLineNumbersInEditorCheckBox_toggled(bool checked);
    void on_editorWidthInDFMOnlyCheckBox_toggled(bool checked);

   private:
    Ui::EditorSettingsWidget *ui;
};
