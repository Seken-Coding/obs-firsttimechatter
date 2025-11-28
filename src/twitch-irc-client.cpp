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

#include "twitch-irc-client.hpp"
#include <QRegularExpression>
#include <QUrl>

TwitchIRCClient::TwitchIRCClient(QObject *parent)
	: QObject(parent),
	  m_socket(new QWebSocket()),
	  m_pingTimer(new QTimer(this)),
	  m_authenticated(false)
{
	connect(m_socket, &QWebSocket::connected, this, &TwitchIRCClient::onConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &TwitchIRCClient::onDisconnected);
	connect(m_socket, &QWebSocket::textMessageReceived, this, &TwitchIRCClient::onTextMessageReceived);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	connect(m_socket, &QWebSocket::errorOccurred, this, &TwitchIRCClient::onError);
#else
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
		&TwitchIRCClient::onError);
#endif
	connect(m_pingTimer, &QTimer::timeout, this, &TwitchIRCClient::sendPing);
}

TwitchIRCClient::~TwitchIRCClient()
{
	disconnect();
	delete m_socket;
}

void TwitchIRCClient::connectToChannel(const QString &channel, const QString &username, const QString &oauth)
{
	if (m_socket->state() != QAbstractSocket::UnconnectedState) {
		disconnect();
	}

	m_channel = channel.toLower();
	m_username = username.toLower();
	m_oauth = oauth;
	m_authenticated = false;

	// Ensure oauth has oauth: prefix
	if (!m_oauth.startsWith("oauth:")) {
		m_oauth = "oauth:" + m_oauth;
	}

	m_socket->open(QUrl("wss://irc-ws.chat.twitch.tv:443"));
}

void TwitchIRCClient::disconnect()
{
	m_pingTimer->stop();
	m_authenticated = false;
	if (m_socket->state() != QAbstractSocket::UnconnectedState) {
		m_socket->close();
	}
}

bool TwitchIRCClient::isConnected() const
{
	return m_socket->state() == QAbstractSocket::ConnectedState && m_authenticated;
}

void TwitchIRCClient::onConnected()
{
	// Request capabilities
	sendRawMessage("CAP REQ :twitch.tv/tags twitch.tv/commands");

	// Authenticate
	sendRawMessage("PASS " + m_oauth);
	sendRawMessage("NICK " + m_username);
}

void TwitchIRCClient::onDisconnected()
{
	m_pingTimer->stop();
	m_authenticated = false;
	emit disconnected();
}

void TwitchIRCClient::onTextMessageReceived(const QString &message)
{
	QStringList lines = message.split("\r\n", Qt::SkipEmptyParts);
	for (const QString &line : lines) {
		parseMessage(line);
	}
}

void TwitchIRCClient::onError(QAbstractSocket::SocketError error)
{
	Q_UNUSED(error);
	emit connectionError(m_socket->errorString());
}

void TwitchIRCClient::sendPing()
{
	sendRawMessage("PING :tmi.twitch.tv");
}

void TwitchIRCClient::sendRawMessage(const QString &message)
{
	if (m_socket->state() == QAbstractSocket::ConnectedState) {
		m_socket->sendTextMessage(message);
	}
}

void TwitchIRCClient::parseMessage(const QString &rawMessage)
{
	// Handle PING
	if (rawMessage.startsWith("PING")) {
		sendRawMessage("PONG :tmi.twitch.tv");
		return;
	}

	// Handle authentication success (001 numeric)
	if (rawMessage.contains(" 001 ")) {
		m_authenticated = true;
		// Join channel
		sendRawMessage("JOIN #" + m_channel);
		// Start ping timer (4 minutes)
		m_pingTimer->start(240000);
		emit connected();
		return;
	}

	// Handle authentication failure
	if (rawMessage.contains("NOTICE") && rawMessage.contains("Login authentication failed")) {
		emit connectionError("Login authentication failed");
		disconnect();
		return;
	}

	// Parse PRIVMSG
	if (rawMessage.contains("PRIVMSG")) {
		QString displayName;
		QString username;
		QString messageText;

		// Extract tags for display-name
		static QRegularExpression displayNameRegex("display-name=([^;]*)");
		QRegularExpressionMatch displayMatch = displayNameRegex.match(rawMessage);
		if (displayMatch.hasMatch()) {
			displayName = displayMatch.captured(1);
		}

		// Extract username from prefix
		static QRegularExpression usernameRegex(":([^!]+)!");
		QRegularExpressionMatch userMatch = usernameRegex.match(rawMessage);
		if (userMatch.hasMatch()) {
			username = userMatch.captured(1);
		}

		// Extract message text
		static QRegularExpression messageRegex("PRIVMSG #[^ ]+ :(.*)$");
		QRegularExpressionMatch msgMatch = messageRegex.match(rawMessage);
		if (msgMatch.hasMatch()) {
			messageText = msgMatch.captured(1);
		}

		if (!username.isEmpty()) {
			// Use username as display name if display name is empty
			if (displayName.isEmpty()) {
				displayName = username;
			}
			emit messageReceived(username.toLower(), displayName, messageText);
		}
	}
}
