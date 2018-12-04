#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace std::placeholders;

class Led {
public:
    Led() = default;
    using State = string; // {off, on};
    using Color = string; // {red, green, blue};
    using Rate = int;     // [0..5]

    struct Data {
        State state{"off"};
        Color color{"red"};
        Rate  rate{0};
        bool  exitReq{false};
    };

    void getData(Data& newData);
    void setData(Data& newData);
    void waitForUpdate(Data& curData);

private:
    bool getDataIfChanged(Data& curData);

    Data data;
    mutex dataMutex;
    condition_variable dataCond;
};

class LedProtocol {
public:
    LedProtocol(Led *pLedToControl);
    void command(string& strCmd, string& strResp);

private:
    using LedCommand = function<bool(vector<string>&, string&, Led::Data&)>;
    using LedCommandsMap = map<string, LedCommand>;
    LedCommandsMap cmdMap;

    void init(Led *pLed);
    void splitCmd(string& strCmd, vector<string>& strCmdVect);
    Led::Rate rateFromString(string& str);
    bool setStateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);
    bool setColorCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);
    bool setRateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);
    bool getStateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);
    bool getColorCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);
    bool getRateCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);
    bool exitCmd(vector<string>& strCmdVect, string& strResp, Led::Data& curData);

    Led *pLed;
};

class LedView {
public:
    LedView(Led *pLedToView);
    void start();
    void waitStop();
    
private: 
    void init(Led *pLedToView);
    void doView();
    void updateOn(Led::Data& curData);
    void updateOff();

    thread viewThread;
    Led *pLed;
};
