/**
 * SFTP download by libssh2
 */

#include <fcntl.h>
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include "sfp.h"

Sftp::Sftp()
    : m_socket(0)
    , m_sshSession(NULL)
    , m_sftpSession(NULL)
{
}

Sftp::~Sftp()
{
    this->Close();
}

int Sftp::Connect(const char* host, int port, const char* username, const char* password)
{
    int rc = libssh2_init(0);
    if (rc != 0)
    {
        m_lastError = "libssh2 initialization failed: %d";
        return rc;
    }

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        system("pause");
        exit(1);
    }
    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    qDebug() << QString("host:%1, port:%2").arg(host, port);

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(host);
    int ret = ::connect(m_socket, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in));
    if (ret != 0)
    {
        m_lastError = QString("failed to connect!, ret=%1").arg(WSAGetLastError());
        return -1;
    }

    // Create a session instance
    m_sshSession = libssh2_session_init();
    if(!m_sshSession)
    {
         m_lastError = "libssh2_session_init failed!";
        return -1;
    }

    // Since we have set non-blocking, tell libssh2 we are blocking
    libssh2_session_set_blocking(m_sshSession, 1);

    rc = libssh2_session_handshake(m_sshSession, m_socket);
    if (rc != 0)
    {
        m_lastError = "Failure establishing SSH session: %d";
        return rc;
    }

    libssh2_hostkey_hash(m_sshSession, LIBSSH2_HOSTKEY_HASH_SHA1);

    // check what authentication methods are available
    const char* userauthlist = libssh2_userauth_list(m_sshSession, username, strlen(username));
    if (strstr(userauthlist, "password") == NULL)
    {
        m_lastError = "Authentication by password not supported";
        return 1;
    }

    // authenticate via password
    qDebug() << QString("user:%1, pwd:%2").arg(username, password);
    if (libssh2_userauth_password(m_sshSession, username, password))
    {
        m_lastError = "Authentication by password failed";
        return 2;
    }

    m_sftpSession = libssh2_sftp_init(m_sshSession);
    if (!m_sftpSession)
    {
        m_lastError = "Unable to init SFTP session";
        return -1;
    }

    return 0;
}

int Sftp::Upload(const char* remoteFile, const char* localPath)
{
    // Request a file via SFTP
    LIBSSH2_SFTP_HANDLE* sftpHandle = libssh2_sftp_open(m_sftpSession, remoteFile,
        LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,
        LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR| LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH);
    if (!sftpHandle)
    {
        m_lastError = QString("Unable to open file with SFTP: %1").arg(libssh2_sftp_last_error(m_sftpSession));
        return -1;
    }


    FILE *local = fopen(localPath, "rb");
    if(!local) {
        m_lastError = QString("Can't open local file: %1").arg(localPath);
        return -1;

    }

    // libssh2_sftp_open() is done, now receive data!
    char buf[1024] = {0};
    size_t nread = 0;
    char *ptr = nullptr;
    ssize_t rc = 0;
    int ret = 0;
    while(1)
    {
        nread = fread(buf, 1, sizeof(buf), local);
        if(nread <= 0) {
            /* end of file */
            break;
        }

        ptr = buf;
        do {
            /* write data in a loop until we block */
            rc = libssh2_sftp_write(sftpHandle, ptr, nread);
            if(rc <= 0) {
                ret = -1;
                m_lastError = QString("Write remote file %1 failed, write data ret=%2").arg(remoteFile, rc);
                break;
            }
            ptr += rc;
            nread -= rc;
        } while(nread);
    }

    libssh2_sftp_close(sftpHandle);
    fclose(local);
    return ret;
}

int Sftp::Download(const char* remoteFile, const char* localPath)
{
    // Request a file via SFTP
    LIBSSH2_SFTP_HANDLE* sftpHandle = libssh2_sftp_open(m_sftpSession, remoteFile, LIBSSH2_FXF_READ, 0);
    if (!sftpHandle)
    {
        m_lastError = QString("Unable to open file with SFTP: %1").arg(libssh2_sftp_last_error(m_sftpSession));
        return -1;
    }

    FILE* fp = fopen(localPath, "wb");
    if (!fp)
    {
        m_lastError = QString("Fail to create file: %1").arg(localPath);
        return 2;
    }

    char buf[1024] = {0};
    ssize_t nread = 0;
    char *ptr = nullptr;
    ssize_t wc = 0;
    int ret = 0;
    while(1)
    {
        nread = libssh2_sftp_read(sftpHandle, buf, sizeof(buf));
        if(nread <= 0) {
            /* end of file */
            break;
        }

        ptr = buf;
        qDebug() << QString::fromLatin1(ptr, nread);
        do {
            /* write data in a loop until we block */
            wc = fwrite(ptr, 1, nread, fp);
            if(wc <= 0) {
                ret = -1;
                m_lastError = QString("Write local file %1 failed, write data ret=%2").arg(localPath, wc);
                break;
            }
            ptr += wc;
            nread -= wc;
        } while(nread);
    }

    fclose(fp);
    libssh2_sftp_close(sftpHandle);
    return ret;
}

int Sftp::ExecuteCommad(const char *command)
{
    LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(m_sshSession);
    if (channel == NULL) {
        m_lastError = QString("open channel session failed");
        return 1;
    }

    int rc = libssh2_channel_exec(channel, command);
    if (rc != 0) {
        m_lastError = QString("Failed to execute remote command:%1").arg(command);
        return 1;
    }

    // 读取并打印远程命令输出
    char buffer[1024];
    int nbytes;
    while ((nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[nbytes] = '\0';
        QString output = QString(buffer);
        qDebug() << output;
        m_output_text = output;
    }

    // 断开连接和清理
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);

    return 0;
}

void Sftp::Close()
{
    if (m_sftpSession)
    {
        libssh2_sftp_shutdown(m_sftpSession);
        m_sftpSession = NULL;
    }

    if (m_sshSession)
    {
        libssh2_session_disconnect(m_sshSession, "Shutdown");
        libssh2_session_free(m_sshSession);
        m_sshSession = NULL;
    }

    if (m_socket)
    {
        close(m_socket);
        m_socket = 0;
    }
    libssh2_exit();
}

QString Sftp::GetLastError()
{
    return m_lastError;
}

QString Sftp::GetOutputText()
{
    return m_output_text;
}
