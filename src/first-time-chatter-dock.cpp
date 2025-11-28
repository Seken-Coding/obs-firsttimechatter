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

#include "first-time-chatter-dock.hpp"
#include "twitch-irc-client.hpp"
#include "settings-dialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDate>
#include <QSettings>
#include <QFont>

FirstTimeChatterDock::FirstTimeChatterDock(QWidget *parent)
	: QFrame(parent),
	  m_ircClient(new TwitchIRCClient(this)),
	  m_isConnected(false),
	  m_isConnecting(false)
{
	// Initialize default bot list
	m_botList = {"nightbot",     "streamelements", "soundalerts", "moobot",
		     "streamlabs",   "tangiabot",      "sery_bot",    "mei_the_fox"};

	setupUI();
	applyStylesheet();
	loadSettings();
	checkDailyReset();

	// Connect IRC client signals
	connect(m_ircClient, &TwitchIRCClient::connected, this, &FirstTimeChatterDock::onIRCConnected);
	connect(m_ircClient, &TwitchIRCClient::disconnected, this, &FirstTimeChatterDock::onIRCDisconnected);
	connect(m_ircClient, &TwitchIRCClient::messageReceived, this, &FirstTimeChatterDock::onIRCMessageReceived);
	connect(m_ircClient, &TwitchIRCClient::connectionError, this, &FirstTimeChatterDock::onIRCConnectionError);
}

FirstTimeChatterDock::~FirstTimeChatterDock()
{
	saveSettings();
}

void FirstTimeChatterDock::setupUI()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(8, 8, 8, 8);
	mainLayout->setSpacing(8);

	// Header layout
	QHBoxLayout *headerLayout = new QHBoxLayout();
	m_titleLabel = new QLabel("First-Time Chatters", this);
	QFont titleFont = m_titleLabel->font();
	titleFont.setBold(true);
	titleFont.setPointSize(14);
	m_titleLabel->setFont(titleFont);

	m_statusLabel = new QLabel(QString::fromUtf8("\u26AB"), this); // Black circle
	m_statusLabel->setToolTip("Disconnected");

	headerLayout->addWidget(m_titleLabel);
	headerLayout->addStretch();
	headerLayout->addWidget(m_statusLabel);
	mainLayout->addLayout(headerLayout);

	// List widget
	m_listWidget = new QListWidget(this);
	m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
	connect(m_listWidget, &QListWidget::itemClicked, this, &FirstTimeChatterDock::onItemClicked);
	mainLayout->addWidget(m_listWidget, 1);

	// Button layout
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	m_connectButton = new QPushButton("Connect", this);
	m_resetButton = new QPushButton("Reset", this);
	m_settingsButton = new QPushButton(QString::fromUtf8("\u2699"), this); // Gear icon
	m_settingsButton->setMaximumWidth(40);
	m_settingsButton->setToolTip("Settings");

	connect(m_connectButton, &QPushButton::clicked, this, &FirstTimeChatterDock::onConnectClicked);
	connect(m_resetButton, &QPushButton::clicked, this, &FirstTimeChatterDock::onResetClicked);
	connect(m_settingsButton, &QPushButton::clicked, this, &FirstTimeChatterDock::onSettingsClicked);

	buttonLayout->addWidget(m_connectButton);
	buttonLayout->addWidget(m_resetButton);
	buttonLayout->addWidget(m_settingsButton);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
}

void FirstTimeChatterDock::applyStylesheet()
{
	setStyleSheet(R"(
        QFrame {
            background-color: #18181b;
            color: #efeff1;
        }
        QLabel {
            font-size: 13px;
            color: #efeff1;
        }
        QListWidget {
            background-color: #0e0e10;
            border: 1px solid #303032;
            border-radius: 4px;
            color: #efeff1;
        }
        QListWidget::item {
            padding: 6px 8px;
            border-radius: 4px;
            margin: 2px 4px;
        }
        QListWidget::item:hover {
            background-color: #26262c;
        }
        QPushButton {
            background-color: #3a3a3d;
            color: #efeff1;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #4a4a4d;
        }
    )");
}

void FirstTimeChatterDock::loadSettings()
{
	QSettings settings("FirstTimeChatterDock", "OBS");
	m_channel = settings.value("channel", "").toString();
	m_username = settings.value("username", "").toString();
	m_oauth = settings.value("oauth", "").toString();
	m_lastSessionDate = settings.value("lastSessionDate", "").toString();

	// Load bot list
	QStringList botList = settings.value("botList").toStringList();
	if (!botList.isEmpty()) {
		m_botList.clear();
		for (const QString &bot : botList) {
			m_botList.insert(bot.toLower());
		}
	}

	// Load clicked chatters
	QStringList clickedList = settings.value("clickedChatters").toStringList();
	for (const QString &chatter : clickedList) {
		m_clickedChatters.insert(chatter);
	}

	// Load seen chatters
	QStringList seenList = settings.value("seenChatters").toStringList();
	for (const QString &chatter : seenList) {
		m_seenChatters.insert(chatter);
	}
}

