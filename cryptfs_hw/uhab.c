/* Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
********************************************************************************************************************************
* @file  uhab.c
* @brief Implements HAB client library
********************************************************************************************************************************
*/
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdarg.h>

#ifdef __QNXNTO__
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <dll_utils_i.h>
#endif

#include "habmm.h"
#if defined(__ANDROID__)
#include <linux/hab_ioctl.h>
#include <android/log.h>
#elif defined(__linux__)
#include "linux/hab_ioctl.h"
#else
#include "hab_ioctl.h"
#ifdef __INTEGRITY
#include "pmem.h"
#include "pmemext.h"
#else
//desktop
#endif
#endif

#ifdef __linux__

#define strlcpy(d,s,l) (strncpy(d,s,l), (d)[(l)-1] = '\0')

#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

/* #define hab_DevCtl(type, snd, snd_size, snd2, snd2_size, rcv, rcv_size, rcv2, rcv2_size) ioctl(fd, type, snd) */
#endif

#ifdef __INTEGRITY
#define gettid() ((unsigned int)(uintptr_t)pthread_self())
#endif

/* Macros */
char uhab_ver_rev[]="$Date: 2018/05/04 $ $Change: 16096567 $ $Revision: #41 $";

#define PAGE_SIZE 4096   // if this has not been defined so far

static int g_log_level = 0;

