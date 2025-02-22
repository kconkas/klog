/***************************************************************************tv
                          mainqsoentrywidget.cpp  -  description
                             -------------------
    begin                : january 2020
    copyright            : (C) 2020 by Jaime Robles
    email                : jaime@robles.es
 ***************************************************************************/

/*****************************************************************************
 * This file is part of KLog.                                                *
 *                                                                           *
 *    KLog is free software: you can redistribute it and/or modify           *
 *    it under the terms of the GNU General Public License as published by   *
 *    the Free Software Foundation, either version 3 of the License, or      *
 *    (at your option) any later version.                                    *
 *                                                                           *
 *    KLog is distributed in the hope that it will be useful,                *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *    GNU General Public License for more details.                           *
 *                                                                           *
 *    You should have received a copy of the GNU General Public License      *
 *    along with KLog.  If not, see <https://www.gnu.org/licenses/>.         *
 *                                                                           *
 *****************************************************************************/
//https://github.com/ea4k/klog/commit/1ac02c7b779922a8e5907ebee088fa64654cbdf5
#include "mainqsoentrywidget.h"

MainQSOEntryWidget::MainQSOEntryWidget(DataProxy_SQLite *dp, QWidget *parent) : QWidget(parent)
{
      //qDebug()<< "MainQSOEntryWidget::MainQSOEntryWidget " << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);

    upAndRunning = false;
    dataProxy = dp;
    qrzLineEdit = new QLineEdit;
    bandComboBox = new QComboBox;
    modeComboBox = new QComboBox;
    dateEdit = new QDateEdit;
    timeEdit = new QTimeEdit;
    realtimeCheckBox = new QCheckBox;
    enabledCR = realtimeCheckBox->backgroundRole();
    //realtimeButton = new QPushButton;
    //realtimeButton->setCheckable(true);
    //realtimeButton->setFixedSize(QSize(28,28));
    //realtimeButton->setIcon(QIcon(":/img/play.svg"));
    //enabledCR = realtimeButton->backgroundRole();
    OKButton = new QPushButton(tr("&Add"), this);
    clearButton = new QPushButton(tr("&Clear"), this);
    timer = new QTimer(this);
    util = new Utilities;
    realTime = true;
    duplicatedQSOSlotInSecs = 0;
    delayInputTimer = new QTimer;


    createUI();
    setInitialData();

    emit debugLog(Q_FUNC_INFO, "END", Debug);
      //qDebug()<< "MainQSOEntryWidget::MainQSOEntryWidget: - END" << endl;
}

