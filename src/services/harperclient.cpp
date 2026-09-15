#ifdef HARPER_ENABLED

#include "services/harperclient.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <functional>

class HarperTransport {
   public:
    virtual ~HarperTransport() = default;

    std::function<void(const QByteArray &data)> dataReceived;
    std::function<void(const QString &message)> errorOccurred;
    std::function<void()> started;
    std::function<void()> stopped;

    virtual bool start(QString *errorMessage) = 0;
    virtual void stop() = 0;
    virtual void write(const QByteArray &data) = 0;
    virtual bool isRunning() const = 0;
};

namespace {

class HarperStdioTransport : public HarperTransport {
   public:
    explicit HarperStdioTransport(QString command) : _command(std::move(command)) {
        static const QRegularExpression errorPattern(
            QStringLiteral("\\bERR\\b|\\bERROR\\b|\\bFATAL\\b"),
            QRegularExpression::CaseInsensitiveOption);

        QObject::connect(&_process, &QProcess::readyReadStandardOutput, [&]() {
            if (dataReceived) {
                dataReceived(_process.readAllStandardOutput());
            }
        });
        QObject::connect(&_process, &QProcess::readyReadStandardError, [&]() {
            const QString errorText = QString::fromUtf8(_process.readAllStandardError());
            if (errorText.trimmed().isEmpty() || !errorOccurred) {
                return;
            }

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
            const QStringList lines = errorText.split(QLatin1Char('\n'), QString::SkipEmptyParts);
#else
            const QStringList lines = errorText.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
#endif
            for (const QString &line : lines) {
                const QString trimmed = line.trimmed();
                if (trimmed.isEmpty()) {
                    continue;
                }

                if (errorPattern.match(trimmed).hasMatch()) {
                    errorOccurred(trimmed);
                }
            }
        });
        QObject::connect(&_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [&](int, QProcess::ExitStatus) {
                             if (stopped) {
                                 stopped();
                             }
                         });
    }

    bool start(QString *errorMessage) override {
        if (_process.state() != QProcess::NotRunning) {
            return true;
        }

        if (_command.trimmed().isEmpty()) {
            if (errorMessage != nullptr) {
                *errorMessage = QObject::tr("Harper command is empty");
            }
            return false;
        }

        _process.start(_command, {QStringLiteral("--stdio")}, QIODevice::ReadWrite);
        if (!_process.waitForStarted(2000)) {
            if (errorMessage != nullptr) {
                *errorMessage =
                    QObject::tr("Failed to start Harper command: %1").arg(_command.trimmed());
            }
            return false;
        }

        if (started) {
            started();
        }

        return true;
    }

    void stop() override {
        if (_process.state() == QProcess::NotRunning) {
            return;
        }

        _process.terminate();
        _process.waitForFinished(1500);
        if (_process.state() != QProcess::NotRunning) {
            _process.kill();
            _process.waitForFinished(1000);
        }
    }

    void write(const QByteArray &data) override { _process.write(data); }

    bool isRunning() const override { return _process.state() != QProcess::NotRunning; }

   private:
    QProcess _process;
    QString _command;
};

class HarperTcpTransport : public HarperTransport {
   public:
    HarperTcpTransport(QString address, int port)
        : _address(std::move(address)), _port(static_cast<quint16>(port)) {
        QObject::connect(&_socket, &QTcpSocket::readyRead, [&]() {
            if (dataReceived) {
                dataReceived(_socket.readAll());
            }
        });
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
        QObject::connect(&_socket,
                         QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
#else
        QObject::connect(
            &_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
#endif
                         [&](QAbstractSocket::SocketError) {
                             if (errorOccurred) {
                                 errorOccurred(_socket.errorString());
                             }
                         });
        QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
            if (stopped) {
                stopped();
            }
        });
    }

