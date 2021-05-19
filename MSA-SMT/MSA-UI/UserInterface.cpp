#include "UserInterface.h"
#include "MSA.h"

#include <vector>

#include <QStringList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QFrame>
#include <QPushButton>
#include <QFont>
#include <QGroupBox>

UserInterface::UserInterface(QWidget *parent) : QMainWindow(parent)
{
	setWindowTitle("Multiple Sequence Alignment");

	// Build a grid layout for the settings
	QWidget* pCentralWidget = new QWidget(this);
	setCentralWidget(pCentralWidget);
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pCentralWidget->setLayout(pMainLayout);

	// Labels
	QLabel* pLengthLabel = new QLabel("Enter the maximum length of the alignment (0=find minimal):", pCentralWidget);
	QLabel* pInputLabel = new QLabel("Enter sequences separated by commas:", pCentralWidget);
	QLabel* pTitle = new QLabel("Multiple Sequence Alignment", pCentralWidget);
	QFont f;
	f.setBold(true);
	f.setPointSize(20);
	pTitle->setFont(f);

	// Settings
	QSpinBox* pMaxLengthSpinBox = new QSpinBox(pCentralWidget);
	pMaxLengthSpinBox->setValue(0);
	pMaxLengthSpinBox->setRange(0, 999);
	pMaxLengthSpinBox->setToolTip("MaxLength=0 solves for the optimal solution, which will take extra time.");
	QCheckBox* pTightConstraintCheckBox = new QCheckBox("Tight Constraints", pCentralWidget);
	pTightConstraintCheckBox->setToolTip("When enabled, creates much tighter constraints for the solver. May run faster in some cases.");
	QTextEdit* pInputEdit = new QTextEdit(pCentralWidget);

	// Buttons
	QPushButton* pAlignButton = new QPushButton("Align", pCentralWidget);
	pAlignButton->setDisabled(true);

	// Create group boxes
	QGroupBox* pSettingsBox = new QGroupBox("Settings", pCentralWidget);
	QGroupBox* pInputBox = new QGroupBox("Input", pCentralWidget);

	// Layout group boxes
	QVBoxLayout* pSettingsLayout = new QVBoxLayout(pSettingsBox);
	pSettingsBox->setLayout(pSettingsLayout);
	pSettingsLayout->addWidget(pLengthLabel);
	pSettingsLayout->addWidget(pMaxLengthSpinBox);
	pSettingsLayout->addWidget(pTightConstraintCheckBox);


	QVBoxLayout* pInputLayout = new QVBoxLayout(pInputBox);
	pInputBox->setLayout(pInputLayout);
	pInputLayout->addWidget(pInputLabel);
	pInputLayout->addWidget(pInputEdit);

	// Main layout
	pMainLayout->addWidget(pTitle);
	pMainLayout->addWidget(pSettingsBox);
	pMainLayout->addWidget(pInputBox, 1);
	pMainLayout->addWidget(pAlignButton, 0, Qt::AlignRight);


	// Interactions
	connect(pInputEdit, &QTextEdit::textChanged, [=] {
		const bool empty = (pInputEdit->toPlainText().length() == 0);
		pAlignButton->setDisabled(empty);
	});

	connect(pAlignButton, &QPushButton::pressed, [=] {
		// Open a new window and send the settings
		AlignmentInfo info;
		info.maxLength = pMaxLengthSpinBox->value();
		info.tightConstraints = pTightConstraintCheckBox->isChecked();
		info.plainText = pInputEdit->toPlainText();

		AlignmentWindow* w = new AlignmentWindow(info);
		w->show();
	});

}

AlignmentWindow::AlignmentWindow(AlignmentInfo& info, QWidget* parent) : QWidget(parent)
{
	setWindowTitle("Computing Alignment...");

	// Create Layout
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	setLayout(pMainLayout);

	// Create group boxes
	QGroupBox* pInputBox = new QGroupBox("Input", this);
	QGroupBox* pOutputBox = new QGroupBox("Output", this);

	// Create input/output text 
	QTextEdit* pInputEdit = new QTextEdit(this);
	pInputEdit->setReadOnly(true);
	QTextEdit* pOutputEdit = new QTextEdit(this);

	// Create buttons
	QPushButton* pDoneButton = new QPushButton("Done", this);
	QPushButton* pCopyButton = new QPushButton("Copy", this);

	// Layout
	pInputBox->setLayout(new QVBoxLayout(this));
	pOutputBox->setLayout(new QVBoxLayout(this));
	pInputBox->layout()->addWidget(pInputEdit);
	pOutputBox->layout()->addWidget(pOutputEdit);
	pMainLayout->addWidget(pInputBox);
	pMainLayout->addWidget(pOutputBox);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(pCopyButton);
	buttonLayout->addWidget(pDoneButton);
	pMainLayout->addLayout(buttonLayout);

	// Parse the info
	QStringList seq = info.plainText.toUpper().split(',', Qt::SkipEmptyParts);

	// Keep only ACGTU
	std::vector<std::vector<char>> filteredInput;
	for (QString& it : seq) {
		std::vector<char> row;
		
		for (auto c : it) {
			if (c == 'A' || c == 'C' || c == 'G' || c == 'T' || c == 'U') {
				row.push_back(c.toLatin1());
			}
		}
		filteredInput.push_back(row);
	}

	// Build into a QString
	QString inputText;
	for (auto it : filteredInput) {
		for (auto c : it) {
			inputText += c;
		}
		inputText += "\n";
	}
	pInputEdit->setText(inputText);

	// TODO: Send input to MSA computer
	Input input;
	getInput(input);
}