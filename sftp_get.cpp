/**
 * SFTP download by libssh2
 */

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

#include "sftp_get.h"

using namespace std;

SftpGet::SftpGet()
    : m_socket(0)
    , m_sshSession(NULL)
    , m_sftpSession(NULL)
{
}

SftpGet::~SftpGet()
{
}

int SftpGet::Connect(const char* host, int port, const char* username, const char* password)
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

    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(host);
    qDebug() << QString("----host:%1, port:%2----").arg(host).arg(port);
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

    // const char* fingerprint =
    libssh2_hostkey_hash(m_sshSession, LIBSSH2_HOSTKEY_HASH_SHA1);

    // check what authentication methods are available
    const char* userauthlist = libssh2_userauth_list(m_sshSession, username, strlen(username));
    if (strstr(userauthlist, "password") == NULL)
    {
        m_lastError = "Authentication by password not supported";
        return 1;
    }

    // authenticate via password
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

int SftpGet::SFtpRead(const char* remoteFile, char* data_buf, int* data_len)
{
    // Request a file via SFTP
    LIBSSH2_SFTP_HANDLE* sftpHandle = libssh2_sftp_open(m_sftpSession, remoteFile, LIBSSH2_FXF_READ, 0);
    if (!sftpHandle)
    {
        m_lastError = QString("Unable to open file with SFTP: %1\n").arg(libssh2_sftp_last_error(m_sftpSession));
        return -1;
    }

    // libssh2_sftp_open() is done, now receive data!
    int len = 0;
    while(1)
    {
        if (len < *data_len)
        {
            int sz = libssh2_sftp_read(sftpHandle, data_buf + len, *data_len - len);
            if (sz > 0)
            {
                len += sz;
            }
            else
            {
                break;
            }
        }
        else
        {
            m_lastError = "The data buffer is too small for the file data";
            return 1;
        }
    }

    *data_len = len;
    libssh2_sftp_close(sftpHandle);
    return 0;
}

int SftpGet::Download(const char* remoteFile, const char* localPath)
{
    // Request a file via SFTP
    LIBSSH2_SFTP_HANDLE* sftpHandle = libssh2_sftp_open(m_sftpSession, remoteFile, LIBSSH2_FXF_READ, 0);
    if (!sftpHandle)
    {
        //snprintf(m_lastError, sizeof(m_lastError), "Unable to open file with SFTP: %ld\n",
                 //libssh2_sftp_last_error(m_sftpSession));
        return -1;
    }

    // libssh2_sftp_open() is done, now receive data!
    FILE* fp = fopen(localPath, "w");
    if (!fp)
    {
        //snprintf(m_lastError, sizeof(m_lastError), "Fail to create file: %s", localPath);
        return 2;
    }

    char buf[1024];
    while(1)
    {
        int sz = libssh2_sftp_read(sftpHandle, buf, sizeof(buf));
        if (sz > 0)
        {
            fwrite(buf, 1, sz, fp);
        }
        else
        {
            break;
        }
    }

    fclose(fp);
    libssh2_sftp_close(sftpHandle);
    return 0;
}

void SftpGet::Close()
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
        ::close(m_socket);
        m_socket = 0;
    }
    libssh2_exit();
}

QString SftpGet::GetLastError()
{
    return m_lastError;
}