    bool start(QString *errorMessage) override {
        if (_socket.state() == QAbstractSocket::ConnectedState) {
            return true;
        }

        _socket.abort();
        _socket.connectToHost(_address, _port);
        if (!_socket.waitForConnected(2000)) {
            if (errorMessage != nullptr) {
                *errorMessage = QObject::tr("Failed to connect to Harper at %1:%2")
                                    .arg(_address)
                                    .arg(QString::number(_port));
            }
            return false;
        }

        if (started) {
            started();
        }

        return true;
    }

    void stop() override {
        if (_socket.state() == QAbstractSocket::UnconnectedState) {
            return;
        }

        _socket.disconnectFromHost();
        if (_socket.state() != QAbstractSocket::UnconnectedState) {
            _socket.waitForDisconnected(1000);
        }
    }

    void write(const QByteArray &data) override { _socket.write(data); }

    bool isRunning() const override { return _socket.state() == QAbstractSocket::ConnectedState; }

   private:
    QTcpSocket _socket;
    QString _address;
    quint16 _port = 0;
};

QString decodeReplacementTitle(const QString &title) {
    static const QRegularExpression replacementPattern(
        QStringLiteral(R"__regex__(^\s*Replace with:\s*"(.*)"\.?\s*$)__regex__"));
    const QRegularExpressionMatch match = replacementPattern.match(title);
    if (!match.hasMatch()) {
        return QString();
    }

    return match.captured(1);
}

QVector<QJsonObject> parseWorkspaceEditEntries(const QJsonObject &workspaceEdit,
                                               const QString &uri) {
    QVector<QJsonObject> edits;
    if (workspaceEdit.isEmpty() || uri.isEmpty()) {
        return edits;
    }

    const QJsonObject changes = workspaceEdit.value(QStringLiteral("changes")).toObject();
    const QJsonArray changedEdits = changes.value(uri).toArray();
    edits.reserve(changedEdits.size());
    for (const QJsonValue &editValue : changedEdits) {
        const QJsonObject editObject = editValue.toObject();
        if (!editObject.isEmpty()) {
            edits.append(editObject);
        }
    }

    if (!edits.isEmpty()) {
        return edits;
    }

    const QJsonArray documentChanges =
        workspaceEdit.value(QStringLiteral("documentChanges")).toArray();
    for (const QJsonValue &changeValue : documentChanges) {
        const QJsonObject changeObject = changeValue.toObject();
        const QJsonObject documentObject =
            changeObject.value(QStringLiteral("textDocument")).toObject();
        if (documentObject.value(QStringLiteral("uri")).toString() != uri) {
            continue;
        }

        const QJsonArray editArray = changeObject.value(QStringLiteral("edits")).toArray();
        for (const QJsonValue &editValue : editArray) {
            const QJsonObject editObject = editValue.toObject();
            if (!editObject.isEmpty()) {
                edits.append(editObject);
            }
        }
    }

    return edits;
}

QJsonObject buildHarperConfigurationObject(const HarperClient::RequestOptions &options) {
    QJsonObject linters;
    const QStringList knownLinters = {QStringLiteral("SpellCheck"),
                                      QStringLiteral("SpelledNumbers"),
                                      QStringLiteral("AnA"),
                                      QStringLiteral("SentenceCapitalization"),
                                      QStringLiteral("UnclosedQuotes"),
                                      QStringLiteral("WrongApostrophe"),
                                      QStringLiteral("QuoteSpacing"),
                                      QStringLiteral("NoFrenchSpaces"),
                                      QStringLiteral("LongSentences"),
                                      QStringLiteral("RepeatedWords"),
                                      QStringLiteral("Spaces"),
                                      QStringLiteral("CorrectNumberSuffix")};
    for (const QString &linter : knownLinters) {
        linters.insert(linter, options.enabledLinters.contains(linter));
    }

    QJsonObject harperConfig;
    harperConfig.insert(QStringLiteral("linters"), linters);
    harperConfig.insert(QStringLiteral("diagnosticSeverity"), QStringLiteral("hint"));
    harperConfig.insert(QStringLiteral("dialect"),
                        options.dialect.isEmpty() ? QStringLiteral("American") : options.dialect);
    return harperConfig;
}
}    // namespace

