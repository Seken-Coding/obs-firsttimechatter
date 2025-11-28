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

#include "settings-dialog.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

SettingsDialog::SettingsDialog(const QString &channel, const QString &username, const QString &oauth,
			       const QStringList &botList, QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("First-Time Chatter Settings");
	setMinimumWidth(400);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	// Form layout for input fields
	QFormLayout *formLayout = new QFormLayout();

	m_channelEdit = new QLineEdit(channel, this);
	m_channelEdit->setPlaceholderText("Channel name (without #)");
	formLayout->addRow("Channel:", m_channelEdit);

	m_usernameEdit = new QLineEdit(username, this);
	m_usernameEdit->setPlaceholderText("Your Twitch username");
	formLayout->addRow("Username:", m_usernameEdit);

	m_oauthEdit = new QLineEdit(oauth, this);
	m_oauthEdit->setPlaceholderText("oauth:xxxxxxxxxxxxxxxxxxxx");
	m_oauthEdit->setEchoMode(QLineEdit::Password);
	formLayout->addRow("OAuth Token:", m_oauthEdit);

	mainLayout->addLayout(formLayout);

	// OAuth help link
	QLabel *oauthLink = new QLabel("<a href=\"https://twitchapps.com/tmi/\">Get your OAuth token here</a>", this);
	oauthLink->setOpenExternalLinks(true);
	mainLayout->addWidget(oauthLink);

	// Bot list
	QLabel *botLabel = new QLabel("Bot List (one per line):", this);
	mainLayout->addWidget(botLabel);

	m_botListEdit = new QTextEdit(this);
	m_botListEdit->setPlainText(botList.join("\n"));
	m_botListEdit->setMaximumHeight(120);
	mainLayout->addWidget(m_botListEdit);

	// Buttons
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	QPushButton *okButton = new QPushButton("OK", this);
	QPushButton *cancelButton = new QPushButton("Cancel", this);

	connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);
	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
}

QString SettingsDialog::channel() const
{
	return m_channelEdit->text().trimmed();
}

QString SettingsDialog::username() const
{
	return m_usernameEdit->text().trimmed();
}

QString SettingsDialog::oauth() const
{
	return m_oauthEdit->text().trimmed();
}

QStringList SettingsDialog::botList() const
{
	QString text = m_botListEdit->toPlainText();
	QStringList bots = text.split('\n', Qt::SkipEmptyParts);
	QStringList result;
	for (const QString &bot : bots) {
		QString trimmed = bot.trimmed();
		if (!trimmed.isEmpty()) {
			result.append(trimmed);
		}
	}
	return result;
}