/*
void MainQSOEntryWidget::slotRealTimeButtonResize()
{
    int heigh = qrzLineEdit->sizeHint ().height ();
    realtimeButton->setFixedSize(QSize(heigh, heigh));
    //realtimeButton->setSizePolicy (QSizePolicy::Fixed);

}
*/
void MainQSOEntryWidget::createUI()
{
      //qDebug()<< "MainQSOEntryWidget::createUI" << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    qrzLineEdit->setToolTip(tr("Callsign of the QSO."));
    bandComboBox->setToolTip(tr("Band of the QSO."));
    modeComboBox->setToolTip(tr("Mode of the QSO."));
    dateEdit->setToolTip(tr("Date of the QSO."));
    timeEdit->setToolTip(tr("Time of the QSO."));
    OKButton->setToolTip(tr("Add the QSO to the log."));
    clearButton->setToolTip(tr("Clears the QSO entry."));
    realtimeCheckBox->setToolTip(tr("KLog will show real time if enabled."));
    realtimeCheckBox->setText (tr("Real time"));
    //realtimeButton->setToolTip(tr("KLog will show real time if enabled."));
    QHBoxLayout *TimeLayout = new QHBoxLayout;
    TimeLayout->addWidget(dateEdit);
    TimeLayout->addWidget(timeEdit);
    //TimeLayout->addWidget(realtimeButton);
    TimeLayout->addWidget(realtimeCheckBox);
    TimeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

    QHBoxLayout *BandModeLayout = new QHBoxLayout;
    BandModeLayout->addWidget(bandComboBox);
    BandModeLayout->addWidget(modeComboBox);

    QHBoxLayout *QrzBandModeLayout = new QHBoxLayout;
    QrzBandModeLayout->addWidget(qrzLineEdit);
    QrzBandModeLayout->addLayout(BandModeLayout);

    qrzgroupBox = new QGroupBox(tr("Callsign"));
    qrzgroupBox->setFlat(true);
    QVBoxLayout *qrzvbox = new QVBoxLayout;
    qrzvbox->addLayout(QrzBandModeLayout);
    qrzgroupBox->setLayout(qrzvbox);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(OKButton);
    buttonsLayout->addWidget(clearButton);

    dateEdit->setDisplayFormat("yyyy-MM-dd");
    timeEdit->setDisplayFormat("HH:mm:ss");

    QGridLayout *widgetLayout = new QGridLayout;
    widgetLayout->addWidget(qrzgroupBox, 0, 0, 1, 0);
    widgetLayout->addLayout(TimeLayout, 1, 0);
    widgetLayout->addLayout(buttonsLayout,1, 1);

    setLayout(widgetLayout);

    palRed.setColor(QPalette::Text, Qt::red);
    palBlack.setColor(QPalette::Text, Qt::black);
    palWhite.setColor(QPalette::Text, Qt::white);

    connect(qrzLineEdit, SIGNAL(returnPressed()), this, SLOT(slotOKButtonClicked() ) );
    //connect(qrzLineEdit, SIGNAL(res), this, SLOT(slotOKButtonClicked() ) );
    //connect(qrzLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotQRZTextChanged() ) );
    connect(qrzLineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotStartDelayInputTimer() ) );
    connect(delayInputTimer, SIGNAL(timeout()), this, SLOT(slotDelayInputTimedOut() ) );

    connect(bandComboBox, SIGNAL(currentIndexChanged (QString)), this, SLOT(slotBandComboBoxChanged(QString) ) ) ;
    connect(modeComboBox, SIGNAL(currentIndexChanged (QString)), this, SLOT(slotModeComboBoxChanged(QString) ) ) ;

    connect(OKButton, SIGNAL(clicked()), this, SLOT(slotOKButtonClicked() ) );

    connect(clearButton, SIGNAL(clicked()), this, SLOT(slotClearButtonClicked() ) );
    connect(timer, SIGNAL(timeout()), this, SLOT(slotUpdateTime()) );
    //connect(realtimeButton, SIGNAL(clicked()), this, SLOT(slotRealtimeButtonClicked()) );
    connect(realtimeCheckBox, SIGNAL(clicked()), this, SLOT(slotCheckBoxClicked()));
      //qDebug()<< "MainQSOEntryWidget::createUI-END" << endl;

    emit debugLog(Q_FUNC_INFO, "END", Debug);

}

/*
void MainQSOEntryWidget::resizeEvent(QResizeEvent *event)
{
    //qDebug() << Q_FUNC_INFO;
    slotRealTimeButtonResize();

}
*/


void MainQSOEntryWidget::slotCheckBoxClicked()
{
   //qDebug() << Q_FUNC_INFO;
    if (realtimeCheckBox->isChecked())
    {
        realTime = true;
        //realtimeButton->setIcon(QIcon(":/img/play.svg"));
        timeEdit->setBackgroundRole(enabledCR);
    }
    else
    {
        realTime = false;
        //realtimeButton->setIcon(QIcon(":/img/stop.svg"));
        timeEdit->setBackgroundRole(QPalette::BrightText);
    }
}