HarperClient::HarperClient(QObject *parent) : QObject(parent) {
    _timeoutTimer = new QTimer(this);
    _timeoutTimer->setSingleShot(true);
    connect(_timeoutTimer, &QTimer::timeout, this, [this]() {
        if (_pendingRequest.requestId > 0) {
            failPendingRequest(tr("Harper request timed out"), _pendingRequest.requestId);
        }
    });
}

HarperClient::~HarperClient() {
    cancelAllRequests();
    if (_transport) {
        _transport->stop();
    }
}

int HarperClient::checkText(const RequestOptions &options) {
    if (options.requestId <= 0) {
        return 0;
    }

    ensureTransport(options);
    cancelAllRequests();

    QString errorMessage;
    if (!ensureStarted(options, &errorMessage)) {
        Q_EMIT checkError(options.requestId, errorMessage);
        return options.requestId;
    }

    _currentOptions = options;
    _hasCurrentOptions = true;

    _pendingRequest.requestId = options.requestId;
    _pendingRequest.uri =
        QStringLiteral("file:///qownnotes-harper-%1.md").arg(QString::number(options.requestId));
    _pendingRequest.text = options.text;
    _pendingRequest.matches.clear();
    _pendingRequest.diagnostics.clear();
    _pendingRequest.codeActionRequestToMatchIndex.clear();

    _timeoutTimer->start(options.timeoutMs);

    if (!_initialized) {
        if (_initializeRequestId < 0) {
            startInitialization();
        }
        return options.requestId;
    }

    sendConfiguration(options);
    openPendingDocument();
    return options.requestId;
}

void HarperClient::cancelRequest(int requestId) {
    if (_pendingRequest.requestId != requestId || requestId <= 0) {
        return;
    }

    if (_timeoutTimer) {
        _timeoutTimer->stop();
    }

    if (_initialized && !_pendingRequest.uri.isEmpty()) {
        closeDocument(_pendingRequest.uri);
    }

    clearPendingRequest();
}

void HarperClient::cancelAllRequests() {
    if (_pendingRequest.requestId > 0) {
        cancelRequest(_pendingRequest.requestId);
    }
}

QString HarperClient::serverName() const { return _serverName; }

QString HarperClient::serverVersion() const { return _serverVersion; }

void HarperClient::ensureTransport(const RequestOptions &options) {
    const bool configurationChanged = !_hasCurrentOptions ||
                                      (_currentOptions.transportMode != options.transportMode) ||
                                      (_currentOptions.binaryPath != options.binaryPath) ||
                                      (_currentOptions.tcpAddress != options.tcpAddress) ||
                                      (_currentOptions.tcpPort != options.tcpPort);

    if (!configurationChanged) {
        return;
    }

    if (_transport) {
        _transport->stop();
    }

    resetProtocolState();

    if (options.transportMode == TcpTransport) {
        _transport.reset(new HarperTcpTransport(options.tcpAddress.trimmed().isEmpty()
                                                    ? QStringLiteral("127.0.0.1")
                                                    : options.tcpAddress.trimmed(),
                                                (options.tcpPort > 0) ? options.tcpPort : 4000));
    } else {
        _transport.reset(new HarperStdioTransport(options.binaryPath.trimmed().isEmpty()
                                                      ? QStringLiteral("harper-ls")
                                                      : options.binaryPath.trimmed()));
    }

    _transport->dataReceived = [this](const QByteArray &data) {
        _readBuffer.append(data);
        parseMessages();
    };
    _transport->errorOccurred = [this](const QString &message) {
        if (_pendingRequest.requestId > 0) {
            failPendingRequest(message, _pendingRequest.requestId);
        }
    };
    _transport->stopped = [this]() {
        const int requestId = _pendingRequest.requestId;
        resetProtocolState();
        if (requestId > 0) {
            failPendingRequest(tr("Harper service stopped unexpectedly"), requestId);
        }
    };
}

