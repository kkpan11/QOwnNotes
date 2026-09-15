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

#include "localtrashsettingswidget.h"

#include <entities/trashitem.h>
#include <services/settingsservice.h>

#include "ui_localtrashsettingswidget.h"

LocalTrashSettingsWidget::LocalTrashSettingsWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::LocalTrashSettingsWidget) {
    ui->setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    ui->legacyTrashSupportGroupBox->setVisible(false);
#else
    ui->trashModeGroupBox->setVisible(false);
#endif

    updateTrashSettingsState();
}

LocalTrashSettingsWidget::~LocalTrashSettingsWidget() { delete ui; }

void LocalTrashSettingsWidget::readSettings() {
    SettingsService settings;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    switch (TrashItem::trashMode()) {
        case TrashItem::TrashMode::NoTrashing:
            ui->noTrashRadioButton->setChecked(true);
            break;
        case TrashItem::TrashMode::SystemTrash:
            ui->systemTrashRadioButton->setChecked(true);
            break;
        case TrashItem::TrashMode::LocalTrash:
            ui->localTrashRadioButton->setChecked(true);
            break;
        default:
            ui->localTrashRadioButton->setChecked(true);
            break;
    }
#else
    ui->localTrashEnabledCheckBox->setChecked(
        settings.value(QStringLiteral("localTrash/supportEnabled"), true).toBool());
#endif

    ui->localTrashClearCheckBox->setChecked(
        settings.value(QStringLiteral("localTrash/autoCleanupEnabled"), true).toBool());
    ui->localTrashClearTimeSpinBox->setValue(
        settings.value(QStringLiteral("localTrash/autoCleanupDays"), 30).toInt());

    updateTrashSettingsState();
}

void LocalTrashSettingsWidget::storeSettings() {
    SettingsService settings;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    TrashItem::TrashMode mode = TrashItem::TrashMode::NoTrashing;

    if (ui->systemTrashRadioButton->isChecked()) {
        mode = TrashItem::TrashMode::SystemTrash;
    } else if (ui->localTrashRadioButton->isChecked()) {
        mode = TrashItem::TrashMode::LocalTrash;
    }

    settings.setValue(QStringLiteral("localTrash/mode"), static_cast<int>(mode));
    settings.setValue(QStringLiteral("localTrash/supportEnabled"),
                      mode == TrashItem::TrashMode::LocalTrash);
#else
    settings.setValue(QStringLiteral("localTrash/supportEnabled"),
                      ui->localTrashEnabledCheckBox->isChecked());
#endif

    settings.setValue(QStringLiteral("localTrash/autoCleanupEnabled"),
                      ui->localTrashClearCheckBox->isChecked());
    settings.setValue(QStringLiteral("localTrash/autoCleanupDays"),
                      ui->localTrashClearTimeSpinBox->value());
}

void LocalTrashSettingsWidget::on_localTrashEnabledCheckBox_toggled(bool checked) {
    Q_UNUSED(checked)
    updateTrashSettingsState();
}

void LocalTrashSettingsWidget::on_localTrashClearCheckBox_toggled(bool checked) {
    Q_UNUSED(checked)
    updateTrashSettingsState();
}

void LocalTrashSettingsWidget::on_noTrashRadioButton_toggled(bool checked) {
    Q_UNUSED(checked)
    updateTrashSettingsState();
}

void LocalTrashSettingsWidget::on_systemTrashRadioButton_toggled(bool checked) {
    Q_UNUSED(checked)
    updateTrashSettingsState();
}

void LocalTrashSettingsWidget::on_localTrashRadioButton_toggled(bool checked) {
    Q_UNUSED(checked)
    updateTrashSettingsState();
}

void LocalTrashSettingsWidget::updateTrashSettingsState() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const bool localTrashEnabled = ui->localTrashRadioButton->isChecked();
#else
    const bool localTrashEnabled = ui->localTrashEnabledCheckBox->isChecked();
#endif

    ui->localTrashGroupBox->setEnabled(localTrashEnabled);
    ui->localTrashClearCheckBox->setEnabled(localTrashEnabled);
    ui->localTrashClearFrame->setEnabled(localTrashEnabled &&
                                         ui->localTrashClearCheckBox->isChecked());
}
