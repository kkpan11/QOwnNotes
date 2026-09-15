#pragma once

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVector>

struct HarperMatch {
    int offset = 0;
    int length = 0;
    QString message;
    QString shortMessage;
    QString ruleId;
    QString ruleCategory;
    QStringList replacements;
    QString contextText;

    bool containsPosition(int position) const {
        return (position >= offset) && (position < (offset + length));
    }
};

Q_DECLARE_METATYPE(HarperMatch)
Q_DECLARE_METATYPE(QVector<HarperMatch>)
