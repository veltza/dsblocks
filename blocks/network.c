#include <ifaddrs.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <net/if_arp.h>
#include <sys/stat.h>

#include "../util.h"
#include "network.h"

#define ETHER_ICON_UP                   COL12 " " COL0
#define ETHER_ICON_DOWN                 COL13 " " COL0
#define WIFI_ICON_UP                    COL12 " " COL0
#define WIFI_ICON_DOWN                  COL13 " " COL0

#define NETWORK_INTERFACES              "/sys/class/net"

#define NETWORK_MANAGER                 (char *[]){ "networkmanager_dmenu", NULL }
#define NETWORK_EDITOR                  (char *[]){ "nm-connection-editor", NULL }

enum { ETHER, WIFI };

int
isconnected(char *iface)
{
    char path[512], str[80];
    struct ifaddrs *address, *addresses;
    int family, status = 0;

    sprintf(path, NETWORK_INTERFACES "/%s/carrier", iface);
    if (!readstr(path, str, sizeof str) || str[0] == '0')
        return 0;

    sprintf(path, NETWORK_INTERFACES "/%s/operstate", iface);
    if (!readstr(path, str, sizeof str) || str[0] != 'u' || str[1] != 'p')
        return 0;

    if (getifaddrs(&addresses) == -1)
        return 0;

    for (address = addresses; address; address = address->ifa_next) {
        family = address->ifa_addr->sa_family;
        if ((family == AF_INET || family == AF_INET6)
            && strcmp(address->ifa_name, iface) == 0) {
            status = 1;
            break;
        }
    }
    freeifaddrs(addresses);
    return status;
}

void
readnetworkstatus(int *ethstatus, int *wifistatus)
{
    DIR *dir;
    struct dirent *dirent;
    struct stat sb;
    char path[512], *iface;
    int type;

    *ethstatus = 0;
    *wifistatus = 0;

    if (!(dir = opendir(NETWORK_INTERFACES)))
       return;

    while ((dirent = readdir(dir))) {
        iface = dirent->d_name;
        if (iface[0] == '.')
            continue;

        sprintf(path, NETWORK_INTERFACES "/%s/type", iface);
        if (!readint(path, &type) || type != ARPHRD_ETHER)
            continue;

        sprintf(path, NETWORK_INTERFACES "/%s/wireless", iface);
        type = (stat(path, &sb) == 0) ? WIFI : ETHER;

        sprintf(path, NETWORK_INTERFACES "/%s/phy80211", iface);
        type = (stat(path, &sb) == 0) ? WIFI : type;

        if (type == ETHER)
            *ethstatus = *ethstatus || isconnected(iface);
        else
            *wifistatus = *wifistatus || isconnected(iface);
    }
    closedir(dir);
}

size_t
networku(char *str, int sigval)
{
    static char *ethicons[] = { ETHER_ICON_DOWN, ETHER_ICON_UP };
    static char *wifiicons[] = { WIFI_ICON_DOWN, WIFI_ICON_UP };
    static int lastconnection = ETHER;
    int ethstatus, wifistatus;

    readnetworkstatus(&ethstatus, &wifistatus);

    if (ethstatus && !wifistatus)
        lastconnection = ETHER;
    else if (!ethstatus && wifistatus)
        lastconnection = WIFI;

    if (ethstatus && wifistatus)
        return SPRINTF(str, "%s%s", ethicons[ethstatus], wifiicons[wifistatus]);
    else if (ethstatus || lastconnection == ETHER)
        return SPRINTF(str, "%s", ethicons[ethstatus]);
    else
        return SPRINTF(str, "%s", wifiicons[wifistatus]);
}

void
networkc(int button)
{
    switch (button) {
    case 1:
        cspawn(NETWORK_MANAGER);
        break;
    case 3:
        cspawn(NETWORK_EDITOR);
        break;
    default:
        break;
    }
}
