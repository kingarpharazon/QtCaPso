#pragma warning(push, 3)
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#pragma warning(pop)
#include <QMessageBox>
#include "controller.h"
#include "localcapso.h"
#include "localsettingsdialog.h"
#include "globalsettingsdialog.h"
#include "globalcapso.h"

Controller::Controller(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	mCurrentType(LOCAL),
	mCurrFileName("results.txt"),
	mResultsFile(mCurrFileName),
	mResultsStream(&mResultsFile),
	mTimerId(-1),
	mTimerCount(0),
	mSeasonLength(10)
{
	this->setupUi(this);

    createCa();
    initializeSettings();
	makeConnections();
	createView();
	createSettingsDialog();

	initializeResultsFile();
	writeResults();
}

Controller::~Controller()
{
	mResultsFile.close();

	delete mCellularAutomaton;
	delete mView;
}

void Controller::timerEvent(QTimerEvent*)
{
	mCellularAutomaton->nextGen();

	mTimerCount++;

	if(!(mTimerCount % mSeasonLength))
	{
		writeResults();
	}

	statusBarGeneration->showMessage(QString::number(mTimerCount));

	mView->update();
}

void Controller::play()
{
	if(mTimerId == -1)
	{
		mTimerId = startTimer(0);
	}
}

void Controller::pause()
{
	if(mTimerId != -1)
	{
		killTimer(mTimerId);

		mResultsStream.flush();

		mTimerId = -1;
	}
}

void Controller::step()
{
	if(mTimerId != -1)
	{
		killTimer(mTimerId);

		mTimerId = -1;
	}

	mCellularAutomaton->nextGen();

	mTimerCount++;

	if(!(mTimerCount % mSeasonLength))
	{
		writeResults();
	}

	statusBarGeneration->showMessage(QString::number(mTimerCount));

	mView->update();
}

void Controller::clear()
{
    if(mTimerId != -1)
    {
        killTimer(mTimerId);

        mTimerId = -1;
    }

	mCellularAutomaton->clear();

	mView->update();
}

void Controller::initialize()
{
	if(mTimerId != -1)
	{
		killTimer(mTimerId);

		mTimerId = -1;
	}

	mCellularAutomaton->initialize();

	mTimerCount = 0;

	initializeResultsFile();

	writeResults();

	mView->update();
}

void Controller::showSettings()
{
	mSettingsDialog->exec();
}

void Controller::exportBitmap()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Export Bitmap",
		QCoreApplication::applicationDirPath() + "/lattice.PNG", tr("PNG (*.PNG);;JPG (*.jpg);; TIFF (*.tiff)"));

	if(!fileName.isEmpty())
	{
		mView->latticeImage().save(fileName);
	}
}

