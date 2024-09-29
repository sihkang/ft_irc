논블로킹이란?

* 기본적으로 파일 디스크립터(소켓, 파일 등)에 대해 read/write 작업을 수행할 때,
작업이 완료될 때까지 프로그램이 기다리는 방식이 블로킹 모드입니다.
논블로킹 모드에서는 작업이 즉시 완료되지 않더라도 프로그램을 계속 실행합니다.

	읽기 작업(read)

		* 블로킹 모드 : 'read()' 함수가 데이터가 읽힐 때까지 프로그램 실행을 멈춥니다.
		만약 데이터가 없으면 계속 대기합니다.

		* 논블로킹 모드 : 'read()' 함수가 즉시 반환됩니다. 
		만약 읽을 데이터가 없으면 '-1' 반환하고 errno를 EAGAIN, EWOULDBLOCK으로 설정


	쓰기 작업(write)

		* 블로킹 모드 : 'write()' 함수가 모든 데이터가 기록될 때까지 프로그램 실행을 멈춥니다.
		만약 출력 버퍼가 가득 차 있다면, 공간이 생길때까지 대기합니다.

		* 논블로킹 모드 : 'write()' 함수가 즉시 반환됩니다.
		만약 버퍼에 공간이 없다면 '-1' 반환하고 errno를 EAGAIN, EWOULDBLOCK으로 설정

※ MacOS에서는 fcntl(fd, F_SETFL, O_NONBLOCK) 을 사용하여 논블로킹 모드로 설정

----------------------------------------------------------------------------------------------------------------------------------------
socket 관련 함수 정리 및 비유

socket() : 클라이언트로부터의 연결 요청을 받을 수 있도록 socket 함수를 이용하여 네트워크 연결 장치인 소켓을 생성하는 함수
- 전화기 준비
- int socket(int domain, int type, int protocol);
* domain : 소켓이 사용할 주소 패밀리를 지정합니다.
	※ ex) AF_INET, AF_INET6, AF_UNIX 등
* type : 소켓의 유형을 지정합니다.
	※ ex) SOCK_STREAM 등
* protocol : 사용할 프로토콜을 지정합니다.
	※ ex) IPPROTO_TCP 등
* 반환값
	* 성공 시 : 새로 생성된 소켓의 파일 디스크립터
	* 실패 시 : -1

bind() : 네트워크 연결 장치인 소켓을 xxxx번 포트에 연결하는 함수
- 전화기를 xxxx번 콘센트에 물리적으로 연결
- int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
* sockfd : 'socket()' 함수로 생성된 소켓 파일 디스크립터입니다.
* addr : 바인딩할 로컬 주소를 나타내는 'sockaddr' 구조체에 대한 포인터입니다.
* addrlen : 'addr' 구조체의 크기를 바이트 단위로 지정
* 반환값
	* 성공 시 : '0'을 반환
	* 실패 시 : '-1'을 반환

listen() : 소켓으로 통신이 이루어지도록 운영체제에 개통을 요청하는 함수
- 전화국에 개통 요청
- int listen(int sockfd, int backlog)
* sockfd : 서버 소켓을 나타내는 파일 디스크립터. 'socket()' 함수를 호출하여 생성된 소켓을 지정
* backlog : 대기열의 최대 길이. 연결 요청이 대기할 수 있는 최대 클라이언트 수를 지정합니다.
	※ SOMAXCONN으로 보통 설정되는데 SOMAXCONN은 시스템이 허용하는 최대 연결 요청 수를 나타내는 상수

accept() : 클라이언트로부터의 연결 요청을 받아들이는 함수
- 전화를 기다리다가 전화가 오면 받음
- int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
* sockfd : 'socket()' 함수로 생성된 서버 소켓의 파일 디스크립터
* addr : 클라이언트의 주소 정보를 저장할 구조체에 대한 포인터입니다.
클라이언트의 주소 정보가 필요하지 않을 경우에는 'NULL'을 전달할 수 있습니다.
* addrlen : 'addr' 구조체의 크기를 나타내는 포인터입니다.
호출 전에 이 값은 할당된 구조체의 크기로 초기화되어야 합니다.
* 반환값
	* 성공 시 : 클라이언트와의 통신을 위한 새로운 소켓의 파일 디스크립터를 반환
	* 실패 시 : '-1'을 반환

read()/write() : 클라이언트에게 서비스를 제공
- 전화 통화

close() : 클라이언트와의 연결을 종료
- 전화 끊기

