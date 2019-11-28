// libuvdemo.cpp : Defines the entry point for the application.
//

#include "uvsocket.h"
#include "uv.h"
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <deque>
#include <string>

typedef struct {
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;

typedef enum {
	TCP = 0,
	UDP,
	PIPE
} stream_type;

/* Die with fatal error. */
#define FATAL(msg)                                        \
  do {                                                    \
    fprintf(stderr,                                       \
            "Fatal error in %s on line %d: %s\n",         \
            __FILE__,                                     \
            __LINE__,                                     \
            msg);                                         \
    fflush(stderr);                                       \
    abort();                                              \
  } while (0)

/* Have our own assert, so we are sure it does not get optimized away in
 * a release build.
 */
#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)

#define TEST_PORT 9123
#define TEST_PIPENAME "/tmp/uv-test-sock"
#define notify_parent_process() ((void) 0)

static uv_loop_t* loop;

static int server_closed;
static stream_type serverType;
static uv_tcp_t tcpServer;
static uv_udp_t udpServer;
static uv_pipe_t pipeServer;
static uv_handle_t* server;

std::map<int, std::map<int, std::map<uv_stream_t*, uv_stream_t*>>> g_map;
typedef enum {
	CMD_NUL = 0,
	CMD_REG = 1,
	CMD_MSG = 2,
	CMD_MAX,
}CMD_TYPE; 
typedef enum {
	SRV_NUL = 0,
	SRV_SRV = 1,
	SRV_CLI = 2,
	SRV_GAT = 3,
	SRV_MAX
}SRV_TYPE;

#pragma pack(push,1)
typedef struct  {
	int bsn;
	int cmd;
	int src_type;
	int64_t src_fd;
	int dst_type;
	int64_t dst_fd;
	int data_len;
	void* p_data;
	void print_data()
	{
		printf("bsn=%d,cmd=%d,src_type=%d,src_fd=%lld,dst_type=%d,dst_fd=%lld,data_len=%d\n",
			this->bsn,this->cmd,this->src_type,this->src_fd,this->dst_type,this->dst_fd,this->data_len);
		if (this->data_len > 0)
		{
			for (size_t i = 0; i < this->data_len; i++)
			{
				if (i % 16)
				{
					printf("\n");
				}
				printf("%02X ", (uint8_t)((const char *)(this->p_data))[i]);
			}
		}
	}
}uvdata_packet_header;
#pragma pack(pop)

static void after_write(uv_write_t* req, int status);
static void after_read(uv_stream_t*, ssize_t nread, const uv_buf_t* buf);
static void on_close(uv_handle_t* peer);
static void on_server_close(uv_handle_t* handle);
static void on_connection(uv_stream_t*, int status);


static void after_write(uv_write_t* req, int status) {
	write_req_t* wr;

	printf("after_write\n");

	/* Free the read/write buffer and the request */
	wr = (write_req_t*)req;
	free(wr->buf.base);
	free(wr);

	if (status == 0)
		return;

	fprintf(stderr,
		"uv_write error: %s - %s\n",
		uv_err_name(status),
		uv_strerror(status));
}


static void after_shutdown(uv_shutdown_t* req, int status) {
	printf("after_shutdown\n");
	uv_close((uv_handle_t*)req->handle, on_close);
	free(req);
}

/*if (buf->base[0] == 'L')
{
	std::map<int, std::map<int, std::map<int, uv_stream_t*>>> g_map;
	g_map.insert({
			{ 00, //业务码
				{
					{0,{}},//服务器
					{1,{}},//客户端
					{2,{}},//集群端
				}
			}
		});
	g_map.at(00).at(1).insert({ handle->u.fd, handle });
}*/
static int parse_request(uv_stream_t* handle,
	ssize_t nread,
	const uv_buf_t* buf)
{
	printf("parse_request(%d)\n", nread);
	uvdata_packet_header* p_data = (uvdata_packet_header*)buf->base;
	if (p_data)
	{
		p_data->print_data();
		switch (p_data->cmd)
		{
		case CMD_REG:
		{
			if (p_data->src_type > SRV_NUL&& p_data->src_type < SRV_MAX)
			{
				g_map.insert({
						{ p_data->bsn, //业务码
							{
								{SRV_SRV,{}},//服务器
								{SRV_CLI,{}},//客户端
								{SRV_GAT,{}},//集群端
							}
						}
					});
				g_map.at(p_data->bsn).at(p_data->src_type).insert({ handle, handle });

				write_req_t* wr = (write_req_t*)malloc(sizeof * wr);
				ASSERT(wr != NULL);
				wr->buf = uv_buf_init(buf->base, nread);
				memcpy(wr->buf.base + 12, handle, sizeof(handle));
				if (uv_write(&wr->req, handle, &wr->buf, 1, after_write)) {
					FATAL("uv_write failed");
				}
				return 1;
			}
		}
		break;
		case CMD_MSG:
		{
			if (p_data->src_type > SRV_NUL&& p_data->src_type < SRV_MAX
				&&
				p_data->dst_type > SRV_NUL&& p_data->dst_type < SRV_MAX)
			{

			}
			write_req_t* wr = (write_req_t*)malloc(sizeof * wr);
			ASSERT(wr != NULL);
			wr->buf = uv_buf_init(buf->base, nread);
			memcpy(wr->buf.base + 12, g_map.at(p_data->bsn).at(p_data->src_type).begin()->first, sizeof(handle));
			if (uv_write(&wr->req, handle, &wr->buf, 1, after_write)) {
				FATAL("uv_write failed");
			}
			return 1;
		}
		break;
		default:
			break;
		}
	}
	return 0;
}
static void after_read(uv_stream_t* handle,
	ssize_t nread,
	const uv_buf_t* buf) {
	int i;
	write_req_t* wr;
	uv_shutdown_t* sreq;
	printf("after_read(%d)\n", nread);

	if (nread < 0) {
		/* Error or EOF */
		ASSERT(nread == UV_EOF);

		free(buf->base);
		sreq = (uv_shutdown_t*)malloc(sizeof * sreq);
		ASSERT(0 == uv_shutdown(sreq, handle, after_shutdown));
		return;
	}

	if (nread == 0) {
		/* Everything OK, but nothing read. */
		free(buf->base);
		return;
	}

	/*
	 * Scan for the letter Q which signals that we should quit the server.
	 * If we get QS it means close the stream.
	 */
	if (!server_closed) {
		/*for (i = 0; i < nread; i++) {
			if (buf->base[i] == 'Q') {
				if (i + 1 < nread && buf->base[i + 1] == 'S') {
					free(buf->base);
					uv_close((uv_handle_t*)handle, on_close);
					return;
				}
				else {
					uv_close(server, on_server_close);
					server_closed = 1;
				}
			}
		}*/
	}

	if (parse_request(handle, nread, buf) == 1)
	{
		return;
	}
	wr = (write_req_t*)malloc(sizeof * wr);
	ASSERT(wr != NULL);
	wr->buf = uv_buf_init(buf->base, nread);

	if (uv_write(&wr->req, handle, &wr->buf, 1, after_write)) {
		FATAL("uv_write failed");
	}
}


static void on_close(uv_handle_t* peer) {
	printf("on_close\n");
	free(peer);
}


static void echo_alloc(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf) {
	printf("echo_alloc(suggested_size=%d)\n", suggested_size);
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}


static void on_connection(uv_stream_t* server, int status) {
	uv_stream_t* stream;
	int r;

	if (status != 0) {
		fprintf(stderr, "Connect error %s\n", uv_err_name(status));
	}
	ASSERT(status == 0);
	printf("on_connection\n");
	switch (serverType) {
	case TCP:
		stream = (uv_stream_t*)malloc(sizeof(uv_tcp_t));
		ASSERT(stream != NULL);
		r = uv_tcp_init(loop, (uv_tcp_t*)stream);
		ASSERT(r == 0);
		break;

	case PIPE:
		stream = (uv_stream_t*)malloc(sizeof(uv_pipe_t));
		ASSERT(stream != NULL);
		r = uv_pipe_init(loop, (uv_pipe_t*)stream, 0);
		ASSERT(r == 0);
		break;

	default:
		ASSERT(0 && "Bad serverType");
		abort();
	}

	/* associate server with stream */
	stream->data = server;

	r = uv_accept(server, stream);
	ASSERT(r == 0);

	r = uv_read_start(stream, echo_alloc, after_read);
	ASSERT(r == 0);
}


static void on_server_close(uv_handle_t* handle) {
	printf("on_server_close\n");
	ASSERT(handle == server);
	exit(0);
}


static void on_send(uv_udp_send_t* req, int status);


static void on_recv(uv_udp_t* handle,
	ssize_t nread,
	const uv_buf_t* rcvbuf,
	const struct sockaddr* addr,
	unsigned flags) {
	uv_udp_send_t* req;
	uv_buf_t sndbuf;

	printf("on_recv(%d)\n", nread);
	ASSERT(nread > 0);
	ASSERT(addr->sa_family == AF_INET);

	req = (uv_udp_send_t*)malloc(sizeof(*req));
	ASSERT(req != NULL);

	sndbuf = *rcvbuf;
	ASSERT(0 == uv_udp_send(req, handle, &sndbuf, 1, addr, on_send));
}


static void on_send(uv_udp_send_t* req, int status) {
	printf("on_send\n");
	ASSERT(status == 0);
	free(req);
}


static int tcp4_echo_start(int port) {
	struct sockaddr_in addr;
	int r;

	ASSERT(0 == uv_ip4_addr("0.0.0.0", port, &addr));

	server = (uv_handle_t*)&tcpServer;
	serverType = TCP;

	r = uv_tcp_init(loop, &tcpServer);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Socket creation error\n");
		return 1;
	}

	r = uv_tcp_bind(&tcpServer, (const struct sockaddr*) & addr, 0);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Bind error\n");
		return 1;
	}

	r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 1;
	}

	return 0;
}


