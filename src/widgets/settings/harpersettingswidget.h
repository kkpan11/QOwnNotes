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

#include <QString>
#include <QStringList>
#include <QWidget>

namespace Ui {
class HarperSettingsWidget;
}

class HarperSettingsWidget : public QWidget {
    Q_OBJECT

   public:
    explicit HarperSettingsWidget(QWidget *parent = nullptr);
    ~HarperSettingsWidget();

    void initialize();
    void readSettings();
    void storeSettings();

   private slots:
    void on_harperEnabledCheckBox_toggled(bool checked);
    void on_harperTransportComboBox_currentIndexChanged(int index);
    void on_harperBrowseButton_clicked();
    void on_harperAutoDetectButton_clicked();
    void on_harperTestConnectionButton_clicked();
    void on_harperResetIgnoredRulesButton_clicked();
    void on_harperResetIgnoredWordsButton_clicked();

   private:
    Ui::HarperSettingsWidget *ui;

    QStringList enabledLintersFromUi() const;
    void setupLayoutStretch();
    void setHarperOptionsEnabled(bool enabled);
    void updateTransportUi();
    void updateStatusLabel(const QString &overrideText = QString());
    static QString detectHarperBinary(const QString &candidate = QString());
    static QString queryHarperVersion(const QString &binaryPath);
};