/*
void MainQSOEntryWidget::slotRealtimeButtonClicked()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);

    if (realtimeButton->isChecked())
    {
        realTime = true;
        realtimeButton->setIcon(QIcon(":/img/stop.svg"));
        timeEdit->setBackgroundRole(enabledCR);
    }
    else
    {
        realTime = false;
        realtimeButton->setIcon(QIcon(":/img/play.svg"));
        timeEdit->setBackgroundRole(QPalette::BrightText);
    }

    emit debugLog(Q_FUNC_INFO, "END", Debug);
}
*/
void MainQSOEntryWidget::setCleaning (const bool _c)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    cleaning = _c;
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::slotQRZTextChanged()
{
    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: " << qrzLineEdit->text() << " / Length: " << QString::number((qrzLineEdit->text()).size()) << "###### START ######" << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);

    if ((qrzLineEdit->text()).length()<1)
    {
        //emit clearForNextQSOSignal();
        //qDebug() << Q_FUNC_INFO;
        slotClearButtonClicked();
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: QRZ <1 - END" << endl;
    return;
    }
    int cursorP = qrzLineEdit->cursorPosition();
    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursor position: " << QString::number(cursorP) << endl;
    qrzLineEdit->setText((qrzLineEdit->text()).toUpper());
    if (cleaning)
    {
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: Cleaning - END" << endl;
        emit debugLog(Q_FUNC_INFO, "END-2", Debug);
        return;
    }

    if (qrzAutoChanging)
    {
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: qrzAutoChanging - END" << endl;
        qrzAutoChanging = false;
        emit debugLog(Q_FUNC_INFO, "END-3", Debug);
        return;
    }

    qrzAutoChanging = true;

   //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursor position.1: " << QString::number(cursorP) << endl;

    if ( (qrzLineEdit->text()).endsWith(' ') )
    {/*Remove the space and moves the focus to SRX to write the RX exchange*/
        previousQRZ = (qrzLineEdit->text()).simplified();
        qrzLineEdit->setText(previousQRZ);
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: Space detected" << endl;
    }

    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: Simplifiying & Capitalizing" << endl;
    qrzLineEdit->setText(((qrzLineEdit->text())).simplified());
    qrzLineEdit->setText((qrzLineEdit->text()).remove(" "));

    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: checking for invalid chars" << endl;
    //TODO: This validCharactersInCall may be removed?
    InValidCharsInPrevCall = validCharactersInCall(qrzLineEdit->text());
    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: checking for invalid chars 00 " << endl;
    if (!util->isValidCall(qrzLineEdit->text()))
    {
        qrzLineEdit->setPalette(palRed);
        //emit showInfoLabel(tr("Callsign not valid"));
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: QRZ not valid - Red" << endl;
        emit debugLog(Q_FUNC_INFO, "END-4", Debug);
    }
    else
    {
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: QRZ is valid - Black" << endl;
        if (getDarkMode())
        {
            qrzLineEdit->setPalette(palWhite);
        }
        else
        {
            qrzLineEdit->setPalette(palBlack);
        }

        currentQrz = qrzLineEdit->text();
        //emit showInfoLabel(tr(""));
    }
    /*
      //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: checking for length" << endl;
    if (((qrzLineEdit->text()).length() < 1))
    { // If QRZ box is blank, Information labels should be cleared.
        slotClearButtonClicked();
        emit debugLog(Q_FUNC_INFO, "END-5", Debug);
        return;
    }
    */

    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: checking for modify or length<1" << endl;
    if (qrzSmallModDontCalculate)
    //if ((modify) || ((qrzLineEdit->text()).length() < 1) || (qrzSmallModDontCalculate))
    {
        //qDebug() << "MainQSOEntryWidget::slotQRZTextChanged: qrzSmallModDontCalculate < 1" << endl;
        qrzSmallModDontCalculate=false;
        emit debugLog(Q_FUNC_INFO, "END-6", Debug);
        return;
    }

    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: running..." << endl;
    qrzSmallModDontCalculate = true; // A kind of flag to prevent multiple calls to this method.
    currentQrz = qrzLineEdit->text();

    if ((currentQrz).count('\\'))
    { // Replaces \ by / to ease operation.
        //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: Replacing \\ by /" << endl;
        currentQrz.replace(QChar('\\'), QChar('/'));
        qrzLineEdit->setText(currentQrz);
    }

    currentQrz = qrzLineEdit->text();
    //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursor position.3: " << QString::number(cursorP) << endl;
    if (cursorP>currentQrz.length())
    {// A Space that has been removed without updating the cursor
         //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursorP > currentQRZ.length" << endl;
    }
    else
    {
        if (cursorP==0)
        {

        }
        else if ((currentQrz.at(cursorP-1)).isSpace())
        {
            //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursor position.5: " << QString::number(cursorP) << endl;
            previousQRZ = currentQrz.remove(cursorP-1, 1);
            //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursor position.6: " << QString::number(cursorP) << endl;
            cursorP--;
            //qDebug()<< "MainQSOEntryWidget::slotQRZTextChanged: cursor position.7: " << QString::number(cursorP) << endl;
            qrzLineEdit->setText(previousQRZ);
        }
    }

    currentQrz = qrzLineEdit->text();
    //qDebug() << "MainQSOEntryWidget::slotQRZTextChanged: Emitting: " << currentQrz << endl;
    emit currentQRZSignal(currentQrz);

    qrzSmallModDontCalculate = false; // If the text has not been modified in this method
    //qDebug() << "MainQSOEntryWidget::slotQRZTextChanged: cursorP at the end : " << QString::number(cursorP) << endl;
    qrzLineEdit->setCursorPosition(cursorP);
    checkIfDupe(Q_FUNC_INFO);
    qrzAutoChanging = false;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);

    //qDebug() << "MainQSOEntryWidget::slotQRZTextChanged: END" << endl;
}