----------------------------------------------------------------------------------------------------------------------------------------
poll 이란?

- 'poll' 시스템 호출은 여러 파일 디스크립터를 모니터링하여, 
하나 이상의 파일 디스크립터에서 이벤트(예: 읽기, 쓰기, 오류)가 발생했는지 확인하는데 사용됩니다.
이는 서버가 여러 클라이언트와 동시에 통신할 수 있게 해주기 때문에 매우 유용합니다.

- 'poll' 의 동작 방식
* 'poll' 시스템 호출은 'struct pollfd' 배열을 사용합니다.

struct pollfd{
	int fd; 		// 파일 디스크립터
	short events;	// 모니터링할 이벤트 (입력, 출력, 오류 등)
	short revents;	// 발생한 이벤트 (입력, 출력, 오류 등)
}

short events의 필드값
* POLLIN : 읽기 가능한 데이터가 있음
* POLLRDNORM : 일반 데이터 읽기 가능
* POLLRDBAND : 우선순위가 높은 데이터 읽기 가능
* POLLPRI : 긴급한 읽기 가능 데이터가 있음
* POLLOUT : 쓰기 가능
* POLLWRNORM : 일반 데이터 쓰기 가능

- poll() 함수

* int poll(struct pollfd *fds, nfds_t nfds, int timeout);
	* fds : 감시할 파일 디스크립터의 배열을 가리키는 포인터입니다.
	각 파일 디스크립터는 'pollfd' 구조체로 표현됩니다.
	* nfds : 감시할 파일 디스크립터의 수입니다.
	* timeout : 대기할 시간 (밀리초) 입니다.
		* 'timeout > 0' : 지정된 밀리초 동안 대기합니다.
		* 'timeout == 0' : 즉시 반환합니다.
		* 'timeout == -1' : 무한정 대기합니다.

std::vector <struct pollfd> poll_fds;
* 여러 파일 디스크립터를 모니터링하기 위해 'pollfd' 구조체를 저장하는 벡터

----------------------------------------------------------------------------------------------------------------------------------------
struct addrinfo 구조체란?

struct addrinfo 구조체

- 네트워크 프로그래밍에서 주소 정보를 저장하기 위해 사용되는 구조체입니다.
- 이 구조체는 'getaddrinfo()' 함수와 함께 사용되어 호스트 이름이나 서비스 이름을 IP 주소와
포트 번호로 변환하는데 사용됩니다.
'getaddrinfo()' 함수는 DNS 조회, 주소 변환 등을 수행하며, 결과를 'addrinfo' 구조체의 리스트로 반환합니다.

struct addrinfo
{
	int				ai_flags;		// 옵션 플래그
	int				ai_family;		// 주소 패밀리 (AF_INET, AF_INET6 등)
	int				ai_socktype;	// 소켓 타입 (SOCK_STREAM, SOCK_DGRAM 등)
	int				ai_protocol;	// 프로토콜 (IPPROTO_TCP, IPPROTO_UDP 등)
	size_t			ai_addrlen;		// ai_addr의 길이
	struct sockaddr	*ai_addr;		// 소켓 주소 (구조체)
	char			*ai_canonname;	// 정식 이름
	struct addrinfo	*ai_next; 		// 다음 구조체에 대한 포인터 (다중 결과일 때 사용)
}

- ai_flags : 'getaddrinfo()' 함수의 동작을 제어하는 플래그입니다.
예를 들어, 'AI_PASSIVE' 는 서버 소켓에 사용할 수 있는 주소를 반환하도록 합니다.
- ai_family : 주소 패밀리를 지정합니다. IPv4를 사용할 경우 'AF_INET', IPv6은 'AF_INET6',
IPv4와 IPv6 중 어떤 것이든 상관없이 주소를 얻고자 할때는 'AF_UNSPEC'을 쓴다.
- ai_socktype : 소켓 타입을 지정합니다. TCP을 사용할 경우 'SOCK_STREAM'을 설정합니다.
- ai_protocol : 사용할 프로토콜을 지정합니다. TCP는 'IPPROTO_TCP'을 설정합니다.
- ai_addrlen : 'ai_addr' 필드의 길이를 바이트 단위로 나타냅니다.
- ai_addr: 소켓 주소를 나타내는 'sockaddr' 구조체에 대한 포인터입니다.
- ai_cannonname : 호스트의 정식 이름을 나타냅니다. 이 필드는 'AI_CANONNAME' 플래그가 설정된
경우에만 유효합니다.
- ai_next : 다음 'addrinfo' 구조체를 가리키는 포인터입니다. 이 필드를 통해 연결 리스트 형태로
결과를 탐색할 수 있습니다.