#ifdef __QNXNTO__
#include "logger_utils.h"
#define UHAB_LOG(_fmt_, ...) \
    do { \
        logger_log(QCLOG_AMSS_MM, 0, _SLOG_INFO, "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#define UHAB_DBG(_fmt_, ...) \
    do { \
        if (g_log_level > 0) logger_log(QCLOG_AMSS_MM, 0, _SLOG_INFO, "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#elif defined (ANDROID)
#define UHAB_LOG(_fmt_, ...) \
    do { \
        __android_log_print(ANDROID_LOG_INFO, "uhab", "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#define UHAB_DBG(_fmt_, ...) \
    do { \
        __android_log_print(ANDROID_LOG_DEBUG, "uhab", "%s: " _fmt_, __func__, ##__VA_ARGS__); \
    } while(0)
#else
#define UHAB_LOG(_fmt_, ...) \
    do { \
        fprintf(stderr, "uhab(%d:%lu):%s: " _fmt_ "\n", getpid(), gettid(), __func__, ##__VA_ARGS__); \
    } while(0)
#define UHAB_DBG(_fmt_, ...) \
    do { \
        if (g_log_level > 0) fprintf(stderr, "uhab:debug(%d:%lu):%s: " _fmt_ "\n", getpid(), gettid(), __func__, ##__VA_ARGS__); \
    } while(0)

#endif

#if defined (__linux__) || defined (__QNXNTO__)
static void initialize_logging()
{
	char *str_dbg;

	str_dbg = getenv("HAB_DEBUG");
	if (str_dbg)
	{
		g_log_level = atoi(str_dbg);
	}
}
#endif

//#define DO_UHAB_TRACE

#ifdef DO_UHAB_TRACE
#define UHAB_TRACE_IN(__vcid__) UHAB_LOG(">> vc %X", __vcid__)
#define UHAB_TRACE_OUT(__vcid__) UHAB_LOG("<< vc %X", __vcid__)
#else
#define UHAB_TRACE_IN(__vcid__)
#define UHAB_TRACE_OUT(__vcid__)
#endif

/* Global variables */
static int fd = -1;
#ifdef __INTEGRITY
Connection connection;
#endif
static int refcnt = 0;

struct imp_mmap_node {
	uint32_t exportid; // original exported id from the remote side (allocated/granted)
	uint64_t grantidx; // local index generated based on the exportid
	void    *uaddr;    // user space linear address after mmap
	uint32_t size;
	struct imp_mmap_node *next; // to construct single linked list
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#ifndef __QNXNTO__
static struct imp_mmap_node *gimp_mmap_list = NULL;
#endif
static int impmmap_total = 0;

#ifdef __QNXNTO__
#define FIELD_SIZE(__struct__, __member__) sizeof(((__struct__*)0)->__member__)

int hab_DevCtl(_Uint16t type,
    const void *snd, int snd_size,
    const void *snd2, int snd2_size,
    void *rcv, int rcv_size,
    void *rcv2, int rcv2_size) {
    struct _io_msg    hdr;
    int                status = -1;
    iov_t            siov[3] = { {&hdr, sizeof(hdr)}, {(void*)snd, snd_size}, {(void*)snd2, snd2_size}};
    iov_t            riov[2] = { {rcv, rcv_size}, {rcv2, rcv2_size}}; // this is only formated for hab_recv to have size first, then actual msg. for hab_open, only one rcv buffer is provided
    int                sparts = 1, rparts = 0;

	/* Send: Part 1 */
	hdr.type = _IO_MSG;
	hdr.combine_len = sizeof(hdr);
	hdr.mgrid = _IOMGR_QC_HAB;
	hdr.subtype = type;

	if (snd != NULL)
		sparts++;
	if (snd2 != NULL)
		sparts++;

	// for variable size of the receive it has to depend on the reply header, which is not used here
	// predefined protocol is used instead. fixed size reply first, variable size reply at the end
	if (rcv != NULL)
		rparts = 1; // simple one part reply, no copy is needed. reply filled to sender buffer directly
	if (rcv2 != NULL) {
	  	//assert(rcv); // first recv buffer has to be provided
		if (rcv2_size == FIELD_SIZE(struct hab_recv, sizebytes)) {
			rparts = 2; // recv's reply size is returned through rpart, the data is directly copied in driver
			SETIOV(&riov[0], rcv2, rcv2_size);
			SETIOV(&riov[1], rcv, rcv_size);
		} else if (rcv_size == FIELD_SIZE(struct hab_info, ids)) {
			rparts = 2; // rcv2 actual filled length is not returned
			// rpart order is normal
		}
	}

	errno = EOK;

	status = MsgSendv_r(fd, siov, sparts, riov, rparts); /* return exact status */

	if (status != 0 && status != -ETIMEDOUT && status != -EAGAIN) {
		UHAB_LOG("MsgSendv failed to send message (device_id:0x%p, type:0x%x) %d %d\n", snd, type, status, errno);
	}

    return status;
}
#endif
#ifdef __INTEGRITY
// this version handles up to 2 return values from driver
int hab_DevCtl(int type,
	const void *snd, int snd_size,
	const void *snd2, int snd2_size,
	void *rcv, int rcv_size,
	void *rcv2, int rcv2_size)
{
	Error e;
	int32_t ret = 0;
	Buffer B[5]; // please make sure this struct matches the driver side
	bzero((void*)B, sizeof(B));
	B[0].BufferType =  DataImmediate;
	B[0].TheAddress = type;
	B[0].Transferred = 0;

	B[1].BufferType =  DataBuffer;
	B[1].TheAddress = (Address)snd;
	B[1].Length = snd_size;
	B[1].Transferred = 0;

	B[2].BufferType =  DataBuffer | WaitForReply;
	B[2].TheAddress = (Address)snd2;
	B[2].Length = snd2_size;
	B[2].Transferred = 0;

	B[3].BufferType =  DataBuffer;
	B[3].TheAddress = (Address)rcv;
	B[3].Length = rcv_size;
	B[3].Transferred = 0;

	B[4].BufferType =  DataImmediate | LastBuffer;
	B[4].Transferred = 0;

	e = SynchronousSend(connection,B);
	ret = (int)B[4].TheAddress; // return value is from driver directly
	if (e != Success) {
		UHAB_LOG("failed to send 0x%x of snd1 %d, snd2 %d, ret %d, recv %d, recv2 %d\n",
			type, snd_size, snd2_size, e, ret, rcv_size, rcv2_size);
		// ToDo: do we need to convert INTEGRITY-include/INTEGRITY_enum_error.h to posix errno?
		ret = -EPERM;
	} else if (ret && ret != -ETIMEDOUT) {
		UHAB_LOG("hab driver return error! ret %d, ret length %d, ret transfered %d\n",
			ret, B[4].Length, B[4].Transferred);
	} else if (rcv2 != NULL) { // for hab_recv and hab_query only with vmid
		// recv size will not be provided if there is error already
		uint32_t *recv_size = (uint32_t *)rcv2; // rcv2 is msg retrieved size, but names for query
		*recv_size = B[3].Transferred; // msg received in recv_buffer, but this breaks for query!
	}

	return ret;
}
#endif
#ifdef __linux__
// linux version handle return value within the ioctl parameter, the rest is ignored
int hab_DevCtl(int type,
	const void *snd, int snd_size,
	const void *snd2, int snd2_size,
	void *rcv, int rcv_size,
	void *rcv2, int rcv2_size)
{
	int ret = ioctl(fd, type, snd);
	if (ret == -1) {
		ret = -errno;
	} else if (ret != 0) {
		UHAB_LOG("unexpected error code %d returned\n", ret);
	}

	return ret;
}
#endif

//==================== utility functions ====================
void list_add(struct imp_mmap_node * node, struct imp_mmap_node ** head) {
	// add to the head
	struct imp_mmap_node *tmp;
    pthread_mutex_lock(&mutex);
	if (!(*head)) {
		*head = node;
		node->next = NULL;
	} else {
		tmp = *head;
		*head = node;
		node->next = tmp;
	}
	impmmap_total++;
    pthread_mutex_unlock(&mutex);
}

// return node has the same user address, or NULL returned if not found
struct imp_mmap_node * list_detach_by_uaddr(void * uaddr, struct imp_mmap_node ** head) {
	struct imp_mmap_node *tmp = NULL, *prev = NULL;

    pthread_mutex_lock(&mutex);
	if (!(*head)) {
		UHAB_DBG("%s: wrong list head NULL\n", __func__);
        pthread_mutex_unlock(&mutex);
		return NULL;
	}

	tmp = *head;
	if (tmp->uaddr == uaddr) { // head is the one
		*head = tmp->next;
        pthread_mutex_unlock(&mutex);
		return tmp;
	}
	prev = tmp;
	while (tmp->next) {
		if (tmp->uaddr == uaddr) {
            prev->next = tmp->next; // detach the current one

            UHAB_DBG("find matching uva to detach %p\n", uaddr);

            break;
        }
        prev = tmp;
		tmp = tmp->next; // traves to the tail
	}
    pthread_mutex_unlock(&mutex);
    return tmp;
}

//=================== UHAB functions =======================
/* habmm_socket_open
Params:
handle - An opaque handle associated with a successful virtual channel creation
MM_ID - multimedia ID used to allocate the physical channels to service all
the virtual channels created through this open
timeout - timeout value specified by the client to avoid forever block
flags - future extension

Return:
status (success/failure/timeout)
*/

int32_t habmm_socket_open(int32_t *handle, unsigned int mm_ip_id,
		uint32_t timeout, uint32_t flags)
{
	int32_t ret = 0;

	struct hab_open arg = {
		.vcid = 0, // out
		.mmid = mm_ip_id,
		.timeout = timeout,
		.flags = flags,
	};

	UHAB_TRACE_IN(*handle);

	pthread_mutex_lock(&mutex);
	if (0 == refcnt) { // first time
#if defined (__linux__) || defined (__QNXNTO__)
		initialize_logging();
#endif
#ifdef __QNXNTO__
		UHAB_DBG("hab: refcnt %d wait for hab device node..\n", refcnt);
		Resource_BlockWait("/dev/hab");
#endif
#ifdef __INTEGRITY
#ifdef HAB_FE
		while (RequestResource((Object*)&connection, "habSndConnection_fe", NULL)
			!= Success)
			usleep(10000);
#else
		while (RequestResource((Object*)&connection, "habSndConnection", NULL)
			!= Success)
			usleep(10000);
#endif
		fd = 0; // this is not used in GHS
		refcnt++;
#else
		fd = open("/dev/hab", O_RDWR);  // ToDo: single node only
		if (-1 != fd) {
			UHAB_DBG("hab: opened %d\n", fd);
			refcnt++;
		} else {
			UHAB_LOG("hab: open failed, error code %d %s\n", errno, strerror(errno));
			ret = -errno;
		}
#endif
	} else {
		refcnt++;
		UHAB_DBG("hab: re-using fd %d, cnt %d, mmid %d, timeout %d\n", fd, refcnt, mm_ip_id, timeout);
	}
	pthread_mutex_unlock(&mutex);

	if (-1 != fd) {
		ret = hab_DevCtl(IOCTL_HAB_VC_OPEN, &arg, sizeof(arg), NULL, 0, &arg.vcid, sizeof(arg.vcid), NULL, 0);
		if (ret) {
			if (ret != -ETIMEDOUT)
				UHAB_LOG("fd %d, return %d, vcid %x, error code %s, close fd now...\n", fd, ret, arg.vcid, strerror(errno));

			pthread_mutex_lock(&mutex);
			refcnt--;
			if (!refcnt) {
#ifndef __INTEGRITY
				close(fd);
#endif
				fd = -1;
			}
			pthread_mutex_unlock(&mutex);
		} else {
			UHAB_LOG("opened fd %d, return %d, vcid %x\n", fd, ret, arg.vcid);
		}
	}

	*handle = (ret == 0) ? arg.vcid : 0;

	UHAB_TRACE_OUT(*handle);

	return ret;
}

/* habmm_socket_close
Params:

handle - handle to the virtual channel that was created by habmm_socket_open

Return:
status - (success/failure)
*/

int32_t habmm_socket_close(int32_t handle)
{
	int32_t ret = 0;

	struct hab_close arg = {
		.vcid = handle,
		.flags = 0,
	};

	UHAB_TRACE_IN(handle);
	ret = hab_DevCtl(IOCTL_HAB_VC_CLOSE, &arg, sizeof(arg), NULL, 0, NULL, 0, NULL, 0);
	if (ret)
		UHAB_LOG("%s: fd %d, return %d, vcid %x, error code %s\n", __FUNCTION__, fd, ret, arg.vcid, strerror(errno));

	pthread_mutex_lock(&mutex);
	refcnt--;
	if (0 == refcnt) {
		UHAB_LOG("%s: close fd %d, cnt %d, vcid %X\n", __FUNCTION__, fd, refcnt, handle);
#ifndef __INTEGRITY
		close(fd);
#endif
		fd = -1;
	} else {
		UHAB_LOG("%s: skip close fd %d, cnt %d, vcid %X\n", __FUNCTION__, fd, refcnt, handle);
	}
	pthread_mutex_unlock(&mutex);

	UHAB_TRACE_OUT(handle);
	return ret;
}

/* habmm_socket_send
Params:

handle - handle created by habmm_socket_open
src_buff - data to be send across the virtual channel
size_bytes - size of the data to be send. Either the whole packet is sent or not
flags - future extension

Return:
status (success/fail/disconnected)
*/

int32_t habmm_socket_send(int32_t handle, void *src_buff,
			uint32_t size_bytes, uint32_t flags)
{
	int32_t ret;
	struct hab_send arg = {
		.vcid = handle,
		.data = (uintptr_t) src_buff,
		.sizebytes = size_bytes,
        .flags = flags,
	};

	UHAB_TRACE_IN(handle);

	ret = hab_DevCtl(IOCTL_HAB_SEND, &arg, sizeof(arg), src_buff, arg.sizebytes, NULL, 0, NULL, 0);
	if (ret)
		UHAB_LOG("%s: fd %d, return %d, vcid %X, error code %s\n", __FUNCTION__, fd, ret, handle, strerror(errno));

	UHAB_TRACE_OUT(handle);
	return ret;
}


/* habmm_socket_recv
Params:

handle - communication channel created by habmm_socket_open
dst_buff - buffer pointer to store received data
size_bytes - size of the dst_buff. returned value shows the actual
bytes received.
timeout - timeout value specified by the client to avoid forever block
flags - future extension


Return:
status (success/failure/timeout/disconnected)
*/
int32_t habmm_socket_recv(int32_t handle, void *dst_buff, uint32_t *size_bytes,
					uint32_t timeout, uint32_t flags)
{
	int32_t ret;
	struct hab_recv arg = {
		.vcid = handle,
		.data = (uintptr_t) dst_buff,
		.sizebytes = *size_bytes,
        .flags = flags,
	};

	UHAB_TRACE_IN(handle);

	ret = hab_DevCtl(IOCTL_HAB_RECV, &arg, sizeof(arg), NULL, 0, dst_buff, arg.sizebytes, &arg.sizebytes, sizeof(arg.sizebytes));
	if (ret) {
		UHAB_LOG("%s: fd %d, return %d, received bytes %d, vcid %X, error code %s\n", __FUNCTION__, fd, ret,
             arg.sizebytes, handle, strerror(errno));
	}

	*size_bytes = arg.sizebytes; // return actual received bytes

	UHAB_TRACE_OUT(handle);
	return ret;
}

/* habmm_socket_sendto
Params:

handle - handle created by habmm_socket_open
src_buff - data to be send across the virtual channel
size_bytes - size of the data to be send. The packet is fully sent on success,
or not sent at all upon any failure
remote_handle - the destination of this send using remote FE's virtual
channel handle
flags - future extension

Return:
status (success/fail/disconnected)
*/
int32_t habmm_socket_sendto(int32_t handle, void *src_buff, uint32_t size_bytes,
				int32_t remote_handle, uint32_t flags)
{
	UHAB_TRACE_IN(handle);

	UHAB_TRACE_OUT(handle);
	return 0;
}


/* habmm_socket_recvfrom
Params:

handle - communication channel created by habmm_socket_open
dst_buff - buffer pointer to store received data
size_bytes - size of the dst_buff. returned value shows the actual
bytes received.
timeout - timeout value specified by the client to avoid forever block
remote_handle - the FE who sent this message through the connected
virtual channel to BE.
flags - future extension

Return:
status (success/failure/timeout/disconnected)
*/
int32_t habmm_socket_recvfrom(int32_t handle, void *dst_buff,
	uint32_t *size_bytes, uint32_t timeout,
	int32_t *remote_handle, uint32_t flags)
{
	UHAB_TRACE_IN(handle);
	UHAB_TRACE_OUT(handle);
	return 0;
}

/*
Params:

handle - communication channel created by habmm_socket_open
buff_to_share - buffer to be exported
size_bytes - size of the exporting buffer in bytes
export_id - to be returned by this call upon success
flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_export(int32_t handle, void *buff_to_share, uint32_t size_bytes,
				uint32_t *pexport_id, uint32_t flags)
{
	int32_t ret;
	struct hab_export arg = {
		.vcid = handle,
		.buffer = (uintptr_t) buff_to_share,
		.sizebytes = size_bytes,
        .flags = flags
	};

	UHAB_TRACE_IN(handle);

#ifdef __INTEGRITY
	uint64_t port_id = 0;
	ret = pmem_get_portable_id(buff_to_share, &port_id);
	if (ret) {
		UHAB_LOG("pmem_get_portable_id %llx failed. Invalid address 0x%p, vcid %X\n", port_id, buff_to_share, handle);
		return -EINVAL;
	}
	arg.buffer = (uintptr_t) port_id;
#endif

	if (size_bytes%PAGE_SIZE) {
		UHAB_LOG("Error! export size (%d) has to be page aligned! vcid %X\n", size_bytes, handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}

	UHAB_DBG("%s: fd %d, exported bytes %d, memory %p, vcid %X\n", __FUNCTION__, fd, arg.sizebytes, buff_to_share, handle);

	ret = hab_DevCtl(IOCTL_HAB_VC_EXPORT, &arg, sizeof(arg), NULL, 0, &arg.exportid, sizeof(arg.exportid), NULL, 0);
	if (ret)
		UHAB_LOG("%s: return %d, export id %d, vcid %X, error code %s\n", __FUNCTION__, ret, arg.exportid, handle, strerror(errno));
	if (!ret) {
		*pexport_id = arg.exportid;

		UHAB_DBG("%s: return %d, export id %d, vcid %X, error code %s\n", __FUNCTION__, ret, arg.exportid, handle, strerror(errno));
	}

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
Params:

handle - communication channel created by habmm_socket_open
buff_shared - buffer to be imported. returned upon success
size_bytes - size of the imported buffer in bytes
import_id - received when exporter sent its exporting ID through
habmm_socket_send() previously
flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_import(int32_t handle, void **buff_shared, uint32_t size_bytes,
				uint32_t export_id, uint32_t flags)
{
	int32_t ret;
	struct hab_import arg = {
		.vcid = handle,
		.sizebytes = size_bytes,
        .exportid = export_id,
        .flags = flags
	};
#ifndef __QNXNTO__
	struct imp_mmap_node * node;
#endif
	UHAB_TRACE_IN(handle);

	if (size_bytes%PAGE_SIZE) {
		UHAB_LOG("Error! import size (%d) has to be page aligned! vcid %X\n", size_bytes, handle);
		UHAB_TRACE_OUT(handle);
		return -EINVAL;
	}


	UHAB_DBG("%s: fd %d, exported bytes %d, export id %d, vcid %X\n", __FUNCTION__, fd, arg.sizebytes, arg.exportid, handle);

	ret = hab_DevCtl(IOCTL_HAB_VC_IMPORT, &arg, sizeof(arg), NULL, 0, &arg.index, sizeof(arg.index), NULL, 0);

	UHAB_DBG("%s: fd %d, return %d, add map memory %llx, vcid %X, error code %s\n", __FUNCTION__, fd, ret, (unsigned long long)arg.index, handle, strerror(errno));

    if (ret) {
		UHAB_LOG("%s: fd %d, return %d, add map memory %llx, vcid %X, error code %s\n", __FUNCTION__, fd, ret, (unsigned long long)arg.index, handle, strerror(errno));
		UHAB_LOG ("failed to import export id %d\n", export_id);
		UHAB_TRACE_OUT(handle);
		return ret;
	}

#ifdef __QNXNTO__
    // Memory is already mapped to this process by the HAB process
    *buff_shared = (void*) arg.index;
#elif  __INTEGRITY
	uint64_t port_id = arg.index;
    ret = pmem_map_from_id(/*PMEM_GRAPHICS_COMMAND_ID*/PMEM_MDP_ID, port_id, buff_shared);//mmid matters if this is loopback testing
    if (ret){
        UHAB_LOG("pmem_map_from_id failed, address = 0x%llx, vcid %X\n", port_id, handle);
        ret = -EINVAL;
    }
#else
    if (flags & HABMM_EXPIMP_FLAGS_FD) {
        *buff_shared = *(void**)&arg.kva;
        ret = 0;
    } else {
        void *pvoid;

        pvoid =  mmap(0, size_bytes, PROT_READ|PROT_WRITE, MAP_SHARED, fd, arg.index);

        if (MAP_FAILED == pvoid) { // -1
            UHAB_LOG("failed to mmap %d bytes at index %llx on fd %d, vcid %X, error code %s(%d)\n", size_bytes, (unsigned long long)arg.index, fd, handle, strerror(errno), errno);
            ret = -ENOMEM;
        } else {
            UHAB_DBG("mmap works for %d bytes at index %llx, vcid %X\n", size_bytes, (unsigned long long)arg.index, handle);

            ret = 0;

            node = (struct imp_mmap_node *)malloc(sizeof(struct imp_mmap_node));
            if (node) {
                node->exportid = export_id;
                node->grantidx = arg.index;
                node->uaddr = pvoid;
                node->size = size_bytes;

                list_add(node, &gimp_mmap_list);
                *buff_shared = pvoid;
            } else {
                ret = munmap(pvoid, size_bytes);
                if (ret)
                    UHAB_LOG("failed to munmap, return %d, error code %s(%d)\n", ret, strerror(errno), errno);

                UHAB_LOG("failed to malloc imp_mmap_node\n");
                ret = -ENOMEM;
            }
        }
    }
#endif

    UHAB_TRACE_OUT(handle);
    return ret;
}

/*
Params:

in handle - communication channel created by habmm_socket_open
in export_id - all resource allocated with export_id are to be freed
in flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_unexport(int32_t handle, uint32_t export_id, uint32_t flags) {
	int32_t ret;
	struct hab_unexport arg = {
		.vcid = handle,
        .exportid = export_id,
        .flags = flags
	};
	UHAB_TRACE_IN(handle);

	ret = hab_DevCtl(IOCTL_HAB_VC_UNEXPORT, &arg, sizeof(arg), NULL, 0, NULL, 0, NULL, 0);
	if (ret)
		UHAB_LOG("%s: return %d on exportid %d, vcid %X, error code %s\n", __FUNCTION__, ret, arg.exportid, handle, strerror(errno));

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
Params:

in handle - communication channel created by habmm_socket_open
in export_id - received when exporter sent its exporting ID through
               habmm_socket_send() previously
in buff_shared - received from habmm_import() together with export_id
in flags - future extension

Return:
status (success/failure)
*/
int32_t habmm_unimport(int32_t handle, uint32_t export_id, void *buff_shared, uint32_t flags) {
	int32_t ret;
	struct hab_unimport arg = {
		.vcid = handle,
		.exportid = export_id,
		.flags = flags
	};

#if !defined(__QNXNTO__) && !defined(__INTEGRITY)
	struct imp_mmap_node * node = list_detach_by_uaddr(buff_shared, &gimp_mmap_list);

	UHAB_TRACE_IN(handle);

	if (node) {
		ret = munmap(buff_shared, node->size);

		UHAB_DBG("%s: return %d on %p, size %d, exportid %d, vcid %X\n", __FUNCTION__, ret, node->uaddr, node->size, node->exportid, handle);

		if (ret) {
			UHAB_LOG("%s: return %d on %p, size %d, exportid %d, vcid %X, error code %s\n", __FUNCTION__, ret, node->uaddr, node->size, node->exportid, handle, strerror(errno));
			UHAB_LOG("failed to unmap address %p, but continue to unimport in khab!\n", node->uaddr);
		}
	}
#else
	UHAB_TRACE_IN(handle);
#endif

	ret = hab_DevCtl(IOCTL_HAB_VC_UNIMPORT, &arg, sizeof(arg), NULL, 0, NULL, 0, NULL, 0);
//	if (ret)
//		ret = -errno;

#if !defined(__QNXNTO__) && !defined(__INTEGRITY)
    if(node)
	    free(node);
#endif

	if (ret) {
		UHAB_LOG("%s: failed to unimport error %d(%s), vcid %X\n", __FUNCTION__, ret, strerror(ret), handle);
	} else {
	    UHAB_DBG("%s: unimport successful: exportid %d, vcid %X\n", __FUNCTION__, export_id, handle);
    }

	UHAB_TRACE_OUT(handle);
	return ret;
}

/*
 * Description:
 *
 * Query various information of the opened hab socket.
 *
 * Params:
 *
 * in handle - communication channel created by habmm_socket_open
 * in habmm_socket_info - retrieve socket information regarding local and remote
 *                        VMs
 * in flags - future extension
 *
 * Return:
 * status (success/failure)
 *
 */
int32_t habmm_socket_query(int32_t handle, struct hab_socket_info *info,
		uint32_t flags)
{
	int32_t ret;
	char names[sizeof(info->vmname_remote)*2]; /* single retrieving buffer for two names */

	struct hab_info arg = {
		.vcid = handle,
		.names = (uintptr_t) names,
		.namesize = sizeof(names),
		.flags = flags
	};

	UHAB_TRACE_IN(handle);

	if (!info) {
		ret = -EINVAL;
		UHAB_LOG("%s: return %d on query vcid %X, NULL parameter\n", __FUNCTION__, ret, handle);
		return ret;
	}

	ret = hab_DevCtl(IOCTL_HAB_VC_QUERY, &arg, sizeof(arg), NULL, 0,
					 &arg.ids, sizeof(arg.ids), (void*)(uintptr_t)arg.names, arg.namesize);
	if (ret) {
		UHAB_LOG("%s: return %d on query vcid %X, error code %s\n", __FUNCTION__, ret, handle, strerror(errno));
//		ret = -errno;
	} else {
		info->vmid_local = arg.ids & 0xFFFFFFFF;
		info->vmid_remote = (arg.ids & 0xFFFFFFFF00000000UL) >> 32;

#if !defined(__INTEGRITY)
		// ToDo GHS: names needs to be returned properly. now it is conflicting with recv's size
		if (arg.namesize) {
			char *nm = (char*)(uintptr_t)arg.names;
			strlcpy(info->vmname_local, nm, sizeof(info->vmname_local));
			strlcpy(info->vmname_remote, &nm[sizeof(info->vmname_remote)], sizeof(info->vmname_remote)); // single buffer for two names
		} else {
			info->vmname_remote[0] = 0;
			info->vmname_local[0] = 0;
		}
#else
		// ToDo GHS: names needs to be returned properly. now it is conflicting with recv's size
		info->vmname_remote[0] = 0;
		info->vmname_local[0] = 0;		
#endif
	}

	UHAB_TRACE_OUT(handle);
	return ret;
}
