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
