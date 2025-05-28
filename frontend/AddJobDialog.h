#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QDialogButtonBox>

class AddJobDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddJobDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Post a New Job");
        auto *layout = new QFormLayout(this);

        titleEdit = new QLineEdit(this);
        descEdit = new QTextEdit(this);
        employerEdit = new QLineEdit(this);
        expiryDateEdit = new QDateEdit(QDate::currentDate(), this);
        expiryDateEdit->setCalendarPopup(true);
        paymentEdit = new QLineEdit(this);
        skillsEdit = new QLineEdit(this);
        tagsEdit = new QLineEdit(this);

        layout->addRow("Job Title:", titleEdit);
        layout->addRow("Description:", descEdit);
        layout->addRow("Employer Name:", employerEdit);
        layout->addRow("Expiry Date:", expiryDateEdit);
        layout->addRow("Payment:", paymentEdit);
        layout->addRow("Required Skills\n(comma-separated):", skillsEdit);
        layout->addRow("Job Tags\n(comma-separated):", tagsEdit);

        auto *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &AddJobDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &AddJobDialog::reject);
        layout->addRow(buttons);
    }

    // getters to pull values after accept()
    QString jobTitle() const { return titleEdit->text(); }
    QString jobDescription() const { return descEdit->toPlainText(); }
    QString employerName() const { return employerEdit->text(); }
    QDate expiryDate() const { return expiryDateEdit->date(); }
    QString payment() const { return paymentEdit->text(); }
    QStringList requiredSkills() const
    {
        return skillsEdit->text().split(',', Qt::SkipEmptyParts);
    }
    QStringList jobTags() const
    {
        return tagsEdit->text().split(',', Qt::SkipEmptyParts);
    }

private:
    QLineEdit *titleEdit;
    QTextEdit *descEdit;
    QLineEdit *employerEdit;
    QDateEdit *expiryDateEdit;
    QLineEdit *paymentEdit;
    QLineEdit *skillsEdit;
    QLineEdit *tagsEdit;
};
