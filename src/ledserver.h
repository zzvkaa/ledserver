#include <string>
#include <functional>
#include <map>
#include "led.h"

class LedServer {
public:
    LedServer() = default;
    void mainLoop();
private:
    Led led;
    LedProtocol ledProtocol{&led};
    LedView ledView{&led};

    std::string fifoInName{"/tmp/zzvkaa.ledserver.in.fifo"};
    std::string fifoOutName{"/tmp/zzvkaa.ledserver.out.fifo"};
    int fifoInFd{-1};
    int fifoOutFd{-1};
    std::string fifoCmd;

    void createFifos();
    void deleteFifos();
    void openFifos();
    void closeFifos();
    void writeStringToFifo(std::string& str);
    char readCharFromFifo();
    void initFunctionsMap();

    bool processFifoChar(char c);
};
