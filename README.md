# ft_irc (Team Project)
Internet Relay Chat server.
Team project (2 people)

IRC 서버에 대한 이해, 네트워크, I/O 멀티플렉싱 등 이해하고자 진행한 프로젝트입니다.

---

## 프로젝트 소개
- 소켓을 활용하여 상용 클라이언트들의 동작을 지원하는 서버를 만들자.
- TCP/IP를 통해 클라이언트와 서버의 소통 진행.
- "-std=c++98" 플래그를 사용하여 컴파일.
- 클라이언트는 상용 클라이언트인 `irssi` 사용.

### 팀 구성
`dokoh` : 소켓 연결 구현

`sihkang` : 프로토콜에 맞는 서버 - 클라이언트 간 메시지 응답 처리
			커맨드 구현 (invite, kick, mode, privmsg, quit, topic 등)

---
## 구현 사항
- `dokoh` : 클라이언트의 연결을 진행하고, 통신이 이루어지도록 세팅.
- `sihkang` 
  - 클라이언트로부터 받은 raw data 자체를 프로토콜에 의거하여 파싱 처리
  - 해당 데이터 요청에 대응
  1. 클라이언트 접속(join)에 따른 서버 연결 처리.
  2. 클라이언트로부터 받은 privmsg를 다른 클라이언트로 포워딩.
  3. 채널의 operator / regular user 구현
  4. 채널 명령어 (KICK, INVITE, TOPIC, MODE) 구현
  - File transfer 기능 활용

RFC2812: https://datatracker.ietf.org/doc/html/rfc2812
해당 RFC 문서를 참조하여 구현.

---
## 사용법
### server

1. 서버 동작
```c
// repo clone 후,
make
./Ircserv <PORT> <PASSWORD>
```

### 테스트 방식
도커를 활용하여 독립된 환경에서 여러 클라이언트가 접근해보도록 하였음.
![Alt text](./images/docker.png)

각 클라이언트는 `irssi` 상용 irc 클라이언트 활용.

```
irssi -c host.docker.internal -p <Port> -n <Nickname> --password <PASSWORD>
```
8080 포트에 닉네임으로 알맞은 서버 비밀번호를 입력하여 서버에 접속.
![Alt text](./images/irssi.png)
각각 클라이언트가 asdf, qwer 닉네임으로 접속한 모습.
이후 `/join ch1` 을 통해 채널에 접속하였다.
![Alt text](./images/ch1.png)

---

## 리팩토링

1. write/read -> send/recv 함수로 교체
소켓 프로그래밍에 좀 더 알맞은 입출력 함수로 변경해보았음.
플래그설정을 통해 메시지 처리에 대한 옵션이 많지만, fcntl을 통해 미리 설정한 후 진행하였으므로 별도의 플래그를 사용하지는 않았음.



---
## simple flowChart
![Alt text](./images/flowchart.png)