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
class DebugOptionSettingsWidget;
}

class DebugOptionSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit DebugOptionSettingsWidget(QWidget *parent = nullptr);
    ~DebugOptionSettingsWidget();

    void initialize();
    void readSettings();
    void storeSettings();

    static void removeLogFile();

   private slots:
    void on_fileLoggingCheckBox_toggled(bool checked);
    void on_clearLogFileButton_clicked();

   private:
    Ui::DebugOptionSettingsWidget *ui;
};
