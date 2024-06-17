#ifndef SFTP_GET_H
#define SFTP_GET_H

#include "widget.h"

#include <QApplication>

#include <iostream>

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include <ctype.h>

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <Windows.h>
#endif

#include "libssh2.h"
#include "libssh2_sftp.h"

class SftpGet
{
public:
    SftpGet();
    ~SftpGet();

    int Connect(const char* host, int port, const char* username, const char* password);

    int SFtpRead(const char* remoteFile, char* data_buf, int* data_len);

    int Download(const char* remoteFile, const char* localPath);

    void Close();

    QString GetLastError();

    SOCKET m_socket;

    LIBSSH2_SESSION *m_sshSession;

    LIBSSH2_SFTP *m_sftpSession;

    QString m_lastError;
};

#endif // SFTP_GET_H
