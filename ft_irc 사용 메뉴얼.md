## 1. 서버 동작

```
./Ircserv 8080 1234
```

각각, 포트번호와 서버 비밀번호를 기입하여 실행해준다.

## 2. irssi 실행
실행 이전, 클라이언트가 서버와 별개의 환경에서 실행되어지도록 도커로 클라이언트 환경을 세팅한다.

```
 // 우분투 도커 이미지 실행
docker run --rm -it ubuntu /bin/bash


// 도커 내 환경 세팅 (상용 서버, 클라이언트, 그리고 그 둘이 주고받는 메시지를 확인할 tcpflow 설치)
apt update 
apt install -y inspircd irssi tcpflow vim


// 도커 호스트 IP로 연결 , 포트 , 닉네임, 패스워드 기입
irssi -c host.docker.internal -p <port> -n <nickname> --password <server_pass> 
 
```

inspircd는 상용 서버로, 우리가 프로그래밍할 ft_irc의 롤모델이라고 생각하면서 참고하도록 하자.
실제 RFC문서의 프로토콜 규약과 서버의 세팅이 다른 경우가 허다했다...

상황과 시대의 흐름에 따라 통신과정도 달라지면서 프로토콜 규약과 다른 것들이 존재하니, 무조건 프로토콜대로만 구현하기보다는 실제로는 어떻게 사용되고 있는지를 참고하면 좋은 듯하다.


###### 상용 서버 inspircd 에 irssi로 접속하기.
```
mkdir -p /var/run/inspircd
inspircd --runasroot
irssi -c localhost -p 6667 -n myNick
```

###### tcpflow로 서버-클라 간 메시지 확인하기
```
# container localhost (inspircd)
tcpflow -i lo port 6667 -c 

# local device (ft_irc)
tcpflow host host.docker.internal and port 8080 -c
```


## 3. 서버 - 클라 연결
## irssi 첫 연결 시

서버 - 클라가 첫 연결된 순간 tcpflow 에서는 다음과 같은 기록이 남는다.

```
tcpflow: listening on lo
127.000.000.001.06667-127.000.000.001.54642: :irc.local NOTICE * :*** Raw I/O logging is enabled on this server. All messages, passwords, and commands are being recorded.
:irc.local NOTICE * :*** Looking up your hostname...

127.000.000.001.54642-127.000.000.001.06667: CAP LS 302

127.000.000.001.54642-127.000.000.001.06667: JOIN :

127.000.000.001.06667-127.000.000.001.54642: :irc.local 451 * JOIN :You have not egistered.
```
: `inspircd`에서는 가볍게 클라이언트에 클라이언트의 모든 기록이 저장된다는 주의를 보내며 시작한다.
tcpflow에 나타나는 메시지의 구조는 다음과 같다. 

> [from IP주소.포트번호]-[to IP주소.포트번호] : "송수신된 데이터"

여기서는 54642가 `irssi` 클라이언트의 포트가 된다.   
이후 `CAP LS 302` 메시지가 있는데, 보안사항에 대해 서버 - 클라 간 메시지를 주고받는다고 함.

1. `CAP`: 이는 IRC 프로토콜의 Capability Negotiation 기능을 나타냅니다. 클라이언트와 서버는 서로 지원하는 확장 기능을 협상할 수 있습니다.
    
2. `LS`: 이는 "List"의 약어로, 서버가 클라이언트에게 자신이 지원하는 기능 목록을 보내겠다는 의미입니다.
    
3. `302`: 이는 IRC 프로토콜의 응답 코드입니다. 302 코드는 "List of server capabilities"를 의미합니다.
    

따라서 이 메시지는 **IRC 서버가 클라이언트에게 자신이 지원하는 기능 목록을 보내겠다는 것**을 나타냅니다. 클라이언트는 이 목록을 보고 자신이 필요로 하는 기능을 선택하여 서버에 요청할 수 있습니다.

클라이언트는 해당 서버로의 `JOIN :` 메시지를 송부하여 접근한다.
그리고 해당 메시지의 답으로, "127.000.000.001.06667-127.000.000.001.54642: :irc.local 451 * JOIN :You have not egistered." 라는 문구를 보낸다.

일반적으로 IRC 서버는 클라이언트가 JOIN 명령을 보냈을 때 사용자 등록이 되어 있지 않다면 **451 응답 코드와 함께 NICK, USER, PASS 명령을 보내 사용자 등록 절차를 진행하도록 요구**합니다.

irssi 라면 `JOIN :` 이후 자동적으로 PASS, NICK, USER 순으로 response를 서버로 보낸다.
nc 라면 직접 각 과정에 대해 입력하여 `ctrl + v + m`을 통해 `\r\n` 으로 메시지를 전송해주도록 하자.

## 4. 채널 접속하기
성공적으로 클라이언트를 연결한 후 [(status))] 라고 되어있는 입력라인에 메시지를 적어 엔터를 쳐봐도 아무런 응답이 없을것이다.

해당 창은 irssi의 메인창으로 window 1번이다.
서버에서 오는 메시지를 보여주는 창이고, 여러가지 상태들에 대한 정보를 준다.

