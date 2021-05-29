#include "UserInterface.h"

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
#include <QMenuBar>
#include <QElapsedTimer>

UserInterface::UserInterface(QWidget *parent) : QMainWindow(parent)
{
	setWindowTitle("Multiple Sequence Alignment");

	// Build a grid layout for the settings
	QWidget* pCentralWidget = new QWidget(this);
	setCentralWidget(pCentralWidget);
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pCentralWidget->setLayout(pMainLayout);

	// Labels
	QLabel* pLengthLabel = new QLabel("Enter the maximum length of the alignment:", pCentralWidget);
	QLabel* pInputLabel = new QLabel("Enter sequences separated by commas:", pCentralWidget);
	QLabel* pTitle = new QLabel("Multiple Sequence Alignment", pCentralWidget);
	QFont f;
	f.setBold(true);
	f.setPointSize(20);
	pTitle->setFont(f);

	// Settings
	QSpinBox* pMaxLengthSpinBox = new QSpinBox(pCentralWidget);
	pMaxLengthSpinBox->setValue(10);
	pMaxLengthSpinBox->setRange(0, 999);
	pMaxLengthSpinBox->setToolTip("A smaller max length will give a more optimal solution.");
	QCheckBox* pTightConstraintCheckBox = new QCheckBox("Tight Constraints", pCentralWidget);
	pTightConstraintCheckBox->setToolTip("When enabled, creates much tighter constraints for the solver. Runs faster in most cases.");
	pTightConstraintCheckBox->setChecked(true);
	QTextEdit* pInputEdit = new QTextEdit(pCentralWidget);
	pInputEdit->setFontFamily("Courier New");

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

	// Menu bar
	QMenuBar* pMenuBar = new QMenuBar();
	setMenuBar(pMenuBar);

	// Add default options
	QMenu* pLoadMenu = pMenuBar->addMenu("Load");
	pLoadMenu->addAction("Small Example (< 1 sec)", [=] {
		pInputEdit->setText("AAAGT,\nAAGT,\nAAAT");
		pMaxLengthSpinBox->setValue(5);
		pTightConstraintCheckBox->setChecked(true);
	});
	pLoadMenu->addAction("UNSAT Example (< 1 sec)", [=] {
		pInputEdit->setText("AAAGTTGCAC,\nAAGTCCGTTC,\nAAATTTGGCG");
		pMaxLengthSpinBox->setValue(11);
		pTightConstraintCheckBox->setChecked(true);
	});
	pLoadMenu->addAction("Many Sequences (< 1 sec)", [=] {
		pInputEdit->setText("GCAGCCTGGCCAA,\nGCAGCCTGTGCAA,\nGCAGCCTGGGCAA,\nTCAGCCTGTGCAA,\nTCAGCCTGTGCAA");
		pMaxLengthSpinBox->setValue(16);
		pTightConstraintCheckBox->setChecked(true);
	});
	pLoadMenu->addAction("Long Example (< 10 sec)", [=] {
		pInputEdit->setText("ACTGGGTCGCCACAGGTCGGTGA,\nACTGCGTCCCCCGCAGGTCACTGA,\nACTGCGTCCCCGCAGGCCACTGA");
		pMaxLengthSpinBox->setValue(30);
		pTightConstraintCheckBox->setChecked(true);
	});
	pLoadMenu->addAction("Super Long 2 seq (< 20 sec)", [=] {
		pInputEdit->setText("CCCATCAGCGGCATCGCCACGTTCAAGGACACCTACGTCGCCACCGCCGGCTACGACAACCAGGTGATCCTCTGGGA,\nCCCATCAGCGGCGTCGCCGCCCACGAGGACTCCTACGTCGCCACCGCCGGTTACGACAACCACGTCATCCTCTGGGA");
		pMaxLengthSpinBox->setValue(86);
		pTightConstraintCheckBox->setChecked(true);
	});
	pLoadMenu->addAction("Extra Long Example (< 75 sec)", [=] {
		pInputEdit->setText("CAGCCTGGCCAACTTCCTCCTGCCCTTTGGCGACAGTGTGTTGA,\nCAGCCTGTGCAACTTCCTCGCGCCGGCGGGCGAGCTGATCCTGA,\nCAGCCTGTGCAACTTCCTCCTGCCCTTTGGCGACAGTGTGTTGA");
		pMaxLengthSpinBox->setValue(53);
		pTightConstraintCheckBox->setChecked(true);
	});

	// Fix a bug where this menu wouldn't display properly the first time
	pLoadMenu->show();
	pLoadMenu->hide();




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
	const QString title = QString("Alignment k=%1").arg(info.maxLength);
	setWindowTitle(title);

	// Create Layout
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	setLayout(pMainLayout);

	// Create group boxes
	QGroupBox* pInputBox = new QGroupBox("Input", this);
	QGroupBox* pOutputBox = new QGroupBox("Output", this);
	QGroupBox* pStatusBox = new QGroupBox();

	// Create input/output text 
	QTextEdit* pInputEdit = new QTextEdit(this);
	pInputEdit->setReadOnly(true);
	pInputEdit->setFontFamily("Courier New");
	QTextEdit* pOutputEdit = new QTextEdit(this);
	pOutputEdit->setReadOnly(true);
	pOutputEdit->setFontFamily("Courier New");

	// Create status text
	QLabel* pStatusLabel = new QLabel("Aligning...", pStatusBox);
	QFont f("Courier New");
	pStatusLabel->setFont(f);

	// Create buttons
	QPushButton* pDoneButton = new QPushButton("Cancel", this);
	QPushButton* pCopyButton = new QPushButton("Copy", this);
	pCopyButton->setDisabled(true);

	// Layout
	pInputBox->setLayout(new QVBoxLayout(this));
	pOutputBox->setLayout(new QVBoxLayout(this));
	pStatusBox->setLayout(new QVBoxLayout(this));
	pInputBox->layout()->addWidget(pInputEdit);
	pOutputBox->layout()->addWidget(pOutputEdit);
	pStatusBox->layout()->addWidget(pStatusLabel);
	pMainLayout->addWidget(pInputBox);
	pMainLayout->addWidget(pOutputBox);
	pMainLayout->addWidget(pStatusBox);

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
		if (row.size() > 0)
		{
			// Filter blank rows
			filteredInput.push_back(row);
		}
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


	// Create a new thread for computation
	Input input;
	input.rawInput = filteredInput;
	input.k = info.maxLength;
	input.tightConstraints = info.tightConstraints;

	WorkerThread* workerThread = new WorkerThread(input, this);
	connect(workerThread, &WorkerThread::resultReady, this, [=](Output output)
	{
		// Build output into a QString
		if (output.isSAT)
		{
			QString outputText;
			for (auto it : output.decodedOutput) {
				for (auto c : it) {
					outputText += c;
				}
				outputText += "\n";
			}

			pOutputEdit->setText(outputText);
			pCopyButton->setDisabled(false);
		}
		else {
			pOutputEdit->setText("No Alignment!");
		}

		// Generate a string to display the timing
		const double sec = (double)output.nsec * 1e-9;
		char buffer[32] = {};
		snprintf(buffer, 32, "Complete (%fs)", sec);
		pStatusLabel->setText(buffer);

		pDoneButton->setText("Done");

	}, Qt::QueuedConnection);
	connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
	workerThread->start();






	// Button interactions
	connect(pDoneButton, &QPushButton::pressed, [=] {
		close();
	});
	connect(pCopyButton, &QPushButton::pressed, [=] {
		pOutputEdit->selectAll();
		pOutputEdit->copy();
	});
}

// Run the MSA in another thread
void WorkerThread::run() 
{ 
	QElapsedTimer timer;
	timer.start();

	Output output = computeMSA(input);

	output.nsec = timer.nsecsElapsed();
	emit resultReady(output);
}