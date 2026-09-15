#pragma once

#ifdef HARPER_ENABLED

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>

#include "services/harpertypes.h"

class HarperTransport;
class QTimer;

class HarperClient : public QObject {
    Q_OBJECT

   public:
    enum TransportMode { StdioTransport = 0, TcpTransport = 1 };

    struct RequestOptions {
        int requestId = 0;
        int timeoutMs = 5000;
        QString text;
        QString dialect;
        QStringList enabledLinters;
        int transportMode = StdioTransport;
        QString binaryPath;
        QString tcpAddress;
        int tcpPort = 4000;
    };

    explicit HarperClient(QObject *parent = nullptr);
    ~HarperClient() override;

    int checkText(const RequestOptions &options);
    void cancelRequest(int requestId);
    void cancelAllRequests();
    QString serverName() const;
    QString serverVersion() const;

   Q_SIGNALS:
    void checkFinished(int requestId, QVector<HarperMatch> matches);
    void checkError(int requestId, QString errorMessage);

   private:
    struct PendingRequestState {
        int requestId = 0;
        QString uri;
        QString text;
        QVector<HarperMatch> matches;
        QVector<QJsonObject> diagnostics;
        QHash<int, int> codeActionRequestToMatchIndex;
    };

    void ensureTransport(const RequestOptions &options);
    bool ensureStarted(const RequestOptions &options, QString *errorMessage);
    void resetProtocolState();
    void closeDocument(const QString &uri);
    void startInitialization();
    void sendMessage(const QJsonObject &object);
    void parseMessages();
    void handleRequest(const QJsonObject &object);
    void handleResponse(const QJsonObject &object);
    void handleNotification(const QJsonObject &object);
    void sendInitializedNotification();
    void sendConfiguration(const RequestOptions &options);
    void sendResult(const QJsonValue &requestId, const QJsonValue &result);
    void sendError(const QJsonValue &requestId, int code, const QString &message);
    void openPendingDocument();
    int requestCodeAction(const QString &uri, const QJsonObject &diagnosticObject);
    void finalizePendingRequestIfReady();
    void failPendingRequest(const QString &errorMessage, int requestId = 0);
    void clearPendingRequest();
    void updateServerInfo(const QJsonObject &result);
    QJsonObject buildSettingsObject(const RequestOptions &options) const;

    static QVector<int> buildLineOffsets(const QString &text);
    static int absoluteOffsetForPosition(const QVector<int> &lineOffsets, int line, int character);
    static QString diagnosticCodeToString(const QJsonValue &value);
    static QString categoryForRuleId(const QString &ruleId);
    static QString contextTextForMatch(const QString &text, int offset, int length);
    static QStringList replacementsFromCodeActions(const QJsonValue &value, const QString &uri,
                                                   const HarperMatch &match);

    QByteArray _readBuffer;
    std::unique_ptr<HarperTransport> _transport;
    QTimer *_timeoutTimer = nullptr;
    RequestOptions _currentOptions;
    bool _hasCurrentOptions = false;
    bool _initialized = false;
    int _nextLspRequestId = 1;
    int _initializeRequestId = -1;
    PendingRequestState _pendingRequest;
    QString _serverName;
    QString _serverVersion;
    qint64 _nextTcpReconnectAttemptAtMs = 0;
};

#endif
