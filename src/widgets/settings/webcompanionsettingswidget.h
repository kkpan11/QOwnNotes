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
class WebCompanionSettingsWidget;
}

class WebCompanionSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit WebCompanionSettingsWidget(QWidget *parent = nullptr);
    ~WebCompanionSettingsWidget();

    void initialize();
    void readSettings();
    void storeSettings();

   signals:
    void needRestart();

   private slots:
    void on_webSocketServerServicePortResetButton_clicked();
    void on_enableSocketServerCheckBox_toggled();
    void on_bookmarkSuggestionApiEnabledCheckBox_toggled(bool checked);
    void on_bookmarkSuggestionApiPortResetButton_clicked();
    void on_bookmarkSuggestionApiShowTokenButton_clicked();
    void on_bookmarkSuggestionApiCopyTokenButton_clicked();
    void on_bookmarkSuggestionApiGenerateTokenButton_clicked();
    void on_webSocketTokenButton_clicked();

   private:
    Ui::WebCompanionSettingsWidget *ui;
    QString _bookmarkTagLabelHtml;
    QString _commandSnippetTagLabelHtml;
};
