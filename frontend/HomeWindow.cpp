#include "HomeWindow.h"
#include "LoginWindow.h"
#include "SignUpWindow.h"
#include "HiringManagerPortal.h"
#include "FreelancerPortal.h"
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
    // Create only non-user-specific pages at startup
    QWidget *homePage = RenderHomePage();
    LoginWindow *loginPage = new LoginWindow();
    SignUpWindow *signupPage = new SignUpWindow();
    
    // Add objects to navigation stack -- indexes are as shown:
    stack->addWidget(homePage);       // Index 0
    stack->addWidget(loginPage);      // Index 1
    stack->addWidget(signupPage);     // Index 2
    
    // Add placeholders for portals
    hiringManagerIndex = -1;   // We'll set this when we create the portal
    freelancerIndex = -1;      // We'll set this when we create the portal

    // Connect login success signals to portal creation methods
    connect(loginPage, &LoginWindow::hiringManagerLoginSuccess, 
            this, &HomeWindow::createAndShowHiringManagerPortal);
            
    connect(loginPage, &LoginWindow::freelancerLoginSuccess, 
            this, &HomeWindow::createAndShowFreelancerPortal);

    // Other navigation connections
    connect(loginPage, &LoginWindow::returnToHomeRequested, 
            [this](){ stack->setCurrentIndex(0); });
            
    connect(signupPage, &SignUpWindow::returnToHomeRequested, 
            [this](){ stack->setCurrentIndex(0); });
}

void HomeWindow::createAndShowHiringManagerPortal()
{
    // Check if we already have a hiring manager portal
    if (hiringManagerIndex >= 0) {
        // Remove the old portal if it exists
        QWidget* oldPortal = stack->widget(hiringManagerIndex);
        if (oldPortal) {
            stack->removeWidget(oldPortal);
            delete oldPortal;
        }
    }
    
    // Create a new portal
    HiringManagerPortal *portal = new HiringManagerPortal();
    hiringManagerIndex = stack->addWidget(portal);
    
    // Connect its navigation signals
    connect(portal, &HiringManagerPortal::returnToHomeRequested, 
            [this](){ stack->setCurrentIndex(0); });
    
    // Show the portal
    stack->setCurrentIndex(hiringManagerIndex);
}

void HomeWindow::createAndShowFreelancerPortal()
{
    // Check if we already have a freelancer portal
    if (freelancerIndex >= 0) {
        // Just show the existing portal instead of recreating
        stack->setCurrentIndex(freelancerIndex);
        return;
    }
    
    // Create a new portal only if it doesn't exist
    FreelancerPortal *portal = new FreelancerPortal();
    freelancerIndex = stack->addWidget(portal);

    connect(portal, &FreelancerPortal::returnToHomeRequested, 
            [this](){ 
                stack->setCurrentIndex(0); 
                // Optionally clear the portal when returning home
                // clearFreelancerPortal();
            });
    
    // Show the portal
    stack->setCurrentIndex(freelancerIndex);
}

// Add this method to properly clear the portal when needed
void HomeWindow::clearFreelancerPortal()
{
    if (freelancerIndex >= 0) {
        QWidget* oldPortal = stack->widget(freelancerIndex);
        if (oldPortal) {
            stack->removeWidget(oldPortal);
            delete oldPortal;
        }
        freelancerIndex = -1;
    }
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