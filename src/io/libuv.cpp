#include <cassert>
#include "libuv.h"
#include "../routes.h"
#include "string.h"


using namespace std;

uv_loop_t *loop;
struct sockaddr_in addr;
const int  default_backlog = 128;

// 申请连接内存
void read(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*) malloc(suggested_size);  
  buf->len = suggested_size; 
}

// 发送数据
void write(uv_write_t *req, int status) {  
    if (status) fprintf(stderr, "Write error %s\n", uv_strerror(status));
    free(req);  
} 

// 发送数据
void send(uv_stream_t *client, ssize_t nread, const uv_buf_t *buff) {
  printf("echo_read:1"); 
  if(nread < 0) {
    if(nread != UV_EOF) fprintf(stderr, "read error %s \n", uv_err_name(nread));
    uv_close((uv_handle_t*) client, NULL);
  } else {
		uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t)); 
    //uv_buf_t wrbuf = uv_buf_init(buff->base, nread);

    printf("echo_read:1");
    // 转发到交易中心
    Routes *route = new Routes();
    char *futures_str = route->getCThostFtdcTraderApi();
    printf("echo_read:2");

    uv_buf_t wrbuf = uv_buf_init(futures_str, strlen(futures_str));
    
    
    uv_write(req, client, &wrbuf, 1, write);
	}

  printf("echo_read:%s\r\n", buff->base);
  
  if(buff->base) free(buff->base);

}

void onConnect(uv_stream_t *server, int status) {
  if (status < 0) {   
    fprintf(stderr, "one New connection error %s\n", uv_strerror(status));  
    return;  
  }   
        
  uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));  
  uv_tcp_init(loop, client);  
  
  if (uv_accept(server, (uv_stream_t*) client) == 0) {   
    uv_read_start((uv_stream_t*) client, read, send);  
  } else {   
    uv_close((uv_handle_t*) client, NULL);  
  }   
  
  printf("one new connection have been OK, status:%d\r\n", status);   

}


int IIOService::listen(const char *host, int port) {
	printf("######## start ftp service HOST: %s PORT: %d ################### \n", host, port);  
	
	loop = uv_default_loop(); 
  uv_tcp_t server;  
  uv_tcp_init(loop, &server);  
  uv_ip4_addr(host, port, &addr); 
  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  
	int listen_status = uv_listen((uv_stream_t*) &server, default_backlog, onConnect);   
	if (listen_status) {   
  	fprintf(stderr, "Listen error %s\n", uv_strerror(listen_status));  
  	return 1;  
  }   
  
	return uv_run(loop, UV_RUN_DEFAULT);

}