bool HarperClient::ensureStarted(const RequestOptions &options, QString *errorMessage) {
    if (!_transport) {
        if (errorMessage != nullptr) {
            *errorMessage = tr("Harper transport is not configured");
        }
        return false;
    }

    if (_transport->isRunning()) {
        return true;
    }

    if (options.transportMode == TcpTransport) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now < _nextTcpReconnectAttemptAtMs) {
            if (errorMessage != nullptr) {
                *errorMessage = tr("Waiting before retrying the Harper TCP connection");
            }
            return false;
        }
    }

    const bool started = _transport->start(errorMessage);
    if (!started && options.transportMode == TcpTransport) {
        _nextTcpReconnectAttemptAtMs = QDateTime::currentMSecsSinceEpoch() + 2000;
    }

    return started;
}

void HarperClient::resetProtocolState() {
    _readBuffer.clear();
    _initialized = false;
    _initializeRequestId = -1;
    _serverName.clear();
    _serverVersion.clear();
}

void HarperClient::closeDocument(const QString &uri) {
    if (!_initialized || uri.isEmpty()) {
        return;
    }

    QJsonObject doc;
    doc.insert(QStringLiteral("uri"), uri);

    QJsonObject params;
    params.insert(QStringLiteral("textDocument"), doc);

    QJsonObject object;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("method"), QStringLiteral("textDocument/didClose"));
    object.insert(QStringLiteral("params"), params);
    sendMessage(object);
}

void HarperClient::startInitialization() {
    QJsonObject clientInfo;
    clientInfo.insert(QStringLiteral("name"), QCoreApplication::applicationName());
    clientInfo.insert(QStringLiteral("version"), QCoreApplication::applicationVersion());

    QJsonObject codeAction;
    codeAction.insert(QStringLiteral("dynamicRegistration"), false);

    QJsonObject publishDiagnostics;
    publishDiagnostics.insert(QStringLiteral("relatedInformation"), false);

    QJsonObject textDocument;
    textDocument.insert(QStringLiteral("codeAction"), codeAction);
    textDocument.insert(QStringLiteral("publishDiagnostics"), publishDiagnostics);

    QJsonObject workspace;
    workspace.insert(QStringLiteral("configuration"), true);

    QJsonObject capabilities;
    capabilities.insert(QStringLiteral("textDocument"), textDocument);
    capabilities.insert(QStringLiteral("workspace"), workspace);

    QJsonObject params;
    params.insert(QStringLiteral("processId"), QCoreApplication::applicationPid());
    params.insert(QStringLiteral("rootPath"), QDir::currentPath());
    params.insert(QStringLiteral("rootUri"), QUrl::fromLocalFile(QDir::currentPath()).toString());
    params.insert(QStringLiteral("clientInfo"), clientInfo);
    params.insert(QStringLiteral("capabilities"), capabilities);

    QJsonObject object;
    _initializeRequestId = _nextLspRequestId++;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("id"), _initializeRequestId);
    object.insert(QStringLiteral("method"), QStringLiteral("initialize"));
    object.insert(QStringLiteral("params"), params);
    sendMessage(object);
}

void HarperClient::sendMessage(const QJsonObject &object) {
    if (!_transport || !_transport->isRunning()) {
        return;
    }

    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Compact);
    const QByteArray header = QByteArray("Content-Length: ") + QByteArray::number(payload.size()) +
                              QByteArray("\r\n\r\n");
    _transport->write(header + payload);
}