static int tcp6_echo_start(int port) {
	struct sockaddr_in6 addr6;
	int r;

	ASSERT(0 == uv_ip6_addr("::1", port, &addr6));

	server = (uv_handle_t*)&tcpServer;
	serverType = TCP;

	r = uv_tcp_init(loop, &tcpServer);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Socket creation error\n");
		return 1;
	}

	/* IPv6 is optional as not all platforms support it */
	r = uv_tcp_bind(&tcpServer, (const struct sockaddr*) & addr6, 0);
	if (r) {
		/* show message but return OK */
		fprintf(stderr, "IPv6 not supported\n");
		return 0;
	}

	r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
	if (r) {
		/* TODO: Error codes */
		fprintf(stderr, "Listen error\n");
		return 1;
	}

	return 0;
}


static int udp4_echo_start(int port) {
	int r;

	server = (uv_handle_t*)&udpServer;
	serverType = UDP;

	r = uv_udp_init(loop, &udpServer);
	if (r) {
		fprintf(stderr, "uv_udp_init: %s\n", uv_strerror(r));
		return 1;
	}

	r = uv_udp_recv_start(&udpServer, echo_alloc, on_recv);
	if (r) {
		fprintf(stderr, "uv_udp_recv_start: %s\n", uv_strerror(r));
		return 1;
	}

	return 0;
}