void MainQSOEntryWidget::setCurrentQRZ(const QString &_qrz)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    currentQrz = _qrz;
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::slotBandComboBoxChanged(const QString _b){
    //qDebug() << Q_FUNC_INFO << ": " << _b;

    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    bottomBandLimit = dataProxy->getLowLimitBandFromBandName (_b);
    upperBandLimit = dataProxy->getUpperLimitBandFromBandName (_b);
    emit bandChanged(_b);
    checkIfDupe(Q_FUNC_INFO);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    //qDebug() << Q_FUNC_INFO << " - END";
}

void MainQSOEntryWidget::slotModeComboBoxChanged(const QString _m)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);

    emit modeChanged(_m);
    checkIfDupe(Q_FUNC_INFO);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::slotOKButtonClicked()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if ((qrzLineEdit->text()).length()<1)
    {
        return;
    }
    emit OKClicked();
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::slotClearButtonClicked()
{
    //qDebug() << "MainQSOEntryWidget::slotClearButtonClicked" << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    clear();
    emit clearForNextQSOSignal();
    setModify(false);
    checkIfDupe(Q_FUNC_INFO);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::clear()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);

    //qDebug() << Q_FUNC_INFO << endl;

    //cleaning = true;

    OKButton->setText(tr("&Add"));
    qrzLineEdit->clear();
    qrzLineEdit->setFocus(Qt::OtherFocusReason);

    //cleaning = false;
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::setInitialData()
{
      //qDebug()<< "MainQSOEntryWidget::setInitialData" << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //Default band/modes
    modify = false;
    qrzAutoChanging = false;
    InValidCharsInPrevCall = false;
    qrzSmallModDontCalculate=false;
    previousQRZ = QString();

    bands.clear();
    modes.clear();
    bands << "10M" << "15M" << "20M" << "40M" << "80M" << "160M";
    modes << "SSB" << "CW" << "RTTY";


    setBands(bands);
    setModes(modes);

    selectDefaultBand(true);
    selectDefaultMode(true);

    dateEdit->setDate(QDate::currentDate());
    timeEdit->setTime(QTime::currentTime());

    UTCTime = true;
    // //qDebug()ime = true;

    timer->start(1000);

    emit debugLog(Q_FUNC_INFO, "END", Debug);
     //qDebug()<< "MainQSOEntryWidget::setInitialData-END" << endl;

}

