#include "HomeWindow.h"
#include "LoginWindow.h"
#include "SignUpWindow.h"
#include "HiringManagerPortal.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent)
{
	stack = new QStackedWidget(this);

	setupNavigationStack();
	// Set the stack as the main layout
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(stack);
	setLayout(mainLayout);
}

void HomeWindow::setupNavigationStack()
{
	// Create page objects here
	QWidget *homePage = RenderHomePage();
	LoginWindow *loginPage = new LoginWindow();
	SignUpWindow *signupPage = new SignUpWindow();
	HiringManagerPortal *h_managerPortal = new HiringManagerPortal;

	// Add objects to navigation stack -- indexes are as shown:
	stack->addWidget(homePage);		// Index 0
	stack->addWidget(loginPage);	// Index 1
	stack->addWidget(signupPage); // Index 2
	stack->addWidget(h_managerPortal);
	
	// add signals here
	connect(loginPage, &LoginWindow::returnToHomeRequested, [this]()
					{ stack->setCurrentIndex(0); });

	connect(loginPage, &LoginWindow::hiringManagerLoginSuccess, [this]()
					{ stack->setCurrentIndex(3); });

	connect(signupPage, &SignUpWindow::returnToHomeRequested, [this]()
					{ stack->setCurrentIndex(0); });

	connect(h_managerPortal, &HiringManagerPortal::returnToHomeRequested, [this]()
					{ stack->setCurrentIndex(0); });

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