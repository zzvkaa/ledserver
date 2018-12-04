#include "ledserver.h"

int main()
{
    LedServer ledServer;
    try {
        ledServer.mainLoop();
    } catch (int err) {
        return err;
    }
    return 0;
}