void HarperClient::parseMessages() {
    static const QRegularExpression contentLengthPattern(QStringLiteral("Content-Length: (\\d+)"),
                                                         QRegularExpression::CaseInsensitiveOption);

    while (true) {
        const int headerEnd = _readBuffer.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            return;
        }

        const QByteArray header = _readBuffer.left(headerEnd);
        const QRegularExpressionMatch match =
            contentLengthPattern.match(QString::fromLatin1(header));
        if (!match.hasMatch()) {
            _readBuffer.remove(0, headerEnd + 4);
            continue;
        }

        const int contentLength = match.captured(1).toInt();
        if ((_readBuffer.size() - headerEnd - 4) < contentLength) {
            return;
        }

        const QByteArray body = _readBuffer.mid(headerEnd + 4, contentLength);
        _readBuffer.remove(0, headerEnd + 4 + contentLength);

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        if ((parseError.error != QJsonParseError::NoError) || !document.isObject()) {
            continue;
        }

        const QJsonObject object = document.object();
        if (object.contains(QStringLiteral("method")) && object.contains(QStringLiteral("id"))) {
            handleRequest(object);
        } else if (object.contains(QStringLiteral("id"))) {
            handleResponse(object);
        } else {
            handleNotification(object);
        }
    }
}

void HarperClient::handleRequest(const QJsonObject &object) {
    const QJsonValue requestId = object.value(QStringLiteral("id"));
    const QString method = object.value(QStringLiteral("method")).toString();

    if (method == QStringLiteral("workspace/configuration")) {
        const QJsonArray items = object.value(QStringLiteral("params"))
                                     .toObject()
                                     .value(QStringLiteral("items"))
                                     .toArray();
        const QJsonObject settingsObject = buildSettingsObject(_currentOptions);
        const QJsonObject harperConfig =
            settingsObject.value(QStringLiteral("harper-ls")).toObject();

        QJsonArray result;
        for (const QJsonValue &itemValue : items) {
            const QString section =
                itemValue.toObject().value(QStringLiteral("section")).toString();
            if (section == QStringLiteral("harper-ls")) {
                result.append(harperConfig);
            } else if (section.isEmpty()) {
                result.append(settingsObject);
            } else {
                result.append(QJsonValue());
            }
        }

        sendResult(requestId, result);
        return;
    }

    if (method == QStringLiteral("client/registerCapability") ||
        method == QStringLiteral("client/unregisterCapability")) {
        sendResult(requestId, QJsonValue(QJsonValue::Null));
        return;
    }

    sendError(requestId, -32601, tr("Unsupported Harper request: %1").arg(method));
}

void HarperClient::handleResponse(const QJsonObject &object) {
    const int responseId = object.value(QStringLiteral("id")).toInt(-1);

    if (responseId == _initializeRequestId) {
        if (object.contains(QStringLiteral("error"))) {
            failPendingRequest(tr("Harper initialization failed"), _pendingRequest.requestId);
            return;
        }

        _initialized = true;
        _initializeRequestId = -1;
        const QJsonObject result = object.value(QStringLiteral("result")).toObject();
        updateServerInfo(result);
        sendInitializedNotification();

        if (_pendingRequest.requestId > 0) {
            sendConfiguration(_currentOptions);
            openPendingDocument();
        }

        return;
    }

    if (!_pendingRequest.codeActionRequestToMatchIndex.contains(responseId)) {
        return;
    }

    const int matchIndex = _pendingRequest.codeActionRequestToMatchIndex.take(responseId);
    if ((matchIndex >= 0) && (matchIndex < _pendingRequest.matches.size())) {
        const QStringList replacements =
            replacementsFromCodeActions(object.value(QStringLiteral("result")), _pendingRequest.uri,
                                        _pendingRequest.matches.at(matchIndex));
        if (!replacements.isEmpty()) {
            _pendingRequest.matches[matchIndex].replacements = replacements;
        }
    }

    finalizePendingRequestIfReady();
}