void Controller::updateSettings(QMap<QString, QVariant> settings)
{
	switch(mCurrentType)
	{
	case GLOBAL:
		{
            auto global = dynamic_cast<GlobalCaPso*>(mCellularAutomaton);

            global->setInitialPreyPercentage(settings["initialNumberOfPreys"].toFloat());
            global->setCompetitionFactor(settings["competenceFactor"].toFloat());
            global->setCompetitionRadius(settings["competenceRadius"].toInt());
            global->setPreyReproductionRadius(settings["preyReproductionRadius"].toInt());
            global->setPreyMeanOffspring(settings["preyReproductiveCapacity"].toInt());

            global->setInitialSwarmSize(settings["initialNumberOfPredators"].toInt());
            global->setCognitiveFactor(settings["predatorCognitiveFactor"].toFloat());
            global->setSocialFactor(settings["predatorSocialFactor"].toFloat());
            global->setMaximumSpeed(settings["predatorMaximumSpeed"].toInt());
            global->setPredatorMeanOffspring(settings["predatorReproductiveCapacity"].toInt());
            global->setPredatorReproductionRadius(settings["predatorReproductionRadius"].toInt());
            global->setInitialInertialWeight(settings["initialInertiaWeight"].toFloat());
            global->setFinalInertiaWeight(settings["finalInertiaWeight"].toFloat());
		}
		break;

	case LOCAL:
		{
//			auto local = dynamic_cast<LocalCaPso*>(mCellularAutomaton);

//			local->setInitialAlivePreys(settings["initialNumberOfPreys"].toFloat());
//            local->setCompetitionFactor(settings["competenceFactor"].toFloat());
//			local->setPreyReproductionRadius(settings["preyReproductionRadius"].toInt());
//			local->setPreyMeanOffspring(settings["preyReproductiveCapacity"].toInt());

//			local->setInitialSwarmSize(settings["initialNumberOfPredators"].toInt());
//			local->setCognitiveFactor(settings["predatorCognitiveFactor"].toFloat());
//			local->setSocialFactor(settings["predatorSocialFactor"].toFloat());
//			local->setMaximumSpeed(settings["predatorMaximumSpeed"].toInt());
//			local->setPredatorMeanOffspring(settings["predatorReproductiveCapacity"].toInt());
//			local->setPredatorReproductionRadius(settings["predatorReproductionRadius"].toInt());
//			local->setSocialRadius(settings["predatorSocialRadius"].toInt());
//			local->setFitnessRadius(settings["fitnessRadius"].toInt());
//			local->setInitialInertialWeight(settings["initialInertiaWeight"].toFloat());
//			local->setFinalInertiaWeight(settings["finalInertiaWeight"].toFloat());
		}
		break;

	case MOVEMENT:
		break;
	}

	mCurrFileName = settings["resultsFilePath"].toString();

    auto local = dynamic_cast<LocalCaPso*>(mCellularAutomaton);

    QFile settingsFile;
    settingsFile.setFileName("settings.xml");

    if(!settingsFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Error!", "Cannot open file: " +
                              settingsFile.fileName());

        exit(1);
    }

    QXmlStreamReader reader(&settingsFile);

    while(!reader.atEnd() && !reader.hasError())
    {
        reader.readNext();

        if(reader.isStartElement())
        {
            QString elementName = reader.name().toString();

            if(elementName == "initialNumberOfPreys")
            {
                QString value = reader.readElementText();

                local->setInitialAlivePreys(value.toFloat());
            }
            else if(elementName == "competitionFactor")
            {
                QString value = reader.readElementText();

                local->setCompetitionFactor(value.toFloat());
            }
            else if(elementName == "preyReproductionRadius")
            {
                QString value = reader.readElementText();

                local->setPreyReproductionRadius(value.toInt());
            }
            else if(elementName == "preyReproductiveCapacity")
            {
                QString value = reader.readElementText();

                local->setPreyMeanOffspring(value.toInt());
            }
            else if(elementName == "fitnessRadius")
            {
                QString value = reader.readElementText();

                local->setFitnessRadius(value.toInt());
            }
            else if(elementName == "initialNumberOfPredators")
            {
                QString value = reader.readElementText();

                local->setInitialSwarmSize(value.toInt());
            }
            else if(elementName == "predatorCognitiveFactor")
            {
                QString value = reader.readElementText();

                local->setCognitiveFactor(value.toFloat());
            }
            else if(elementName == "predatorSocialFactor")
            {
                QString value = reader.readElementText();

                local->setSocialFactor(value.toFloat());
            }
            else if(elementName == "predatorMaximumSpeed")
            {
                QString value = reader.readElementText();

                local->setMaximumSpeed(value.toInt());
            }
            else if(elementName == "predatorReproductiveCapacity")
            {
                QString value = reader.readElementText();

                local->setPredatorMeanOffspring(value.toInt());
            }
            else if(elementName == "predatorReproductionRadius")
            {
                QString value = reader.readElementText();

                local->setPredatorReproductionRadius(value.toInt());
            }
            else if(elementName == "predatorSocialRadius")
            {
                QString value = reader.readElementText();

                local->setSocialRadius(value.toInt());
            }
            else if(elementName == "initialInertiaWeight")
            {
                QString value = reader.readElementText();

                local->setInitialInertialWeight(value.toFloat());
            }
            else if(elementName == "finalInertiaWeight")
            {
                QString value = reader.readElementText();

                local->setFinalInertiaWeight(value.toFloat());
            }
            else if(elementName == "resultsFilePath")
            {
                mCurrFileName = reader.readElementText();
            }
        }
    }

    if(reader.hasError())
    {
        QMessageBox::critical(this, "Error reading settings file",
                              reader.errorString());
    }

    settingsFile.close();
}

