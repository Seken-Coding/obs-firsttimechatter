/*
First Time Chatter Dock for OBS
Copyright (C) 2024 Seken Coding

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#pragma once

#include <QObject>
#include <QString>
#include <QWebSocket>
#include <QTimer>

class TwitchIRCClient : public QObject {
	Q_OBJECT

public:
	explicit TwitchIRCClient(QObject *parent = nullptr);
	~TwitchIRCClient();

	void connectToChannel(const QString &channel, const QString &username, const QString &oauth);
	void disconnect();
	bool isConnected() const;

signals:
	void connected();
	void disconnected();
	void messageReceived(const QString &username, const QString &displayName, const QString &message);
	void connectionError(const QString &error);

private slots:
	void onConnected();
	void onDisconnected();
	void onTextMessageReceived(const QString &message);
	void onError(QAbstractSocket::SocketError error);
	void sendPing();

private:
	void sendRawMessage(const QString &message);
	void parseMessage(const QString &rawMessage);

	QWebSocket *m_socket;
	QTimer *m_pingTimer;
	QString m_channel;
	QString m_username;
	QString m_oauth;
	bool m_authenticated;
};
