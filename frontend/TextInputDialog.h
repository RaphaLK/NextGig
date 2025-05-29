#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>

class TextInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextInputDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Enter Message");

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        inputField = new QTextEdit(this);
        inputField->setPlaceholderText("Write your message or cover letter here...");
        inputField->setMinimumHeight(150); // Makes the box bigger

        budgetField = new QLineEdit(this); // Use QLineEdit instead of QTextEdit
        budgetField->setPlaceholderText("Write proposed budget (e.g. $500) ... ");
        budgetField->setFixedHeight(30); // Smaller height
        mainLayout->addWidget(budgetField);

        mainLayout->addWidget(inputField);
        mainLayout->addWidget(budgetField);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *submitButton = new QPushButton("Submit", this);
        QPushButton *cancelButton = new QPushButton("Cancel", this);

        buttonLayout->addStretch();
        buttonLayout->addWidget(submitButton);
        buttonLayout->addWidget(cancelButton);

        mainLayout->addLayout(buttonLayout);

        connect(submitButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    }

    QString getInputText() const
    {
        return inputField->toPlainText();
    }

    QString getBudgetText() const
    {
        return budgetField->text();
    }

private:
    QTextEdit *inputField;
    QLineEdit *budgetField;
};