bool MainQSOEntryWidget::updateBandComboBox(const QString &_band)
{
    //qDebug() << Q_FUNC_INFO << ": " << _band << endl;

    //QString _currentBand = getBand();
    if (!isBandExisting(_band))
    {// The selected frequency is of a band that is not currently selected
        QString _currentBand = bandComboBox->currentText ();
        //qDebug() << Q_FUNC_INFO << ":  New band found: " << _band << endl;
        if (dataProxy->getIdFromBandName(_band) > 1)
        {// Not affected if 0 (light) is the frequency
         // In this case the user should select the band in the setup
            //qDebug() << Q_FUNC_INFO << ":  Band is valid: " << _band << endl;
            QStringList qsTemp;
            qsTemp.clear();
            qsTemp << bands;
            qsTemp << _band;
            qsTemp.removeDuplicates();
            bands.clear();
            bands = dataProxy->sortBandNamesBottonUp(qsTemp);
            //qDebug() << Q_FUNC_INFO << ": Before setBands" << endl;
            setBands(bands);
            bandComboBox->setCurrentIndex(bandComboBox->findText(_currentBand, Qt::MatchCaseSensitive));
            //qDebug() << Q_FUNC_INFO << ": Band has been added : " << _band << endl;
        }
        else
        {
            //qDebug() << Q_FUNC_INFO << ": (END) Band is NOT  valid: " <<_band<< endl;
            return false;
        }
    }


    //qDebug() << Q_FUNC_INFO << ": Band already existing, no need to add"  << endl;
    return true;




}

void MainQSOEntryWidget::setBands(const QStringList _bands)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //qDebug()<< "MainQSOEntryWidget::setBands" << endl;
    foreach(QString i, _bands)
    {
        //qDebug()<< "MainQSOEntryWidget::setBands - received: " << i << endl;
    }

    bands.clear();
    bands = _bands;
    bands.removeDuplicates();
    bands = dataProxy->sortBandNamesBottonUp(bands);
    bandComboBox->clear();
    bandComboBox->addItems(bands);
    emit validBands(_bands);
    //selectDefaultBand(true);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    //qDebug()<< "MainQSOEntryWidget::setBands-END" << endl;
}

void MainQSOEntryWidget::setModes(const QStringList _modes)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    modes.clear();
    modes = _modes;
    modes.removeDuplicates();
    modes.sort();
    modeComboBox->clear();
    modeComboBox->addItems(modes);
    selectDefaultMode(true);
    emit debugLog(Q_FUNC_INFO, "END", Debug);

}

QStringList MainQSOEntryWidget::getModes()
{
    return modes;
}

bool MainQSOEntryWidget::setFreq(const double _f, bool isRX)
{
    //qDebug() << Q_FUNC_INFO << ": " << QString::number(_f);

    if (isRX)
    {
        if (util->isSameFreq (freqRX, _f))
        {
            return true;
        }
        freqRX = _f;
    }

    if (util->isSameFreq (freqTX, _f))
    {
        return true;
    }
    freqTX = _f;

    if (newBandNeededForFreq (_f))
    {
        if ((bottomBandLimit<=freqTX) && (freqTX<= upperBandLimit))
        {
            return true;
        }
        //qDebug() << Q_FUNC_INFO << ": Freq is not in the current band" << endl;
        QString _newBand = dataProxy->getBandNameFromFreq(_f);
        //qDebug() << Q_FUNC_INFO << ": before setting band: " << _newBand  << endl;
        if (isRX)
        {
            //qDebug() << Q_FUNC_INFO << ": RX Freq no more actions "  << endl;
            return true;
        }
        return setBand(_newBand);

    }
    return false;
}

bool MainQSOEntryWidget::newBandNeededForFreq(const double _f)
{
    //qDebug() << Q_FUNC_INFO << ": " << QString::number(_f);
    QString _newBand = dataProxy->getBandNameFromFreq(_f);
    if (!updateBandComboBox (_newBand))
    {
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
        //qDebug() << Q_FUNC_INFO << " - END false";
        return false;
    }
    //qDebug() << Q_FUNC_INFO << " - END true ";
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    return true;
}

bool MainQSOEntryWidget::setBand(const QString &_band)
{
    //qDebug() << Q_FUNC_INFO << ": " << _band << endl;

    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if (bandComboBox->findText(_band, Qt::MatchCaseSensitive) < 0)
    {
        //qDebug() << "MainQSOEntryWidget::setBand-1: Band not found " << _band << endl;
        if (!updateBandComboBox(_band))
        {
            emit debugLog(Q_FUNC_INFO, "END-1", Debug);
            return false;
        }
    }
    //qDebug() << "MainQSOEntryWidget::setBand-1: Band found " << _band << endl;
    bandComboBox->setCurrentIndex(bandComboBox->findText(_band, Qt::MatchCaseSensitive));
    emit debugLog(Q_FUNC_INFO, "END-2", Debug);
    //qDebug() << Q_FUNC_INFO << " - END"  << endl;
    return true;
}