void Controller::initializeSettings()
{
    switch(mCurrentType)
    {
    case GLOBAL:
        break;
    case LOCAL:
        // Insert prey data
//        mSettings.insert("initialNumberOfPreys", 0.3f);
//        mSettings.insert("competenceFactor", 0.3f);
//        mSettings.insert("preyReproductionRadius", 2);
//        mSettings.insert("preyReproductiveCapacity", 10);
//        mSettings.insert("fitnessRadius", 3);

        // Insert predator data
//        mSettings.insert("initialNumberOfPredators", 3);
//        mSettings.insert("predatorCognitiveFactor", 1.0f);
//        mSettings.insert("predatorSocialFactor", 2.0f);
//        mSettings.insert("predatorMaximumSpeed", 10);
//        mSettings.insert("predatorReproductiveCapacity", 10);
//        mSettings.insert("predatorReproductionRadius", 2);
//        mSettings.insert("predatorSocialRadius", 3);
//        mSettings.insert("initialInertiaWeight", 0.9f);
//        mSettings.insert("finalInertiaWeight", 0.2f);

        // Insert path to results file
//        mSettings.insert("resultsFilePath", QCoreApplication::applicationDirPath() +
//                         "/results.txt");
        break;
    case MOVEMENT:
        break;
    }

    if(!QFile::exists("settings.xml"))
    {
        QFile settingsFile;
        settingsFile.setFileName("settings.xml");
        settingsFile.open(QIODevice::WriteOnly);

        QXmlStreamWriter writer(&settingsFile);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("project");
        writer.writeAttribute("type", "local");
        writer.writeTextElement("initialNumberOfPreys", "0.3");
        writer.writeTextElement("competitionFactor", "0.3");
        writer.writeTextElement("preyReproductionRadius", "2");
        writer.writeTextElement("preyReproductiveCapacity", "10");
        writer.writeTextElement("fitnessRadius", "3");
        writer.writeTextElement("initialNumberOfPredators", "3");
        writer.writeTextElement("predatorCognitiveFactor", "1.0");
        writer.writeTextElement("predatorSocialFactor", "2.0");
        writer.writeTextElement("predatorMaximumSpeed", "10");
        writer.writeTextElement("predatorReproductiveCapacity", "10");
        writer.writeTextElement("predatorReproductionRadius", "2");
        writer.writeTextElement("predatorSocialRadius", "3");
        writer.writeTextElement("initialInertiaWeight", "0.9");
        writer.writeTextElement("finalInertiaWeight", "0.2");
        writer.writeTextElement("resultsFilePath",
                                QCoreApplication::applicationDirPath() +
                                "/results.txt");
        writer.writeEndElement();
        writer.writeEndDocument();

        settingsFile.close();
    }

    updateSettings(mSettings);
}

void Controller::makeConnections()
{
	connect(actionPlay, SIGNAL(triggered()), this, SLOT(play()));
	connect(actionPause, SIGNAL(triggered()), this, SLOT(pause()));
	connect(actionStep, SIGNAL(triggered()), this, SLOT(step()));
	connect(actionClear, SIGNAL(triggered()), this, SLOT(clear()));
	connect(actionInitialize, SIGNAL(triggered()), this, SLOT(initialize()));
	connect(actionSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
	connect(actionExportBitmap, SIGNAL(triggered()), this, SLOT(exportBitmap()));
}

void Controller::createCa()
{
	switch(mCurrentType)
	{
	case GLOBAL:
		mCellularAutomaton = new GlobalCaPso(512, 256);
		mSeasonLength = 10;
		break;

	case LOCAL:
		mCellularAutomaton = new LocalCaPso(512, 256);
		mSeasonLength = 10;
		break;

	case MOVEMENT:
		break;
	}
}

void Controller::createView()
{
    mView = new CaView(mCellularAutomaton->latticeData(),
                       mCellularAutomaton->width(),
                       mCellularAutomaton->height(),
                       this);

	setCentralWidget(mView);
}

void Controller::createSettingsDialog()
{
	switch(mCurrentType)
	{
	case GLOBAL:
		{
			mSettingsDialog = new GlobalSettingsDialog(this);
			auto p = dynamic_cast<GlobalSettingsDialog*>(mSettingsDialog);
			connect(p, SIGNAL(settingsChanged(QMap<QString, QVariant>)),
				this, SLOT(updateSettings(QMap<QString, QVariant>)));
		}
		break;

	case LOCAL:
		{
            mSettingsDialog = new LocalSettingsDialog(mSettings, this);
			auto p = dynamic_cast<LocalSettingsDialog*>(mSettingsDialog);
			connect(p, SIGNAL(settingsChanged(QMap<QString, QVariant>)),
				this, SLOT(updateSettings(QMap<QString, QVariant>)));
		}
		break;

	case MOVEMENT:
		break;
	}
}

void Controller::initializeResultsFile()
{
	mResultsFile.close();
	mResultsFile.setFileName(mCurrFileName);
	mResultsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	mResultsStream.seek(0);
}

void Controller::writeResults()
{
	switch(mCurrentType)
	{
	case LOCAL:
		{
			auto local = dynamic_cast<LocalCaPso*>(mCellularAutomaton);
            mResultsStream << mTimerCount / mSeasonLength << " " <<
                              local->numberOfPreys() << " " <<
                              local->numberOfPredators() << "\n";
		}
		break;
	}
}