void HarperClient::handleNotification(const QJsonObject &object) {
    if (object.value(QStringLiteral("method")).toString() !=
        QStringLiteral("textDocument/publishDiagnostics")) {
        return;
    }

    const QJsonObject params = object.value(QStringLiteral("params")).toObject();
    if (params.value(QStringLiteral("uri")).toString() != _pendingRequest.uri) {
        return;
    }

    const QVector<int> lineOffsets = buildLineOffsets(_pendingRequest.text);
    const QJsonArray diagnosticsArray = params.value(QStringLiteral("diagnostics")).toArray();
    _pendingRequest.matches.clear();
    _pendingRequest.diagnostics.clear();
    _pendingRequest.codeActionRequestToMatchIndex.clear();
    _pendingRequest.matches.reserve(diagnosticsArray.size());
    _pendingRequest.diagnostics.reserve(diagnosticsArray.size());

    for (const QJsonValue &diagnosticValue : diagnosticsArray) {
        const QJsonObject diagnosticObject = diagnosticValue.toObject();
        const QJsonObject rangeObject = diagnosticObject.value(QStringLiteral("range")).toObject();
        const QJsonObject startObject = rangeObject.value(QStringLiteral("start")).toObject();
        const QJsonObject endObject = rangeObject.value(QStringLiteral("end")).toObject();

        const int offset = absoluteOffsetForPosition(
            lineOffsets, startObject.value(QStringLiteral("line")).toInt(),
            startObject.value(QStringLiteral("character")).toInt());
        const int endOffset =
            absoluteOffsetForPosition(lineOffsets, endObject.value(QStringLiteral("line")).toInt(),
                                      endObject.value(QStringLiteral("character")).toInt());
        if ((offset < 0) || (endOffset < offset)) {
            continue;
        }

        HarperMatch match;
        match.offset = offset;
        match.length = qMax(1, endOffset - offset);
        match.message = diagnosticObject.value(QStringLiteral("message")).toString();
        match.ruleId = diagnosticCodeToString(diagnosticObject.value(QStringLiteral("code")));
        match.shortMessage = match.ruleId;
        match.ruleCategory = categoryForRuleId(match.ruleId);
        match.contextText = contextTextForMatch(_pendingRequest.text, match.offset, match.length);

        const int matchIndex = _pendingRequest.matches.size();
        _pendingRequest.matches.append(match);
        _pendingRequest.diagnostics.append(diagnosticObject);

        const int codeActionRequestId = requestCodeAction(_pendingRequest.uri, diagnosticObject);
        if (codeActionRequestId > 0) {
            _pendingRequest.codeActionRequestToMatchIndex.insert(codeActionRequestId, matchIndex);
        }
    }

    if (_pendingRequest.matches.isEmpty()) {
        finalizePendingRequestIfReady();
        return;
    }

    finalizePendingRequestIfReady();
}

void HarperClient::sendInitializedNotification() {
    QJsonObject object;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("method"), QStringLiteral("initialized"));
    object.insert(QStringLiteral("params"), QJsonObject());
    sendMessage(object);
}

void HarperClient::sendConfiguration(const RequestOptions &options) {
    QJsonObject params;
    params.insert(QStringLiteral("settings"), buildSettingsObject(options));

    QJsonObject object;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("method"), QStringLiteral("workspace/didChangeConfiguration"));
    object.insert(QStringLiteral("params"), params);
    sendMessage(object);
}

void HarperClient::sendResult(const QJsonValue &requestId, const QJsonValue &result) {
    if (!_transport || !_transport->isRunning() || requestId.isUndefined()) {
        return;
    }

    QJsonObject object;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("id"), requestId);
    object.insert(QStringLiteral("result"), result);
    sendMessage(object);
}

void HarperClient::sendError(const QJsonValue &requestId, int code, const QString &message) {
    if (!_transport || !_transport->isRunning() || requestId.isUndefined()) {
        return;
    }

    QJsonObject errorObject;
    errorObject.insert(QStringLiteral("code"), code);
    errorObject.insert(QStringLiteral("message"), message);

    QJsonObject object;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("id"), requestId);
    object.insert(QStringLiteral("error"), errorObject);
    sendMessage(object);
}

