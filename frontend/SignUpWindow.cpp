#include "SignUpWindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

SignUpWindow::SignUpWindow(QWidget *parent) : QWidget(parent)
{
	QWidget *signUpPage = RenderSignUpWindow();
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(signUpPage);
	setLayout(mainLayout);
}

QWidget* SignUpWindow::RenderSignUpWindow()
{
	QWidget *signUpWindow = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(signUpWindow);

	QLabel *title = new QLabel("Make an Account", signUpWindow);
	QFont titleFont;
	titleFont.setPointSize(15);
	title->setFont(titleFont);
	
	return signUpWindow;
}