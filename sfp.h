#ifndef SFTP_H
#define SFTP_H

#include <QApplication>
#include <winsock2.h>
#include <Windows.h>
#include "libssh2.h"
#include "libssh2_sftp.h"

class Sftp
{
public:
    Sftp();
    ~Sftp();

    int Connect(const char* host, int port, const char* username, const char* password);

    int Upload(const char* localPath, const char* remoteFile);

    int Download(const char* remoteFile, const char* localPath);

    int ExecuteCommad(const char *command);

    void Close();

    QString GetLastError();

     QString GetOutputText();

    SOCKET m_socket;

    LIBSSH2_SESSION *m_sshSession;

    LIBSSH2_SFTP *m_sftpSession;

    QString m_lastError;
    QString m_output_text;
};

#endif // SFTP_H
