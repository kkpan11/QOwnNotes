#ifndef LAYOUTPRESETWIDGET_H
#define LAYOUTPRESETWIDGET_H

#include <QWidget>

namespace Ui {
class LayoutPresetWidget;
}

class QSettings;

class LayoutPresetWidget : public QWidget {
    Q_OBJECT

   Q_SIGNALS:
    void layoutStored(const QString &layoutUuid);

   public:
    explicit LayoutPresetWidget(QWidget *parent = 0);
    ~LayoutPresetWidget();

    void resizeLayoutPresetImage() const;

    void setManualSettingsStoring(bool enabled);

    void storeLayoutPreset();

   private slots:
    void on_layoutPresetComboBox_currentIndexChanged(int index);

    void resizeEvent(QResizeEvent *event);

    void on_useLayoutPresetPushButton_clicked();

   private:
    Ui::LayoutPresetWidget *ui;

    QSettings *_layoutPresetSettings;

    bool _manualSettingsStoring;

    void loadLayoutPresets();

    void updateCurrentLayoutPreset();

    static QString getLayoutPresetName(const QString &layoutPresetIdentifier);

    static QString getLayoutPresetDescription(const QString &layoutPresetIdentifier);
};

#endif    // LAYOUTPRESETWIDGET_H
