#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string.h>
#include <conio.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_address>\n", argv[0]);
        return 1;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct addrinfo* result = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(argv[1], "8319", &hints, &result) != 0) {
        printf("getaddrinfo failed: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (connect(clientSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    char recvbuf[512];
    int recvbuflen = 512;
    int bytesReceived = recv(clientSocket, recvbuf, recvbuflen, 0);
    if (bytesReceived > 0) {
        printf("Bytes received: %d\n", bytesReceived);
        printf("Response from server: %s\n", recvbuf);

        unsigned short int year, day, hour, min, sec, mm;
        char weekday[10], month[10];
        sscanf(recvbuf, " %s %s %02hu %02hu:%02hu:%02hu %hu", weekday, month, &day, &hour, &min, &sec, &year);

        char mon[12][5] = {
            {"Jan"}, {"Feb"}, {"Mar"}, {"Apr"}, {"May"}, {"Jun"},
            {"Jul"}, {"Aug"}, {"Sep"}, {"Oct"}, {"Nov"}, {"Dec"}
        };
        for (int i = 0; i < 12; i++) {
            if (strcmp(month, mon[i]) == 0) {
                mm = i + 1;
            }
        }

        SYSTEMTIME newTime;
        GetLocalTime(&newTime);
        newTime.wYear = year;
        newTime.wMonth = mm;
        newTime.wDay = day;
        newTime.wHour = hour;
        newTime.wMinute = min;
        newTime.wSecond = sec;
        if (SetLocalTime(&newTime) == 0) {
            printf("Error setting system time.\n");
            return 1;
        }

        printf("Success!\n");
        printf("Press any key to continue...");
        _getch();
    } else if (bytesReceived == 0) {
        printf("Connection closed\n");
    } else {
        printf("recv failed: %d\n", WSAGetLastError());
        printf("Error connecting to server.\n");
        printf("Press any key to continue...");
        _getch();
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
