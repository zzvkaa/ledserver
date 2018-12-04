#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include "ledserver.h"

void LedServer::mainLoop()
{
    char c;
    createFifos();
    openFifos();
    do {
        c = readCharFromFifo();
        if (c != 0) {
            if (!processFifoChar(c))
                break;
        } else {
            // reopen fifos on EOF
            closeFifos();
            openFifos();
        }
    } while (1);
    closeFifos();
    deleteFifos();
    ledView.waitStop();
}

void LedServer::createFifos()
{
    if (0 != mkfifo(fifoInName.c_str(), S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH))
        throw -1;
    if (0 != mkfifo(fifoOutName.c_str(), S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH))
        throw -2;
}

void LedServer::deleteFifos()
{
    unlink(fifoInName.c_str());
    unlink(fifoOutName.c_str());
}

void LedServer::openFifos()
{
    fifoInFd = open(fifoInName.c_str(), O_RDONLY);
    if (fifoInFd == -1)
        throw -3;
    fifoOutFd = open(fifoOutName.c_str(), O_WRONLY);
    if (fifoOutFd == -1)
        throw -4;
}

void LedServer::closeFifos()
{
    close(fifoInFd);
    close(fifoOutFd);
}

char LedServer::readCharFromFifo()
{
    char c;
    if ( read(fifoInFd, &c, 1) <= 0)
        c = 0;
    return c;
}

void LedServer::writeStringToFifo(std::string& str)
{
    write(fifoOutFd, str.c_str(), str.size());
    // TODO: handle write errors
}

bool LedServer::processFifoChar(char c)
{
    string resp;
    if (c == '\n') {
        ledProtocol.command(fifoCmd, resp);
        writeStringToFifo(resp);
        if (fifoCmd == "exit")
            return false;
        fifoCmd.clear();
    } else {
        fifoCmd.push_back(c);
    }
    return true;
}
