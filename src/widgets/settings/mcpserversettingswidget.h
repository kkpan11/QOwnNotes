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
class McpServerSettingsWidget;
}

class McpServerSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit McpServerSettingsWidget(QWidget *parent = nullptr);
    ~McpServerSettingsWidget();

    void initialize();
    void readSettings();
    void storeSettings();

   signals:
    void needRestart();

   private slots:
    void on_mcpServerEnabledCheckBox_toggled(bool checked);
    void on_mcpServerPortResetButton_clicked();
    void on_mcpServerShowTokenButton_clicked();
    void on_mcpServerCopyTokenButton_clicked();
    void on_mcpServerGenerateTokenButton_clicked();

   private:
    Ui::McpServerSettingsWidget *ui;
};
