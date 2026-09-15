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

#include "gitsettingswidget.h"

#include <services/settingsservice.h>
#include <utils/misc.h>

#include <QFileDialog>

#include "dialogs/filedialog.h"
#include "ui_gitsettingswidget.h"

GitSettingsWidget::GitSettingsWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::GitSettingsWidget) {
    ui->setupUi(this);
}

GitSettingsWidget::~GitSettingsWidget() { delete ui; }

void GitSettingsWidget::initialize() {
#ifdef Q_OS_WIN32
    QString downloadText =
        tr("You can download your git client here: <a "
           "href=\"%url\">Git for Windows</a>");
    downloadText.replace("%url", "https://git-scm.com/download/win");
    ui->gitDownloadLabel->setText(downloadText);
#else
    ui->gitDownloadLabel->hide();
#endif

#ifdef USE_LIBGIT2
    ui->gitClientGroupBox->hide();
#else
    ui->gitLibraryGroupBox->hide();
#endif
}

void GitSettingsWidget::readSettings() {
    SettingsService settings;

    // Load git settings
    ui->gitPathLineEdit->setText(Utils::Misc::prependPortableDataPathIfNeeded(
        settings.value(QStringLiteral("gitExecutablePath")).toString(), true));
    ui->gitCommitIntervalTime->setValue(
        settings.value(QStringLiteral("gitCommitInterval"), 30).toInt());
    ui->gitLogCommandLineEdit->setText(settings.value(QStringLiteral("gitLogCommand")).toString());
}

void GitSettingsWidget::storeSettings() {
    SettingsService settings;

    // Store git settings
    settings.setValue(
        QStringLiteral("gitExecutablePath"),
        Utils::Misc::makePathRelativeToPortableDataPathIfNeeded(ui->gitPathLineEdit->text()));
    settings.setValue(QStringLiteral("gitCommitInterval"), ui->gitCommitIntervalTime->value());
    settings.setValue(QStringLiteral("gitLogCommand"), ui->gitLogCommandLineEdit->text());
}

void GitSettingsWidget::on_setGitPathToolButton_clicked() {
    QString path = ui->gitPathLineEdit->text();
    if (path.isEmpty()) {
#ifdef Q_OS_WIN
        path = "git.exe";
#else
        path = QLatin1String("/usr/bin/git");
#endif
    }

#ifdef Q_OS_WIN
    QStringList filters = QStringList()
                          << tr("Executable files") + " (*.exe)" << tr("All files") + " (*)";
#else
    QStringList filters = QStringList() << tr("All files") + " (*)";
#endif

    FileDialog dialog(QStringLiteral("GitExecutable"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilters(filters);
    dialog.selectFile(path);
    dialog.setWindowTitle(tr("Please select the path of your git executable"));
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        path = dialog.selectedFile();

        if (!path.isEmpty()) {
            ui->gitPathLineEdit->setText(path);
        }
    }
}
