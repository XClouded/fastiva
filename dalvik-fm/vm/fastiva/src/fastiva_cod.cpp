#include <Dalvik.h>

#include <libdex/ZipArchive.h>
#include <libdex/OptInvocation.h>
#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

u1 fastiva_cod_addr[4] = { 192, 168, 0, 3 };
u2 fastiva_cod_port = 7777;
u1 initialzed = false;

static char* read_int(char* buf, int* res) {

	for (;(u8)*buf >= '0'; buf++);
	for (;(u8)*buf <  '0'; buf++);
	int v = 0;
	for (;(u8)*buf >= '0'; buf ++) {
		v *= 10;
		v += (u8)*buf - '0';
	}
	*res = v;
	return buf;
}

void init_cod_service() {
	if (initialzed) {
		return;
	}
	
	int fd = open("/sdcard/fastiva.cod", O_RDONLY);
	if (fd >= 0) {
		char lineBuf[1024];
		char* buf = &lineBuf[0];
			int v;

		int cc = read(fd, lineBuf, sizeof(lineBuf)-1);
		for (int i = 0; i < 4; i ++) {
			buf = read_int(buf, (int*)&v);
			fastiva_cod_addr[0] = (u1)v;
		}
			buf = read_int(buf, (int*)&v);
			fastiva_cod_port = (u2)v;
		ALOGE("cod server started addr: %d.%d.%d.%d", fastiva_cod_addr[0], fastiva_cod_addr[1], fastiva_cod_addr[2], fastiva_cod_addr[3]);
		close(fd);
		initialzed = true;
	}
	
}


static int connectSocket() {
#ifdef _WIN32
	WSADATA wsaData = {0};
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	init_cod_service();
	if (fastiva_cod_port == 0) return -1;

	sockaddr_storage ss;
	memset(&ss, 0, sizeof(ss));
	ss.ss_family = AF_INET;
    sockaddr_in* sin =(sockaddr_in*)(void*)(&ss);
    sin->sin_port = htons(fastiva_cod_port);
	sin->sin_addr.s_addr = *(int*)(void*)&fastiva_cod_addr;
	ALOGD("$$$$$$$$ connecting cod server %d.%d.%d.%d", fastiva_cod_addr[0], fastiva_cod_addr[1], fastiva_cod_addr[2], fastiva_cod_addr[3]);

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
		ALOGD("$$$$$$$$ socket open fail: %s", strerror(errno));
		return -1;
	}

	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)(void*)&tv,sizeof(tv));

#ifndef _WIN32
	#define closesocket close
#endif

	if (connect(sock, (sockaddr*)(void*)&ss, sizeof(sockaddr_storage)) < 0) {
		ALOGD("$$$$$$$$ socket connect fail: %s", strerror(errno));
		closesocket(sock);
		return -1;
	}
	return sock;
}

static int d2f_dex2Fastiva(ZipArchive* archive, const char* apk_path, const char* out_path) {

    ALOGD("#########################################################");
    ALOGD("#### run jpp_cod %s", apk_path);
    ALOGD("#########################################################");

    /* First, look for a ".odex" alongside the jar file.  It will
     * have the same name/path except for the extension.
     */
	char* cachedName = (char*)alloca(strlen(out_path) + 16);
	int result = -1;
	
	strcpy(cachedName, out_path);
	strcat(cachedName, ".so");
	int fd = -1;
	int sock = -1;
    ZipEntry entry;
	#define kDexInJarName        "classes.dex"
    entry = dexZipFindEntry(archive, kDexInJarName);
	const char* odexOutputName;
    if (entry == NULL) {
        ALOGI("Zip is good, but no %s inside, and no valid .odex SSSSS "
                "file in the same directory", kDexInJarName);
        goto bail;
    }

#ifdef _WIN32
    fd = open(cachedName, O_CREAT|O_RDWR| _O_BINARY, 0644);
#else
    fd = open(cachedName, O_CREAT|O_RDWR, 0644);
#endif
    if (fd < 0) {
        ALOGI("$$$$$$ Unable to open fastiva output (%s): %s", cachedName, strerror(errno));
        goto bail;
    }
	else {
		int len = 0;
		int fileSize = dexGetZipEntryUncompLen(archive, entry);
			ALOGI("$$$$$$ Sending dex to COD %s %d bytes",
				apk_path, fileSize);
		sock = connectSocket();
		if (sock < 0) {
			goto bail;
		}
		len = send(sock, apk_path, strlen(apk_path)+1, 0);
		if (len < 0) {
			ALOGE("$$$$$$ File send fail 1");
			goto bail;
		}

		int netFileSize = htonl(fileSize);
		len = send(sock, (char*)&netFileSize, 4, 0);
		if (len < 0) {
			ALOGE("$$$$$$ File send fail 2");
			goto bail;
		}

		if (dexZipExtractEntryToFile(archive, entry, fd) == 0) {
			lseek(fd, 0, SEEK_SET);
			u1 buf[8192];
			int off = 0;
			struct timeval tv;

			while (off < fileSize) {
				int len = read(fd, buf, 8192);
				if (len < 0) {
					ALOGE("$$$$$$ File stream closed at %d/%d", off, fileSize);
					goto bail;
				}
				send(sock, (char*)buf, len, 0);
				off += len;
			}

			tv.tv_sec = 20*60;
			tv.tv_usec = 0;
			//setsockopt(sock,SOL_SOCKET,(SO_RCVTIMEO),(char*)(void*)&tv,sizeof(tv));
			len = recv(sock, (char*)&netFileSize, 4, MSG_WAITALL);
			if (len < 0 || netFileSize == 0) {
				ALOGE("$$$$$$ init recv fail 1");
				goto bail;
			}
			int fileSize = htonl(netFileSize);
			ALOGE("$$$$$$ receiving compiled file '%d", fileSize);

			lseek(fd, 0, SEEK_SET);
			off = 0;
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			//setsockopt(sock,SOL_SOCKET,(SO_RCVTIMEO),(char*)(void*)&tv,sizeof(tv));
			while (off < fileSize) {
				len = recv(sock, (char*)buf, 8192, 0);
				if (len < 0) {
					ALOGE("$$$$$$ Socket stream closed at %d/%d", off, fileSize);
					goto bail;
				}
				write(fd, buf, len);
				off += len;
			}

			send(sock, "0", 1, 0);
		}
	};
    ALOGD("$$$$$$ compile finished '%s", apk_path);

	result = 0;

bail:

    if (fd >= 0) {
        close(fd);
		if (result < 0) {
            unlink(cachedName);
		}
		else {
			chmod(cachedName, 0755);
		}
    }
    if (sock >= 0) {
        close(sock);
    }
    return result;
}

extern "C" int d2f_compileDex(int zip_fd, const char* apk_path, const char* out_path) {
	ZipArchive archive;
	int res = dexZipPrepArchive(zip_fd, out_path, &archive);
	if (res != 0) {
	    ALOGD("$$$$ archive open fail %s: %s", out_path, strerror(errno));
		return -1;
	}
	return d2f_dex2Fastiva(&archive, apk_path, out_path);
}

