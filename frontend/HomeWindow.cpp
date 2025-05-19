#include "HomeWindow.h"
#include "LoginWindow.h"
#include "SignUpWindow.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent)
{
	stack = new QStackedWidget(this);

	// Create pages
	QWidget *homePage = RenderHomePage();
	LoginWindow *loginPage = new LoginWindow();
	SignUpWindow *signupPage = new SignUpWindow();

	// Add pages to stack
	stack->addWidget(homePage);	  // Index 0
	stack->addWidget(loginPage);  // Index 1
	stack->addWidget(signupPage); // Index 2

	// Set the stack as the main layout
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(stack);
	setLayout(mainLayout);
}

QWidget *HomeWindow::RenderHomePage()
{
	QWidget *homePage = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(homePage);

	// Title
	QLabel *title = new QLabel("NextGig", homePage);
	QFont titleFont;
	titleFont.setPointSize(20);
	titleFont.setBold(true);
	title->setFont(titleFont);
	title->setAlignment(Qt::AlignCenter);

	// Subtitle
	QLabel *subtitle = new QLabel("Find your next opportunity or hire top talent.", homePage);
	subtitle->setAlignment(Qt::AlignCenter);

	// Buttons
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	QPushButton *loginBtn = new QPushButton("Login", homePage);
	QPushButton *signupBtn = new QPushButton("Sign Up", homePage);
	QPushButton *helpBtn = new QPushButton("Help", homePage);
	helpBtn->setMaximumWidth(45);

	buttonLayout->addWidget(loginBtn);
	buttonLayout->addWidget(signupBtn);
	buttonLayout->addWidget(helpBtn);

	// Assemble home page
	layout->addWidget(title);
	layout->addWidget(subtitle);
	layout->addSpacing(30);
	layout->addLayout(buttonLayout);
	layout->addStretch();

	// Connect buttons to switch pages
	connect(loginBtn, &QPushButton::clicked, [this]()
			{
				stack->setCurrentIndex(1); // Go to login
			});
	connect(signupBtn, &QPushButton::clicked, [this]()
			{
				stack->setCurrentIndex(2); // Go to signup
			});

	return homePage;
}