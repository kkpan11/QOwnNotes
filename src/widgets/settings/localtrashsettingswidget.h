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
#include <QtGlobal>

namespace Ui {
class LocalTrashSettingsWidget;
}

class LocalTrashSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit LocalTrashSettingsWidget(QWidget *parent = nullptr);
    ~LocalTrashSettingsWidget();

    void readSettings();
    void storeSettings();

   private slots:
    void on_localTrashEnabledCheckBox_toggled(bool checked);
    void on_localTrashClearCheckBox_toggled(bool checked);
    void on_noTrashRadioButton_toggled(bool checked);
    void on_systemTrashRadioButton_toggled(bool checked);
    void on_localTrashRadioButton_toggled(bool checked);

   private:
    Ui::LocalTrashSettingsWidget *ui;

    void updateTrashSettingsState();
};