void FirstTimeChatterDock::saveSettings()
{
	QSettings settings("FirstTimeChatterDock", "OBS");
	settings.setValue("channel", m_channel);
	settings.setValue("username", m_username);
	settings.setValue("oauth", m_oauth);
	settings.setValue("lastSessionDate", m_lastSessionDate);
	settings.setValue("botList", QStringList(m_botList.begin(), m_botList.end()));
	settings.setValue("clickedChatters", QStringList(m_clickedChatters.begin(), m_clickedChatters.end()));
	settings.setValue("seenChatters", QStringList(m_seenChatters.begin(), m_seenChatters.end()));
}

void FirstTimeChatterDock::checkDailyReset()
{
	QString currentDate = QDate::currentDate().toString(Qt::ISODate);
	if (m_lastSessionDate != currentDate) {
		// Reset for new day
		m_seenChatters.clear();
		m_clickedChatters.clear();
		m_listWidget->clear();
		m_lastSessionDate = currentDate;
		saveSettings();
	}
}

void FirstTimeChatterDock::updateConnectionStatus()
{
	if (m_isConnected) {
		m_statusLabel->setText(QString::fromUtf8("\U0001F7E2")); // Green circle
		m_statusLabel->setToolTip("Connected");
		m_connectButton->setText("Disconnect");
	} else if (m_isConnecting) {
		m_statusLabel->setText(QString::fromUtf8("\U0001F7E1")); // Yellow circle
		m_statusLabel->setToolTip("Connecting...");
		m_connectButton->setText("Cancel");
	} else {
		m_statusLabel->setText(QString::fromUtf8("\u26AB")); // Black circle
		m_statusLabel->setToolTip("Disconnected");
		m_connectButton->setText("Connect");
	}
}

bool FirstTimeChatterDock::isBot(const QString &username)
{
	return m_botList.contains(username.toLower());
}

void FirstTimeChatterDock::onConnectClicked()
{
	if (m_isConnected || m_isConnecting) {
		m_ircClient->disconnect();
		m_isConnecting = false;
		updateConnectionStatus();
	} else {
		if (m_channel.isEmpty() || m_username.isEmpty() || m_oauth.isEmpty()) {
			onSettingsClicked();
			return;
		}
		m_isConnecting = true;
		updateConnectionStatus();
		m_ircClient->connectToChannel(m_channel, m_username, m_oauth);
	}
}

void FirstTimeChatterDock::onResetClicked()
{
	m_seenChatters.clear();
	m_clickedChatters.clear();
	m_listWidget->clear();
	saveSettings();
}

void FirstTimeChatterDock::onSettingsClicked()
{
	SettingsDialog dialog(m_channel, m_username, m_oauth,
			      QStringList(m_botList.begin(), m_botList.end()), this);
	if (dialog.exec() == QDialog::Accepted) {
		m_channel = dialog.channel();
		m_username = dialog.username();
		m_oauth = dialog.oauth();

		m_botList.clear();
		QStringList bots = dialog.botList();
		for (const QString &bot : bots) {
			m_botList.insert(bot.toLower());
		}

		saveSettings();
	}
}

void FirstTimeChatterDock::onIRCConnected()
{
	m_isConnected = true;
	m_isConnecting = false;
	updateConnectionStatus();
}

void FirstTimeChatterDock::onIRCDisconnected()
{
	m_isConnected = false;
	m_isConnecting = false;
	updateConnectionStatus();
}

void FirstTimeChatterDock::onIRCMessageReceived(const QString &username, const QString &displayName,
						const QString &message)
{
	Q_UNUSED(message);

	// Skip bots
	if (isBot(username)) {
		return;
	}

	// Check if first time chatter this session
	if (!m_seenChatters.contains(username)) {
		m_seenChatters.insert(username);

		// Add to list (newest at top)
		QListWidgetItem *item = new QListWidgetItem(displayName);
		item->setData(Qt::UserRole, username);

		// Check if previously clicked
		if (m_clickedChatters.contains(username)) {
			QFont font = item->font();
			font.setStrikeOut(true);
			item->setFont(font);
			item->setForeground(QColor("#666666"));
		}

		m_listWidget->insertItem(0, item);
		saveSettings();
	}
}

void FirstTimeChatterDock::onIRCConnectionError(const QString &error)
{
	m_isConnecting = false;
	m_isConnected = false;
	m_statusLabel->setText(QString::fromUtf8("\U0001F534")); // Red circle
	m_statusLabel->setToolTip("Error: " + error);
	m_connectButton->setText("Connect");
}

void FirstTimeChatterDock::onItemClicked(QListWidgetItem *item)
{
	QString username = item->data(Qt::UserRole).toString();
	QFont font = item->font();

	if (font.strikeOut()) {
		// Remove strikethrough
		font.setStrikeOut(false);
		item->setFont(font);
		item->setForeground(QColor("#efeff1"));
		m_clickedChatters.remove(username);
	} else {
		// Add strikethrough
		font.setStrikeOut(true);
		item->setFont(font);
		item->setForeground(QColor("#666666"));
		m_clickedChatters.insert(username);
	}

	saveSettings();
}