bool MainQSOEntryWidget::setMode(const QString &_mode)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //qDebug() << "MainQSOEntryWidget::setMode: " << _mode << endl;
    if (modeComboBox->findText(_mode, Qt::MatchCaseSensitive) < 0)
    {
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
        return false;
    }
    else
    {
        modeComboBox->setCurrentIndex(modeComboBox->findText(_mode, Qt::MatchCaseSensitive));
        emit debugLog(Q_FUNC_INFO, "END", Debug);
       return true;
    }
}

bool MainQSOEntryWidget::setQRZ(const QString &_qrz)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //qDebug() << "MainQSOEntryWidget::setQRZ: " << _qrz << endl;
    //TODO: Add validations to prevent that non valid qrz are sent from the outside of this function or at least manage this appropriately.
    qrzLineEdit->setText(_qrz.toUpper());
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    return false;
}

bool MainQSOEntryWidget::setDateTime(const QDateTime _date)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if (_date.isValid())
    {
        dateEdit->setDate(_date.date());
        timeEdit->setTime(_date.time());

        emit debugLog(Q_FUNC_INFO, "END", Debug);
        return true;
    }
    else
    {
        //qDebug() << "MainQSOEntryWidget::setDate - NO VALID DATE" << endl;
        emit debugLog(Q_FUNC_INFO, "END", Debug);
        return false;
    }
}

bool MainQSOEntryWidget::setTime(const QTime _time)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if (_time.isValid())
    {
        timeEdit->setTime(_time);
        emit debugLog(Q_FUNC_INFO, "END", Debug);
        return true;
    }
    else
    {
        emit debugLog(Q_FUNC_INFO, "END", Debug);
        return false;
    }
}

QString MainQSOEntryWidget::getQrz()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    return (qrzLineEdit->text()).toUpper();
}

QString MainQSOEntryWidget::getBand(const int _b)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if (_b<0)
    {
        emit debugLog(Q_FUNC_INFO, "END", Debug);
        //qDebug() << "MainQSOEntryWidget::getBand(1): " << bandComboBox->currentText()<< endl;
        return bandComboBox->currentText();
    }
    else
    {
        if (bandComboBox->count()>=_b)
        {
            emit debugLog(Q_FUNC_INFO, "END-1", Debug);
            //qDebug() << "MainQSOEntryWidget::getBand(2): " << bandComboBox->currentText()<< endl;
            return bandComboBox->itemText(_b);
        }
        else
        {
            emit debugLog(Q_FUNC_INFO, "END-2", Debug);
            //qDebug() << "MainQSOEntryWidget::getBand(3): " << bandComboBox->currentText()<< endl;
            return bandComboBox->currentText();
        }
    }
}

QString MainQSOEntryWidget::getMode(const int _m)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if (_m<0)
    {
        emit debugLog(Q_FUNC_INFO, "END", Debug);
        return modeComboBox->currentText();
    }
    else
    {
        if (modeComboBox->count()>=_m)
        {
            emit debugLog(Q_FUNC_INFO, "END-1", Debug);
            return modeComboBox->itemText(_m);
        }
        else
        {
            emit debugLog(Q_FUNC_INFO, "END-2", Debug);
            return modeComboBox->currentText();
        }
    }
}

QDate MainQSOEntryWidget::getDate()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    return dateEdit->date();
}


QDateTime MainQSOEntryWidget::getDateTime()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //emit debugLog(Q_FUNC_INFO, "END", Debug);
    QDateTime dateTime;
    dateTime.setDate(dateEdit->date());
    dateTime.setTime(timeEdit->time());
    return dateTime;

}

QTime MainQSOEntryWidget::getTime()
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    return timeEdit->time();
}

void MainQSOEntryWidget::toggleRealTime()
{
    //if ( realtimeButton->isChecked ())
    if ( realtimeCheckBox->isChecked ())
    {
        setRealTime (false);
    }
    else {
        setRealTime (true);
    }
}

bool MainQSOEntryWidget::getRealTime()
{
    //return realtimeButton->isChecked ();
    return realtimeCheckBox->isChecked ();

}