static int pipe_echo_start(char* pipeName) {
	int r;

#ifndef _WIN32
	{
		uv_fs_t req;
		uv_fs_unlink(NULL, &req, pipeName, NULL);
		uv_fs_req_cleanup(&req);
	}
#endif

	server = (uv_handle_t*)&pipeServer;
	serverType = PIPE;

	r = uv_pipe_init(loop, &pipeServer, 0);
	if (r) {
		fprintf(stderr, "uv_pipe_init: %s\n", uv_strerror(r));
		return 1;
	}

	r = uv_pipe_bind(&pipeServer, pipeName);
	if (r) {
		fprintf(stderr, "uv_pipe_bind: %s\n", uv_strerror(r));
		return 1;
	}

	r = uv_listen((uv_stream_t*)&pipeServer, SOMAXCONN, on_connection);
	if (r) {
		fprintf(stderr, "uv_pipe_listen: %s\n", uv_strerror(r));
		return 1;
	}

	return 0;
}


static int tcp4_echo_server() {
	loop = uv_default_loop();

	if (tcp4_echo_start(TEST_PORT))
		return 1;
	fprintf(stdout, "tcp4_echo_start 0.0.0.0:%d\n", TEST_PORT);
	notify_parent_process();
	uv_run(loop, UV_RUN_DEFAULT);
	return 0;
}


static int tcp6_echo_server() {
	loop = uv_default_loop();

	if (tcp6_echo_start(TEST_PORT))
		return 1;

	notify_parent_process();
	uv_run(loop, UV_RUN_DEFAULT);
	return 0;
}


static int pipe_echo_server() {
	loop = uv_default_loop();

	if (pipe_echo_start(TEST_PIPENAME))
		return 1;

	notify_parent_process();
	uv_run(loop, UV_RUN_DEFAULT);
	return 0;
}


static int udp4_echo_server() {
	loop = uv_default_loop();

	if (udp4_echo_start(TEST_PORT))
		return 1;

	notify_parent_process();
	uv_run(loop, UV_RUN_DEFAULT);
	return 0;
}

int main(int argc, char** argv)
{
	return tcp4_echo_server();
}