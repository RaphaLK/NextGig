#include "HomeWindow.h"
#include "LoginWindow.h"
#include "SignUpWindow.h"
#include "HiringManagerPortal.h"
#include "FreelancerPortal.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyleFactory>
#include <QMessageBox>
#include <QApplication>
#include <QPixmap>

HomeWindow::HomeWindow(QWidget *parent) : QWidget(parent)
{
    // Set window properties
    setWindowTitle("NextGig - Find Your Next Opportunity");
    setMinimumSize(800, 600);
    
    // Create stacked widget for navigation
    stack = new QStackedWidget(this);
    
    // Set up the application style
    qApp->setStyle(QStyleFactory::create("Fusion"));
    
    // Set application-wide stylesheet
    qApp->setStyleSheet(
        "QWidget { font-family: Arial, sans-serif; }"
        "QLabel { color: #2C3E50; }"
        "QLineEdit, QComboBox { padding: 8px; border: 1px solid #BDC3C7; border-radius: 4px; }"
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #3498DB; }"
    );
    
    setupNavigationStack();
    
    // Set the stack as the main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
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
    
    // Set margins and spacing
    layout->setContentsMargins(40, 60, 40, 60);
    layout->setSpacing(15);
    
    // Logo and title area
    QLabel *logo = new QLabel(homePage);
    logo->setPixmap(QPixmap(":/images/nextgig_logo.png").scaledToWidth(120, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignCenter);
    
    // Title with improved styling
    QLabel *title = new QLabel("NextGig", homePage);
    QFont titleFont;
    titleFont.setPointSize(32);
    titleFont.setBold(true);
    titleFont.setFamily("Arial"); // Consider a more modern font if available
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #2C3E50;"); // Dark blue-gray color
    
    // Subtitle with improved styling
    QLabel *subtitle = new QLabel("Find your next opportunity or hire top talent.", homePage);
    QFont subtitleFont;
    subtitleFont.setPointSize(14);
    subtitleFont.setItalic(true);
    subtitle->setFont(subtitleFont);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color: #7F8C8D; margin-bottom: 20px;"); // Subtle gray color
    
    // Buttons with improved styling
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    
    QPushButton *loginBtn = new QPushButton("Login", homePage);
    QPushButton *signupBtn = new QPushButton("Sign Up", homePage);
    QPushButton *helpBtn = new QPushButton("Help", homePage);
    
    // Style the buttons
    QString buttonStyle = "QPushButton {"
                        "    padding: 10px 30px;"
                        "    font-size: 14px;"
                        "    border-radius: 5px;"
                        "    font-weight: bold;"
                        "}"
                        "QPushButton:hover { background-color: #D6EAF8; }";
    
    loginBtn->setStyleSheet(buttonStyle + "QPushButton { background-color: #3498DB; color: white; }");
    signupBtn->setStyleSheet(buttonStyle + "QPushButton { background-color: #2ECC71; color: white; }");
    helpBtn->setStyleSheet(buttonStyle);
    helpBtn->setFixedWidth(100);
    
    buttonLayout->addWidget(loginBtn);
    buttonLayout->addWidget(signupBtn);
    buttonLayout->addWidget(helpBtn);
    
    // Add a card-like container for buttons
    QWidget *buttonContainer = new QWidget(homePage);
    buttonContainer->setLayout(buttonLayout);
    buttonContainer->setStyleSheet("background-color: #ECF0F1; border-radius: 10px; padding: 20px;");
    
    // Add some features/benefits text
    QLabel *featuresLabel = new QLabel("• Connect with top employers\n• Showcase your portfolio\n• Find projects that match your skills", homePage);
    featuresLabel->setAlignment(Qt::AlignCenter);
    featuresLabel->setStyleSheet("color: #7F8C8D; margin: 20px 0;");
    
    // Assemble home page
    layout->addStretch(1);
    layout->addWidget(logo);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(featuresLabel);
    layout->addSpacing(20);
    layout->addWidget(buttonContainer);
    layout->addStretch(2);
    
    // Set background for the entire page
    homePage->setStyleSheet("QWidget { background-color: #F5F7FA; }");
    
    // Connect buttons to switch pages
    connect(loginBtn, &QPushButton::clicked, [this]() {
        stack->setCurrentIndex(1); // Go to login
    });
    
    connect(signupBtn, &QPushButton::clicked, [this]() {
        stack->setCurrentIndex(2); // Go to signup
    });
    
    // Add help button functionality
    connect(helpBtn, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Help", 
            "NextGig connects talented freelancers with hiring managers.\n\n"
            "• Use Sign Up to create a new account\n"
            "• Use Login if you already have an account\n\n"
            "For support, contact help@nextgig.com");
    });
    
    return homePage;
}