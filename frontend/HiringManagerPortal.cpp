#include "HiringManagerPortal.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QSpacerItem>

HiringManagerPortal::HiringManagerPortal(QWidget *parent) : QWidget(parent)
{
    QWidget *hiringManagerPortal = renderHiringManagerPortal();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(hiringManagerPortal);

    setLayout(mainLayout);
}

// vibes
QWidget* HiringManagerPortal::renderHiringManagerPortal(){
    QWidget *h_managerPortal = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(h_managerPortal);
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    
    // Welcome label
    QLabel *welcomeLabel = new QLabel("Welcome, Hiring Manager!", h_managerPortal);
    QFont welcomeFont;
    welcomeFont.setPointSize(16);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setAlignment(Qt::AlignCenter);

    QString buttonStyle = "padding: 8px; font-size: 15px;";
    // Post new job button
    QPushButton *postJobBtn = new QPushButton("Post Job", h_managerPortal);
    postJobBtn->setStyleSheet(buttonStyle);
    postJobBtn->setFixedWidth(100); 

    // Posted jobs list (placeholder)
    QLabel *jobsLabel = new QLabel("Your Posted Jobs:", h_managerPortal);
    QListWidget *jobsList = new QListWidget(h_managerPortal);
    jobsList->addItem("Frontend Developer - React.js");
    jobsList->addItem("Backend Developer - Node.js");
    jobsList->addItem("UI/UX Designer");

    // Logout button
    QPushButton *logoutBtn = new QPushButton("Logout", h_managerPortal);
    logoutBtn->setStyleSheet("padding: 8px; font-size: 13px; color: #fff; background: #d9534f;");

    // Messages
    QPushButton *messagesBtn = new QPushButton ("Inbox", h_managerPortal);
    messagesBtn->setStyleSheet(buttonStyle);
    messagesBtn->setFixedWidth(100);

    // Layout arrangement
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addSpacing(10);
    optionsLayout->addWidget(postJobBtn, 0, Qt::AlignCenter);
    optionsLayout->addWidget(messagesBtn, 0, Qt::AlignCenter);
    mainLayout->addLayout(optionsLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(jobsLabel);
    mainLayout->addWidget(jobsList);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(logoutBtn, 0, Qt::AlignRight);

    return h_managerPortal;
}