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

#include <QHash>
#include <QJsonObject>
#include <QObject>

class QTcpServer;
class QTcpSocket;

/**
 * @brief MCP (Model Context Protocol) HTTP+SSE server service.
 *
 * Exposes QOwnNotes notes to external AI agents via the MCP protocol over
 * HTTP with Server-Sent Events (SSE) transport.
 *
 * Supported MCP tools:
 *   - search_notes: full-text search across notes
 *   - fetch_note:   retrieve a single note by name or ID
 */
class McpService : public QObject {
    Q_OBJECT

   public:
    explicit McpService(QObject *parent = nullptr);
    ~McpService() override;

    void start();
    void stop();
    bool isRunning() const;

    // Settings helpers
    static bool isEnabled();
    static quint16 getPort();
    static quint16 getDefaultPort();
    static QString getToken();
    static QString getOrGenerateToken();

   private:
    QTcpServer *m_server = nullptr;
    quint16 m_port = 0;

    // Active SSE connections mapped by a unique session ID
    QHash<QString, QTcpSocket *> m_sseClients;

    void handleNewConnection();
    void processRequest(QTcpSocket *socket, const QByteArray &requestData);

    // HTTP helpers
    static void sendHttpResponse(QTcpSocket *socket, int statusCode, const QByteArray &body,
                                 const QString &contentType = QLatin1String("application/json"),
                                 const QString &statusText = QLatin1String("OK"));
    static void sendCorsHeaders(QTcpSocket *socket);

    // SSE helpers
    void openSseStream(QTcpSocket *socket);
    void sendSseEvent(QTcpSocket *socket, const QString &event, const QByteArray &data);
    void handleJsonRpcOverHttp(QTcpSocket *socket, const QByteArray &body,
                               const QString &sessionId);

    // MCP JSON-RPC dispatch
    QJsonObject dispatchJsonRpc(const QJsonObject &request);

    // MCP method handlers
    QJsonObject handleInitialize(const QJsonObject &params, const QJsonValue &id);
    QJsonObject handleToolsList(const QJsonValue &id);
    QJsonObject handleToolsCall(const QJsonObject &params, const QJsonValue &id);

    // MCP tool implementations
    QJsonObject toolSearchNotes(const QJsonObject &arguments);
    QJsonObject toolFetchNote(const QJsonObject &arguments);

    // Auth helpers
    bool validateBearerToken(const QString &requestText) const;
    static QString parseBearerToken(const QString &requestText);
    static QString parseRequestPath(const QString &requestText);
    static QString parseRequestMethod(const QString &requestText);
    static QByteArray parseRequestBody(const QByteArray &requestData);
    static QString parseQueryParam(const QString &requestText, const QString &key);
};
