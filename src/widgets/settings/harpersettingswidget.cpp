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

#include "harpersettingswidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

#include "services/settingsservice.h"
#include "ui_harpersettingswidget.h"

#ifdef HARPER_ENABLED
#include "services/harperchecker.h"
#include "services/harperclient.h"
#endif

HarperSettingsWidget::HarperSettingsWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::HarperSettingsWidget) {
    ui->setupUi(this);
    setupLayoutStretch();
}

HarperSettingsWidget::~HarperSettingsWidget() { delete ui; }

void HarperSettingsWidget::setupLayoutStretch() {
    ui->gridLayout_harperOptions->setColumnStretch(0, 0);
    ui->gridLayout_harperOptions->setColumnStretch(1, 1);
    ui->gridLayout_harperOptions->setColumnStretch(2, 0);

    ui->gridLayout_harperStdio->setColumnStretch(0, 0);
    ui->gridLayout_harperStdio->setColumnStretch(1, 1);
    ui->gridLayout_harperStdio->setColumnStretch(2, 0);
    ui->gridLayout_harperStdio->setColumnStretch(3, 0);

    ui->gridLayout_harperTcp->setColumnStretch(0, 0);
    ui->gridLayout_harperTcp->setColumnStretch(1, 1);
    ui->gridLayout_harperTcp->setColumnStretch(2, 0);
    ui->gridLayout_harperTcp->setColumnStretch(3, 0);
}

void HarperSettingsWidget::initialize() {
    ui->harperTransportComboBox->clear();
    ui->harperTransportComboBox->addItem(tr("Stdio (recommended)"), 0);
    ui->harperTransportComboBox->addItem(tr("TCP"), 1);

    ui->harperDialectComboBox->clear();
    ui->harperDialectComboBox->addItem(tr("American"), QStringLiteral("American"));
    ui->harperDialectComboBox->addItem(tr("British"), QStringLiteral("British"));
    ui->harperDialectComboBox->addItem(tr("Australian"), QStringLiteral("Australian"));
    ui->harperDialectComboBox->addItem(tr("Canadian"), QStringLiteral("Canadian"));
    ui->harperDialectComboBox->addItem(tr("Indian"), QStringLiteral("Indian"));

#ifndef HARPER_ENABLED
    ui->harperEnabledCheckBox->setEnabled(false);
    setHarperOptionsEnabled(false);
    updateStatusLabel(tr("Harper support is not available in this build."));
#else
    updateTransportUi();
    updateStatusLabel();
#endif
}

void HarperSettingsWidget::readSettings() {
    SettingsService settings;

    ui->harperEnabledCheckBox->setChecked(
        settings.value(QStringLiteral("harperEnabled"), false).toBool());
    ui->harperTransportComboBox->setCurrentIndex(
        qMax(0, ui->harperTransportComboBox->findData(
                    settings.value(QStringLiteral("harperTransportMode"), 0).toInt())));
    ui->harperBinaryPathLineEdit->setText(
        settings.value(QStringLiteral("harperBinaryPath"), QStringLiteral("harper-ls")).toString());
    ui->harperTcpAddressLineEdit->setText(
        settings.value(QStringLiteral("harperTcpAddress"), QStringLiteral("127.0.0.1")).toString());
    ui->harperTcpPortSpinBox->setValue(
        settings.value(QStringLiteral("harperTcpPort"), 4000).toInt());
    ui->harperCheckDelaySpinBox->setValue(
        settings.value(QStringLiteral("harperCheckDelay"), 1500).toInt());

    const QString dialect =
        settings.value(QStringLiteral("harperDialect"), QStringLiteral("American")).toString();
    const int dialectIndex = ui->harperDialectComboBox->findData(dialect);
    ui->harperDialectComboBox->setCurrentIndex(dialectIndex >= 0 ? dialectIndex : 0);

    const QStringList enabledLinters =
        settings
            .value(QStringLiteral("harperEnabledLinters"),
                   QStringList() << QStringLiteral("SpellCheck") << QStringLiteral("AnA")
                                 << QStringLiteral("SentenceCapitalization")
                                 << QStringLiteral("RepeatedWords")
                                 << QStringLiteral("LongSentences") << QStringLiteral("Spaces")
                                 << QStringLiteral("QuoteSpacing")
                                 << QStringLiteral("NoFrenchSpaces")
                                 << QStringLiteral("WrongApostrophe")
                                 << QStringLiteral("CorrectNumberSuffix")
                                 << QStringLiteral("UnclosedQuotes"))
            .toStringList();
    ui->harperSpellCheckCheckBox->setChecked(enabledLinters.contains(QStringLiteral("SpellCheck")));
    ui->harperSentenceCapitalizationCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("SentenceCapitalization")));
    ui->harperRepeatedWordsCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("RepeatedWords")));
    ui->harperLongSentencesCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("LongSentences")));
    ui->harperAnACheckBox->setChecked(enabledLinters.contains(QStringLiteral("AnA")));
    ui->harperUnclosedQuotesCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("UnclosedQuotes")));
    ui->harperCorrectNumberSuffixCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("CorrectNumberSuffix")));
    ui->harperSpacesCheckBox->setChecked(enabledLinters.contains(QStringLiteral("Spaces")));
    ui->harperQuoteSpacingCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("QuoteSpacing")));
    ui->harperNoFrenchSpacesCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("NoFrenchSpaces")));
    ui->harperWrongApostropheCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("WrongApostrophe")));
    ui->harperSpelledNumbersCheckBox->setChecked(
        enabledLinters.contains(QStringLiteral("SpelledNumbers")));

    setHarperOptionsEnabled(ui->harperEnabledCheckBox->isChecked());
    updateTransportUi();
    updateStatusLabel();
}

