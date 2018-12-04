#include <sstream>
#include <iostream>
#include <chrono>

#include "led.h"

void Led::getData(Data& curData)
{
    curData = data;
}

void Led::setData(Data& newData)
{
    {
        lock_guard<mutex> dataLock(dataMutex);
        data = newData;
    }
    dataCond.notify_one();
}
void Led::waitForUpdate(Data& curData){
    unique_lock<mutex> dataLock(dataMutex);
    
    if (curData.rate == 0 || curData.state == "off") {
        // wait for data change
        dataCond.wait(dataLock,  
            [&](){return getDataIfChanged(curData);});
    } else {
        // wait for half-period (or data change)
        dataCond.wait_for(dataLock, chrono::milliseconds(500/data.rate), 
            [&](){return getDataIfChanged(curData);});
    }
}
bool Led::getDataIfChanged(Data& curData) {
    if (this->data.state != curData.state
        || this->data.color != curData.color
        || this->data.rate != curData.rate
        || this->data.exitReq) {
        curData = this->data;
        return true;
    }
    return false;
}

LedProtocol::LedProtocol(Led *pLedToControl) {
    init(pLedToControl);
}

void LedProtocol::command(string& strCmd, string& strResp) {
    vector<string> strCmdVect;
    splitCmd(strCmd, strCmdVect);
    auto cmd = cmdMap.find(strCmdVect[0]);
    if (cmd != cmdMap.end()) {
        Led::Data data;
        if (cmd->second(strCmdVect, strResp, data))
            pLed->setData(data);
    } else {
        strResp = "FAILED (unknown command)";
    }
}

void LedProtocol::init(Led *pLedToControl) {
    pLed = pLedToControl;
    cmdMap.emplace("set-led-state", bind(&LedProtocol::setStateCmd, this, _1,_2,_3));
    cmdMap.emplace("set-led-color", bind(&LedProtocol::setColorCmd, this, _1,_2,_3));
    cmdMap.emplace("set-led-rate",  bind(&LedProtocol::setRateCmd,  this, _1,_2,_3));
    cmdMap.emplace("get-led-state", bind(&LedProtocol::getStateCmd, this, _1,_2,_3));
    cmdMap.emplace("get-led-color", bind(&LedProtocol::getColorCmd, this, _1,_2,_3));
    cmdMap.emplace("get-led-rate",  bind(&LedProtocol::getRateCmd,  this, _1,_2,_3));
    cmdMap.emplace("exit",          bind(&LedProtocol::exitCmd,     this, _1,_2,_3));
}

void LedProtocol::splitCmd(string& strCmd, vector<string>& strCmdVect) {
    istringstream istr(strCmd);
    string s;
    strCmdVect.clear();
    while(getline(istr, s, ' ')) {
        strCmdVect.push_back(s);
    }
}

Led::Rate LedProtocol::rateFromString(string& str) {
    int n;
    try {
        n = stoi(str);
    } catch(exception e) {
        n = -1;
    }
    return (Led::Rate)n;
}

bool LedProtocol::setStateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 2) {
        strResp = "FAILED";
        return false;
    } 
    string& newState = strCmdVect[1];
    if (newState != "off" && newState != "on" ) {
        strResp = "FAILED";
        return false;
    }
    strResp = "OK";
    pLed->getData(curData);
    curData.state = newState;
    return true;
}

bool LedProtocol::setColorCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 2) {
        strResp = "FAILED";
        return false;
    } 
    string& newColor = strCmdVect[1];
    if (newColor != "red" && newColor != "green" && newColor != "blue") {
        strResp = "FAILED";
        return false;
    }
    strResp = "OK";
    pLed->getData(curData);
    curData.color = newColor;
    return true;
}

bool LedProtocol::setRateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 2) {
        strResp = "FAILED";
        return false;
    } 
    Led::Rate newRate = rateFromString(strCmdVect[1]);
    if (newRate < 0 || newRate > 5) {
        strResp = "FAILED";
        return false;
    }
    strResp = "OK";
    pLed->getData(curData);
    curData.rate = newRate;
    return true;
}

bool LedProtocol::getStateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 1) {
        strResp = "FAILED";
        return false;
    }
    pLed->getData(curData);
    strResp = "OK " + curData.state;
    return false;
}

bool LedProtocol::getColorCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 1) {
        strResp = "FAILED";
        return false;
    }
    pLed->getData(curData);
    strResp = "OK " + curData.color;
    return false;
}

bool LedProtocol::getRateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 1) {
        strResp = "FAILED";
        return false;
    }
    pLed->getData(curData);
    strResp = "OK " + to_string(curData.rate);
    return false;
}

bool LedProtocol::exitCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData) {
    if (strCmdVect.size() != 1) {
        strResp = "FAILED";
        return false;
    }
    strResp = "OK";
    pLed->getData(curData);
    curData.exitReq = true;
    return true;
}

LedView::LedView(Led *pLedToView) {
    init(pLedToView);
    start();
}

void LedView::start() {
    viewThread = thread(&LedView::doView, this);
}

void LedView::waitStop() {
    viewThread.join();
    cout << endl << endl;
}

void LedView::init(Led *pLedToView) {
    pLed = pLedToView;
    cout << endl << "   LedView:   " << endl << endl;
    updateOff();
}

void LedView::doView() {
    Led::Data curData;
    while (!curData.exitReq) {
        updateOn(curData);            // first half-period (on)
        pLed->waitForUpdate(curData); // waits half-period
        if (curData.state == "on" && curData.rate > 0) {
            updateOff();              // second half-period (blink off)
            pLed->waitForUpdate(curData); // waits half-period
        }
    }
}

void LedView::updateOn(Led::Data& curData) {
    string colorCode;
    if (curData.color == "red")
        colorCode = "\033[0;31m";
    else if (curData.color == "green")
        colorCode = "\033[0;32m";
    else if (curData.color == "blue")
        colorCode = "\033[0;34m";
    
    if (curData.state == "on")
        cout << "\r   [" << colorCode << " * " << "\033[0m" << "]   " << flush;
    else
        cout << "\r   [   ]   " << flush;
}

void LedView::updateOff() {
     cout << "\r   [   ]   " << flush;
}


