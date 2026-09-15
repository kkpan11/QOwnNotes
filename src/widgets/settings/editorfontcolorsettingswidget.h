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

#include <QFont>
#include <QWidget>

class QLineEdit;

namespace Ui {
class EditorFontColorSettingsWidget;
}

class EditorFontColorSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit EditorFontColorSettingsWidget(QWidget *parent = nullptr);
    ~EditorFontColorSettingsWidget();

    void readSettings();
    void storeSettings();
    void setWikiLinkItemsVisible(bool visible);

   signals:
    void needRestart();

   private slots:
    void on_noteTextEditButton_clicked();
    void on_noteTextEditCodeButton_clicked();
    void on_noteTextEditResetButton_clicked();
    void on_noteTextEditCodeResetButton_clicked();

   private:
    Ui::EditorFontColorSettingsWidget *ui;
    QFont noteTextEditFont;
    QFont noteTextEditCodeFont;

    void setFontLabel(QLineEdit *label, const QFont &font);
    void storeFontSettings();
};