해당 창에서는 `/ + <command> + <params>` 형태로 입력해야 값이 전달된다.
irc를 사용하는 이유는 유저 간 메시지 전송이 목적이기 때문에, 그러기 위해선 이야기방(채널)에 접속해야한다.

이는, `/join <채널명>` 으로 할 수 있다.

추가로 비밀번호가 걸려있는 방에 입장시, `/join <채널명> <비밀번호>` 로 입장.

## 5. 채널 모드 설정
채널에서는 다양한 모드를 지원하고, ft_irc에서는 5가지 모드를 구현해야한다.
- i mode : 초대 모드
- t mode : 토픽 설정을 방장만 가능
- o mode : 방장 권한을 부여/제거
- l mode : 방 인원 수 제한
- k mode : 방 비밀번호

```
/mode <(+ or -)mode option> <mode param1> <mode param2> ...
```

위와 같이 irssi 에 입력하여 모드 세팅이 가능하며, o, l, k 모드의 경우 각각 세팅을 위해선 인자가 들어와야한다.
o는 들어온 인자에 방장권한을 부여 또는 박탈, l은 인원 수 설정(해제할땐 인자 필요 X), k는 비밀번호 설정(해제 시 불필요)이다.

예를들어, 
```
// i, t 모드 설정 -> 인자 불필요
/mode +it

// k 모드 설정 -> 인자 입력
/mode +k 1234

// 여러가지 모드 동시 설정 가능
/mode +itolk usr 4 1234


// 심지어는 이딴것도 ...
// 우리는 (+/-) 여러개 받지 않고 그냥 하나만 처리하는 식으로 구현함,,
/mode +itolk -itolk +itolk usr 4 1234 usr usr1 2 asdf
```

설정된 채널 모드의 확인은
`/mode <#채널명>` 을 입력하여 확인할 수 있다.

## 6. 토픽설정, 초대
```
// 방에 설정된 토픽 확인
/topic

// 토픽 설정
/topic <토픽>

// 토픽 초기화
/topic __(공백)
```
토픽에서 사용되는 용법은 3가지로 토픽 확인, 설정, 초기화.

```
// 채널로 유저를 초대한다.
/invite <채널명> <유저>
```
할없하안.

## 7. DCC 파일 전송
DCC 파일 전송은 기본적으로 privmsg를 이용한다.

```
기본적인 privmsg 문법
PRIVMSG <채널명> :<보낼메시지>

DCC 문법
PRIVMSG <유저명> :.DCC SEND <파일관련>
```
으로 전송이 되기 때문에 기존에 작성했던 PRIVMSG 커맨트 처리 함수에서 DCC를 위한 내용을 추가하였다.

대부분의 내용이 irssi 내장 기능이여서 딱히 추가로 많이 뭘 해줄 필요는 없다.
다만 dcc 방식을 숙지하여야 함.

irssi에서 DCC(Direct Client-to-Client) 파일 전송을 하는 방법

1. 파일 전송 준비
    - 전송할 파일의 경로와 파일명을 알고 있어야 합니다.
2. 파일 전송 명령어 실행
    - irssi 내부 명령어 창에서 다음과 같이 입력합니다:        
        ```javascript
        /dcc send <username> <filepath>
        ```
    - `<username>`은 파일을 전송할 대상 사용자의 닉네임, `<filepath>`는 전송할 파일의 경로와 파일명입니다.
3. 파일 전송 승인
    
    - 대상 사용자는 DCC 파일 전송 요청을 받게 됩니다.
	    ```javascript
		/dcc get <username> (유저 네임 생략가능. 생략 시 그냥 다 받아짐)
		```
    - 대상 사용자가 요청을 승인하면 파일 전송이 시작됩니다.
4. 파일 전송 모니터링
    - irssi 내부에서 파일 전송 진행 상황을 확인할 수 있습니다.
    - `/dcc list` 명령어로 진행 중인 DCC 전송 목록을 확인할 수 있습니다.
    - `/dcc get <username>` 명령어로 특정 사용자의 DCC 전송 상황을 확인할 수 있습니다.
5. 파일 전송 완료
    - 파일 전송이 완료되면 irssi에 메시지가 표시됩니다.
    - 전송이 성공적으로 완료되면 대상 사용자의 파일 수신 폴더에 파일이 저장됩니다.


## 8. 응답코드
서버에는 여러가지 응답코드가 있다.
이는 프로토콜 규약으로 명시되어있으며, 각 상황에 맞게 서버가 클라이언트로 리턴해주는 코드이다.

웹서버에서는 흔히 볼수 있는게 `404 not found`.

이 404 응답코드와 같이 여러가지 에러 상황들에 대해 쉽게 알 수 있도록 응답코드의 형태로 클라이언트로 상황을 인폼해준다.

ft_irc에서는 inspircd의 여러가지 에러 상황들에 대해 응답하는 방식을 참고하였다.
```
// 예) 모드 변경 시 채널 방장이어야 한다는 응답코드 482
void Response::rpl482(int client_fd, User &user, std::string chName)
{
	send_message(client_fd, ":dokang 482 " + user.nick + " " + chName
				+ " :You must be a channel op.\r\n");
}
```


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

