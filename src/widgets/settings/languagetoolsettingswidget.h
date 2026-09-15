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
class LanguageToolSettingsWidget;
}

class LanguageToolSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit LanguageToolSettingsWidget(QWidget *parent = nullptr);
    ~LanguageToolSettingsWidget();

    void initialize();
    void readSettings();
    void storeSettings();

   private slots:
    void on_languageToolEnabledCheckBox_toggled(bool checked);
    void on_languageToolTestConnectionButton_clicked();
    void on_languageToolResetIgnoredRulesButton_clicked();
    void on_languageToolResetIgnoredWordsButton_clicked();

   private:
    Ui::LanguageToolSettingsWidget *ui;
    QStringList languageToolEnabledCategoriesFromUi() const;
    void setLanguageToolOptionsEnabled(bool enabled);
};