void HarperSettingsWidget::storeSettings() {
    SettingsService settings;

    settings.setValue(QStringLiteral("harperEnabled"), ui->harperEnabledCheckBox->isChecked());
    settings.setValue(QStringLiteral("harperTransportMode"),
                      ui->harperTransportComboBox->currentData().toInt());
    settings.setValue(QStringLiteral("harperBinaryPath"),
                      ui->harperBinaryPathLineEdit->text().trimmed());
    settings.setValue(QStringLiteral("harperTcpAddress"),
                      ui->harperTcpAddressLineEdit->text().trimmed());
    settings.setValue(QStringLiteral("harperTcpPort"), ui->harperTcpPortSpinBox->value());
    settings.setValue(QStringLiteral("harperDialect"),
                      ui->harperDialectComboBox->currentData().toString());
    settings.setValue(QStringLiteral("harperCheckDelay"), ui->harperCheckDelaySpinBox->value());
    settings.setValue(QStringLiteral("harperEnabledLinters"), enabledLintersFromUi());
}

QStringList HarperSettingsWidget::enabledLintersFromUi() const {
    QStringList linters;
    if (ui->harperSpellCheckCheckBox->isChecked()) {
        linters.append(QStringLiteral("SpellCheck"));
    }
    if (ui->harperSentenceCapitalizationCheckBox->isChecked()) {
        linters.append(QStringLiteral("SentenceCapitalization"));
    }
    if (ui->harperRepeatedWordsCheckBox->isChecked()) {
        linters.append(QStringLiteral("RepeatedWords"));
    }
    if (ui->harperLongSentencesCheckBox->isChecked()) {
        linters.append(QStringLiteral("LongSentences"));
    }
    if (ui->harperAnACheckBox->isChecked()) {
        linters.append(QStringLiteral("AnA"));
    }
    if (ui->harperUnclosedQuotesCheckBox->isChecked()) {
        linters.append(QStringLiteral("UnclosedQuotes"));
    }
    if (ui->harperCorrectNumberSuffixCheckBox->isChecked()) {
        linters.append(QStringLiteral("CorrectNumberSuffix"));
    }
    if (ui->harperSpacesCheckBox->isChecked()) {
        linters.append(QStringLiteral("Spaces"));
    }
    if (ui->harperQuoteSpacingCheckBox->isChecked()) {
        linters.append(QStringLiteral("QuoteSpacing"));
    }
    if (ui->harperNoFrenchSpacesCheckBox->isChecked()) {
        linters.append(QStringLiteral("NoFrenchSpaces"));
    }
    if (ui->harperWrongApostropheCheckBox->isChecked()) {
        linters.append(QStringLiteral("WrongApostrophe"));
    }
    if (ui->harperSpelledNumbersCheckBox->isChecked()) {
        linters.append(QStringLiteral("SpelledNumbers"));
    }

    return linters;
}

void HarperSettingsWidget::setHarperOptionsEnabled(bool enabled) {
    ui->harperOptionsWidget->setEnabled(enabled);
}

void HarperSettingsWidget::updateTransportUi() {
    const bool stdioMode = ui->harperTransportComboBox->currentData().toInt() == 0;
    ui->harperTransportStackedWidget->setCurrentIndex(stdioMode ? 0 : 1);
    updateStatusLabel();
}

void HarperSettingsWidget::updateStatusLabel(const QString &overrideText) {
    if (!overrideText.isEmpty()) {
        ui->harperStatusLabel->setText(overrideText);
        return;
    }

    if (ui->harperTransportComboBox->currentData().toInt() == 0) {
        const QString binaryPath =
            detectHarperBinary(ui->harperBinaryPathLineEdit->text().trimmed());
        if (binaryPath.isEmpty()) {
            ui->harperStatusLabel->setText(tr("harper-ls was not found."));
            return;
        }

        const QString version = queryHarperVersion(binaryPath);
        ui->harperStatusLabel->setText(version.isEmpty()
                                           ? tr("Found Harper at %1").arg(binaryPath)
                                           : tr("Found Harper %1 at %2").arg(version, binaryPath));
        return;
    }

    ui->harperStatusLabel->setText(tr("TCP target %1:%2")
                                       .arg(ui->harperTcpAddressLineEdit->text().trimmed(),
                                            QString::number(ui->harperTcpPortSpinBox->value())));
}