void MainQSOEntryWidget::setRealTime(const bool _realTime)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //qDebug << Q_FUNC_INFO << ": " << util->boolToQString (_realTime);

    realTime = _realTime;
    realtimeCheckBox->setChecked(realTime);
    /*
     * if (_realTime)
    {
        realtimeButton->setIcon(QIcon(":/img/play.svg"));
    }
    else
    {
        realtimeButton->setIcon(QIcon(":/img/stop.svg"));
    }
    */
    //realTime = _realTime;
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}


void MainQSOEntryWidget::setUTC(const bool _utc)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    UTCTime = _utc;
    setDateAndTimeInternally();
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::setModify(const bool _modify)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    modify = _modify;

    if (modify)
    {
        OKButton->setText(tr("&Modify"));
        realtimeCheckBox->setChecked (false);
    }
    else
    {
        OKButton->setText(tr("&Add"));
    }
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

bool MainQSOEntryWidget::getModifying()
{
    return modify;
}

void MainQSOEntryWidget::slotUpdateTime()
{
    //qDebug()<< "MainQSOEntryWidget::slotUpdateTime" << endl;
    //emit debugLog(Q_FUNC_INFO, "Start", Debug);

    //if ( (!modify) && (realtimeButton->isChecked())  )
    if ( (!modify) && (realtimeCheckBox->isChecked())  )
    {
        //qDebug()<< "MainQSOEntryWidget::slotUpdateTime - Real Time & update" << endl;
        setDateAndTimeInternally();
    }

    //emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::setDateAndTimeInternally()
{
    if (UTCTime)
    {
        dateEdit->setDate(QDateTime::currentDateTime().toUTC().date());
        timeEdit->setTime(QDateTime::currentDateTime().toUTC().time());
    }
    else
    {
        dateEdit->setDate(QDateTime::currentDateTime().date());
        timeEdit->setTime(QDateTime::currentDateTime().time());
    }
}

bool MainQSOEntryWidget::validCharactersInCall(const QString &_qrz)
{
    //qDebug()<< "MainQSOEntryWidget::validCharactersInCall: " << _qrz << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    for (int i = 0; i<_qrz.size()-1;i++)
    {
        if (!( ((_qrz.at(i)).isLetterOrNumber()) || (_qrz.at(i)=='\\')  || (_qrz.at(i)=='/')  ))
        {
            emit debugLog(Q_FUNC_INFO, "END-1", Debug);
            return false;
        }

    }
    emit debugLog(Q_FUNC_INFO, "END", Debug);
    return true;
}

bool MainQSOEntryWidget::isModeExisting(const QString &_m)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if (modeComboBox->findText(_m, Qt::MatchCaseSensitive) >= 0)
    {
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
        return true;
    }
    else
    {
        emit debugLog(Q_FUNC_INFO, "END-2", Debug);
        return false;
    }

}

bool MainQSOEntryWidget::isBandExisting(const QString &_b)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    //qDebug() << Q_FUNC_INFO << ": " << _b << endl;
    if (bandComboBox->findText(_b, Qt::MatchCaseSensitive) >= 0)
    {
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
        //qDebug() << Q_FUNC_INFO << " - END true" << endl;
        return true;
    }
    else
    {
        emit debugLog(Q_FUNC_INFO, "END-2", Debug);
        //qDebug() << Q_FUNC_INFO << " - END false" << endl;
        return false;
    }
}

void MainQSOEntryWidget::setUpAndRunning(const bool _u)
{
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    upAndRunning = _u;
    emit debugLog(Q_FUNC_INFO, "END", Debug);
}

void MainQSOEntryWidget::selectDefaultBand(const bool _init)
{
      //qDebug() << "MainQSOEntryWidget::selectDefaultBand" << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if ((upAndRunning) || (!_init))
    {
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
          //qDebug() << "MainQSOEntryWidgetselectDefaultBand-END-1" << endl;
        return;
    }
    QString aux;
    aux = QString();
    int defaultBand = dataProxy->getMostUsedBand(-1); //TODO: The log could be defined here
      //qDebug() << "MainQSOEntryWidget::selectDefaultBand: " << QString::number(defaultBand) << dataProxy->getNameFromBandId (defaultBand)<< endl;
    if (defaultBand<1)
    {

        defaultBand = dataProxy->getIdFromBandName(getBand(0));

    }

    aux = dataProxy->getNameFromBandId(defaultBand);
    //qDebug() << Q_FUNC_INFO << ": before setting band: " << aux << endl;
    setBand(aux);
    emit debugLog(Q_FUNC_INFO, "END", Debug);

      //qDebug() << "MainQSOEntryWidgetselectDefaultBand_END" << endl;
}