QJsonObject HarperClient::buildSettingsObject(const RequestOptions &options) const {
    QJsonObject settingsObject;
    settingsObject.insert(QStringLiteral("harper-ls"), buildHarperConfigurationObject(options));
    return settingsObject;
}

void HarperClient::openPendingDocument() {
    if (!_initialized || (_pendingRequest.requestId <= 0) || _pendingRequest.uri.isEmpty()) {
        return;
    }

    QJsonObject textDocument;
    textDocument.insert(QStringLiteral("uri"), _pendingRequest.uri);
    textDocument.insert(QStringLiteral("languageId"), QStringLiteral("markdown"));
    textDocument.insert(QStringLiteral("version"), 1);
    textDocument.insert(QStringLiteral("text"), _pendingRequest.text);

    QJsonObject params;
    params.insert(QStringLiteral("textDocument"), textDocument);

    QJsonObject object;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("method"), QStringLiteral("textDocument/didOpen"));
    object.insert(QStringLiteral("params"), params);
    sendMessage(object);
}

int HarperClient::requestCodeAction(const QString &uri, const QJsonObject &diagnosticObject) {
    if (!_initialized || uri.isEmpty() || diagnosticObject.isEmpty()) {
        return -1;
    }

    QJsonObject textDocument;
    textDocument.insert(QStringLiteral("uri"), uri);

    const QJsonObject rangeObject = diagnosticObject.value(QStringLiteral("range")).toObject();

    QJsonArray diagnostics;
    diagnostics.append(diagnosticObject);

    QJsonObject context;
    context.insert(QStringLiteral("diagnostics"), diagnostics);

    QJsonObject params;
    params.insert(QStringLiteral("textDocument"), textDocument);
    params.insert(QStringLiteral("range"), rangeObject);
    params.insert(QStringLiteral("context"), context);

    QJsonObject object;
    const int requestId = _nextLspRequestId++;
    object.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    object.insert(QStringLiteral("id"), requestId);
    object.insert(QStringLiteral("method"), QStringLiteral("textDocument/codeAction"));
    object.insert(QStringLiteral("params"), params);
    sendMessage(object);
    return requestId;
}

void HarperClient::finalizePendingRequestIfReady() {
    if (_pendingRequest.requestId <= 0 ||
        !_pendingRequest.codeActionRequestToMatchIndex.isEmpty()) {
        return;
    }

    const int requestId = _pendingRequest.requestId;
    const QVector<HarperMatch> matches = _pendingRequest.matches;
    if (_timeoutTimer) {
        _timeoutTimer->stop();
    }
    closeDocument(_pendingRequest.uri);
    clearPendingRequest();
    Q_EMIT checkFinished(requestId, matches);
}

void HarperClient::failPendingRequest(const QString &errorMessage, int requestId) {
    if (_timeoutTimer) {
        _timeoutTimer->stop();
    }

    const int emittedRequestId = (requestId > 0) ? requestId : _pendingRequest.requestId;
    if (_pendingRequest.requestId > 0) {
        closeDocument(_pendingRequest.uri);
    }
    clearPendingRequest();

    if (emittedRequestId > 0) {
        Q_EMIT checkError(emittedRequestId, errorMessage);
    }
}

void HarperClient::clearPendingRequest() {
    _pendingRequest.requestId = 0;
    _pendingRequest.uri.clear();
    _pendingRequest.text.clear();
    _pendingRequest.matches.clear();
    _pendingRequest.diagnostics.clear();
    _pendingRequest.codeActionRequestToMatchIndex.clear();
}

void HarperClient::updateServerInfo(const QJsonObject &result) {
    const QJsonObject serverInfo = result.value(QStringLiteral("serverInfo")).toObject();
    _serverName = serverInfo.value(QStringLiteral("name")).toString();
    _serverVersion = serverInfo.value(QStringLiteral("version")).toString();
}