QString HarperSettingsWidget::detectHarperBinary(const QString &candidate) {
    if (!candidate.trimmed().isEmpty()) {
        const QString resolved = QStandardPaths::findExecutable(candidate.trimmed());
        return resolved.isEmpty() ? candidate.trimmed() : resolved;
    }

    return QStandardPaths::findExecutable(QStringLiteral("harper-ls"));
}

QString HarperSettingsWidget::queryHarperVersion(const QString &binaryPath) {
    if (binaryPath.trimmed().isEmpty()) {
        return QString();
    }

    QProcess process;
    process.start(binaryPath, {QStringLiteral("--version")});
    if (!process.waitForStarted(1500) || !process.waitForFinished(2000)) {
        return QString();
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

void HarperSettingsWidget::on_harperEnabledCheckBox_toggled(bool checked) {
    setHarperOptionsEnabled(checked);
}

void HarperSettingsWidget::on_harperTransportComboBox_currentIndexChanged(int index) {
    Q_UNUSED(index)
    updateTransportUi();
}

void HarperSettingsWidget::on_harperBrowseButton_clicked() {
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select Harper binary"));
    if (!filePath.isEmpty()) {
        ui->harperBinaryPathLineEdit->setText(filePath);
        updateStatusLabel();
    }
}

void HarperSettingsWidget::on_harperAutoDetectButton_clicked() {
    const QString detected = detectHarperBinary();
    if (detected.isEmpty()) {
        QMessageBox::warning(this, tr("Harper"), tr("Could not find harper-ls in PATH."));
        updateStatusLabel();
        return;
    }

    ui->harperBinaryPathLineEdit->setText(detected);
    updateStatusLabel();
}

void HarperSettingsWidget::on_harperTestConnectionButton_clicked() {
#ifndef HARPER_ENABLED
    QMessageBox::warning(this, tr("Harper"), tr("Harper support is not available in this build."));
    return;
#else
    auto *client = new HarperClient(this);
    HarperClient::RequestOptions options;
    options.requestId = 1;
    options.timeoutMs = 5000;
    options.text = QStringLiteral("This are a test sentence.");
    options.dialect = ui->harperDialectComboBox->currentData().toString();
    options.enabledLinters = enabledLintersFromUi();
    options.transportMode = ui->harperTransportComboBox->currentData().toInt();
    options.binaryPath = ui->harperBinaryPathLineEdit->text().trimmed();
    options.tcpAddress = ui->harperTcpAddressLineEdit->text().trimmed();
    options.tcpPort = ui->harperTcpPortSpinBox->value();

    connect(client, &HarperClient::checkFinished, this,
            [this, client](int, const QVector<HarperMatch> &) {
                QString status = tr("Harper connection successful.");
                const QString version = client->serverVersion();
                if (!version.isEmpty()) {
                    status += QLatin1Char(' ') + version;
                }
                updateStatusLabel(status);
                QMessageBox::information(this, tr("Harper"), status);
                client->deleteLater();
            });
    connect(client, &HarperClient::checkError, this, [this, client](int, const QString &message) {
        updateStatusLabel(message);
        QMessageBox::warning(this, tr("Harper"), tr("Harper connection failed: %1").arg(message));
        client->deleteLater();
    });

    client->checkText(options);
#endif
}

void HarperSettingsWidget::on_harperResetIgnoredRulesButton_clicked() {
#ifdef HARPER_ENABLED
    auto *checker = HarperChecker::instance();
    if (checker == nullptr) {
        return;
    }

    const int count = checker->ignoredRules().size();
    if (count == 0) {
        QMessageBox::information(this, tr("Harper"), tr("There are no ignored rules to reset."));
        return;
    }

    if (QMessageBox::question(this, tr("Harper"), tr("Reset %n ignored rule(s)?", "", count),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        checker->clearIgnoredRules();
        QMessageBox::information(this, tr("Harper"), tr("All ignored rules have been reset."));
    }
#endif
}

void HarperSettingsWidget::on_harperResetIgnoredWordsButton_clicked() {
#ifdef HARPER_ENABLED
    auto *checker = HarperChecker::instance();
    if (checker == nullptr) {
        return;
    }

    const int count = checker->ignoredWords().size();
    if (count == 0) {
        QMessageBox::information(this, tr("Harper"), tr("There are no ignored words to reset."));
        return;
    }

    if (QMessageBox::question(this, tr("Harper"), tr("Reset %n ignored word(s)?", "", count),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        checker->clearIgnoredWords();
        QMessageBox::information(this, tr("Harper"), tr("All ignored words have been reset."));
    }
#endif
}
