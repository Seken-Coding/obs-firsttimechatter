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

#include <QFrame>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QString>

class TwitchIRCClient;

class FirstTimeChatterDock : public QFrame {
	Q_OBJECT

public:
	explicit FirstTimeChatterDock(QWidget *parent = nullptr);
	~FirstTimeChatterDock();

private slots:
	void onConnectClicked();
	void onResetClicked();
	void onSettingsClicked();
	void onIRCConnected();
	void onIRCDisconnected();
	void onIRCMessageReceived(const QString &username, const QString &displayName, const QString &message);
	void onIRCConnectionError(const QString &error);
	void onItemClicked(QListWidgetItem *item);

private:
	void setupUI();
	void applyStylesheet();
	void loadSettings();
	void saveSettings();
	void checkDailyReset();
	void updateConnectionStatus();
	bool isBot(const QString &username);

	TwitchIRCClient *m_ircClient;
	QListWidget *m_listWidget;
	QLabel *m_titleLabel;
	QLabel *m_statusLabel;
	QPushButton *m_connectButton;
	QPushButton *m_resetButton;
	QPushButton *m_settingsButton;

	QSet<QString> m_seenChatters;
	QSet<QString> m_clickedChatters;
	QSet<QString> m_botList;

	QString m_channel;
	QString m_username;
	QString m_oauth;
	QString m_lastSessionDate;

	bool m_isConnected;
	bool m_isConnecting;
};