QVector<int> HarperClient::buildLineOffsets(const QString &text) {
    QVector<int> offsets;
    offsets.reserve(text.count(QLatin1Char('\n')) + 1);
    offsets.append(0);

    for (int i = 0; i < text.size(); ++i) {
        if (text.at(i) == QLatin1Char('\n')) {
            offsets.append(i + 1);
        }
    }

    return offsets;
}

int HarperClient::absoluteOffsetForPosition(const QVector<int> &lineOffsets, int line,
                                            int character) {
    if (line < 0 || line >= lineOffsets.size()) {
        return -1;
    }

    return lineOffsets.at(line) + qMax(0, character);
}

QString HarperClient::diagnosticCodeToString(const QJsonValue &value) {
    if (value.isString()) {
        return value.toString();
    }

    if (value.isDouble()) {
        return QString::number(value.toInt());
    }

    return QString();
}

QString HarperClient::categoryForRuleId(const QString &ruleId) {
    const QString normalized = ruleId.trimmed();
    if (normalized == QStringLiteral("SpellCheck")) {
        return QStringLiteral("Spelling");
    }
    if ((normalized == QStringLiteral("RepeatedWords")) ||
        (normalized == QStringLiteral("SentenceCapitalization")) ||
        (normalized == QStringLiteral("AnA")) ||
        (normalized == QStringLiteral("CorrectNumberSuffix"))) {
        return QStringLiteral("Grammar");
    }
    if ((normalized == QStringLiteral("LongSentences")) ||
        (normalized == QStringLiteral("SpelledNumbers"))) {
        return QStringLiteral("Style");
    }
    if ((normalized == QStringLiteral("Spaces")) ||
        (normalized == QStringLiteral("UnclosedQuotes")) ||
        (normalized == QStringLiteral("QuoteSpacing")) ||
        (normalized == QStringLiteral("NoFrenchSpaces"))) {
        return QStringLiteral("Punctuation");
    }
    if (normalized == QStringLiteral("WrongApostrophe")) {
        return QStringLiteral("Typography");
    }

    return QStringLiteral("Grammar");
}

QString HarperClient::contextTextForMatch(const QString &text, int offset, int length) {
    const int start = qMax(0, offset - 24);
    const int end = qMin(text.size(), offset + length + 24);
    return text.mid(start, end - start);
}

QStringList HarperClient::replacementsFromCodeActions(const QJsonValue &value, const QString &uri,
                                                      const HarperMatch &match) {
    QStringList replacements;
    QSet<QString> seen;
    if (!value.isArray()) {
        return replacements;
    }

    const QJsonArray actions = value.toArray();
    for (const QJsonValue &actionValue : actions) {
        const QJsonObject actionObject = actionValue.toObject();
        if (actionObject.isEmpty()) {
            continue;
        }

        const QString titleReplacement =
            decodeReplacementTitle(actionObject.value(QStringLiteral("title")).toString());
        if (!titleReplacement.isEmpty() && !seen.contains(titleReplacement)) {
            seen.insert(titleReplacement);
            replacements.append(titleReplacement);
        }

        const QJsonObject editObject = actionObject.value(QStringLiteral("edit")).toObject();
        const QVector<QJsonObject> edits = parseWorkspaceEditEntries(editObject, uri);
        for (const QJsonObject &edit : edits) {
            const QJsonObject rangeObject = edit.value(QStringLiteral("range")).toObject();
            const QJsonObject startObject = rangeObject.value(QStringLiteral("start")).toObject();
            const QJsonObject endObject = rangeObject.value(QStringLiteral("end")).toObject();

            Q_UNUSED(startObject)
            Q_UNUSED(endObject)

            const QString newText = edit.value(QStringLiteral("newText")).toString();
            if (newText.isEmpty() || seen.contains(newText)) {
                continue;
            }

            // Only surface direct replacement edits for the matched span.
            if (edit.contains(QStringLiteral("range"))) {
                seen.insert(newText);
                replacements.append(newText);
            }
        }
    }

    Q_UNUSED(match)

    return replacements;
}

#endif