void MainQSOEntryWidget::selectDefaultMode(const bool _init)
{
      //qDebug() << "MainQSOEntryWidgetselectDefaultMode" << endl;
    emit debugLog(Q_FUNC_INFO, "Start", Debug);
    if ((upAndRunning) || (!_init))
    {
        emit debugLog(Q_FUNC_INFO, "END-1", Debug);
        return;
    }
    int defaultMode = dataProxy->getMostUsedMode(-1); //TODO: The log could be defined here
      //qDebug() << "MainQSOEntryWidgetselectDefaultMode: " << QString::number(defaultMode) << endl;

    if (defaultMode < 1)
    {
        defaultMode = dataProxy->getSubModeIdFromSubMode(getBand(0));
    }
    setMode(dataProxy->getNameFromSubModeId(defaultMode));
    //modeComboBox->setCurrentIndex(modeComboBox->findText(dataProxy->getNameFromSubModeId(defaultMode)));

     //qDebug() << "MainQSOEntryWidgetselectDefaultMode3: " << QString::number(defaultMode) << endl;
    emit debugLog(Q_FUNC_INFO, "END", Debug);
     //qDebug() << "MainQSOEntryWidgetselectDefaultMode-END" << endl;
}

void MainQSOEntryWidget::setDuplicatedQSOSlot (const int _secs)
{
    if (_secs >= 0)
    {
        duplicatedQSOSlotInSecs = _secs;
    }
}

void MainQSOEntryWidget::checkIfDupe(const QString &_func)
{
   //qDebug() << Q_FUNC_INFO << "(" << _func << ")" << endl;

    QDateTime _dateTime;
    _dateTime.setDate(dateEdit->date());
    _dateTime.setTime(timeEdit->time());

    if ((dataProxy->isThisQSODuplicated(Q_FUNC_INFO, qrzLineEdit->text(), _dateTime, dataProxy->getIdFromBandName(bandComboBox->currentText()), dataProxy->getIdFromModeName(modeComboBox->currentText()), duplicatedQSOSlotInSecs).length()<2) || modify)
    {
        //qDebug() << Q_FUNC_INFO << " - NOT DUPE " << endl;
        //qDebug() << Q_FUNC_INFO << " - Modify: " << util->boolToQString(modify) << endl;

        qrzgroupBox->setTitle(tr("Callsign"));
    }
    else
    {
        //qDebug() << Q_FUNC_INFO << " - NOT DUPE " << endl;
        qrzgroupBox->setTitle(tr("DUPE", "Translator: DUPE is a common world for hams. Do not translate of not sure"));
    }
}

void MainQSOEntryWidget::slotStartDelayInputTimer()
{
    if (cleaning)
    {
        return;
    }
    if (qrzLineEdit->text ().length ()<1)
    {
        //qDebug() << Q_FUNC_INFO;
        slotClearButtonClicked ();
        return;
    }

    int cursor = qrzLineEdit->cursorPosition ();
    QString aux = util->getClearSQLi (qrzLineEdit->text ());
    qrzLineEdit->setText (aux.toUpper());
    qrzLineEdit->setCursorPosition (cursor);
    delayInputTimer->start(300);
}

void MainQSOEntryWidget::slotDelayInputTimedOut()
{
    delayInputTimer->stop();
    QString text = qrzLineEdit->text();
    if( text != lastQrz)
    {
        text = lastQrz;
        slotQRZTextChanged();
    }
}

bool MainQSOEntryWidget::getDarkMode()
{
    if (OKButton->palette().color (QPalette::Base) == "#646464")
    {
        return true;
    }
    else
    {
        return false;
    }
}