----------------------------------------------------------------------------------------------------------------------------------------
getaddrinfo() 함수란?

- 네트워크 프로그래밍에서 호스트 이름과 서비스 이름(또는 포트 번호)을 IP 주소와 포트 번호로 변환하는데 사용
- 이 함수는 DNS 조회, 주소 변환 등을 수행하며 결과를 'addrinfo' 구조체의 리스트로 반환합니다.

int getaddrinfo(const char *node, const char *service, const strcut addrinfo *hints,
				struct addrinfo **res);

1. 매개 변수 설명

- node : 호스트 이름(예: 'www.example.com')이나 IP 주소 문자열(예: '192.168.1.1')입니다.
'NULL' 로 설정하면 로컬 호스트의 주소를 사용합니다.
- service : 서비스 이름(예: "http")이나 포트 번호 문자열(예: "80")입니다.
- hints : 호출자가 원하는 주소의 유형을 지정하는 'addrinfo' 구조체입니다.
이 구조체를 통해 주소 패밀리(AF_INET, AF_INET6), 소켓 타입(SOCK_STREAM)등의 힌트를 제공합니다.
- res : 'addrinfo' 구조체에 대한 포인터의 주소인데 함수가 반환한 결과를 가리킵니다.

2. 반환값

- 성공 시 0을 반환합니다.
- 오류 발생 시 비-제로 오류 코드를 반환하며, 오류 코드는 'gai_strerror' 함수로 문자열 설명을 얻을 수 있습니다.

----------------------------------------------------------------------------------------------------------------------------------------
setsockopt() 함수란?

- 소켓 옵션을 설정하는데 사용되는 시스템 호출입니다.
- 이 함수는 소켓의 동작을 제어하기 위해 다양한 옵션을 설정할 수 있게 해줍니다.
ex) 소켓의 재사용 여부, 송수신 타임아웃, 버퍼 크기 등

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
- sockfd : 소켓 파일 디스크립터. 'socket()' 함수로 생성된 소켓을 지정
- level : 소켓 레벨 옵션을 설정할 때는 'SOL_SOCKET'을 사용합니다.
- optname : 설정할 옵션의 이름을 지정합니다.
	* SO_REUSEADDR : 이미 사용중인 주소에 소켓을 바인딩할 수 있도록 허용합니다.
	이는 서버 프로그램이 갑작스러운 종료 후 빠르게 재시작할 때 유용합니다.
	* SO_KEEPALIVE : 주기적으로 패킷을 전송하여 연결이 유지되고 있는지 확인합니다.
	이 옵션은, TCP 연결이 끊어졌는지 여부를 자동으로 확인할 수 있습니다.
	* SO_LINGER : 'close()' 호출 후, 소켓을 닫기 전에 잔여 데이터를 보내도록
	설정합니다.
	* SO_RCVBUF : 수신 버퍼의 크기를 설정
	* SO_SNDBUF : 송신 버퍼의 크기를 설정
	* SO_RCVTIMEO : 수신 타임아웃을 설정
	* SO_SNDTIMEO : 송신 타임아웃을 설정
- optval : 옵션 값에 대한 포인터입니다. 이는 설정하려는 옵션의 값을 가리킵니다.
- optlen : 'optval'의 크기를 바이트 단위로 지정합니다.

반환값 
* 성공 시 0을 반환
* 실패 시 -1을 반환

----------------------------------------------------------------------------------------------------------------------------------------
'sockaddr_in' 구조체란?

- IPv4 주소를 나타내기 위해 사용되는 구조체입니다.
주로 소켓 프로그래밍에서 사용되며, 소켓을 특정 IP 주소와 포트에 바인딩하거나 연결할 때 사용

struct sockaddr_in {
    sa_family_t    sin_family; 
    in_port_t      sin_port;   
    struct in_addr sin_addr;   
    char           sin_zero[8]; 
};

* sin_family : 주소 패밀리를 나타냅니다. IPv4 주소를 사용하므로 항상 'AF_INET'으로 설정됩니다.
* sin_port : 포트 번호를 저장합니다. 네트워크 바이트 순서로 저장해야 합니다.
* sin_addr : IP 주소를 저장하는 in_addr 구조체 입니다.
* sin_zero : 구조체의 크기를 sockaddr 구조체와 동일하게 맞추기 위한 패딩입니다.
항상 0으로 채워집니다.

