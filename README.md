# C Socket Multi-Client Chat Server

간단한 다중 클라이언트 채팅 서버/클라이언트 예제입니다.

- C 언어, POSIX socket, pthread 사용
- 여러 명의 사용자가 동시 접속해 채팅 가능
- 각 클라이언트는 별도 닉네임 사용
- 서버는 접속/퇴장/채팅 메시지를 모든 사용자에게 브로드캐스트

---

## 📂 파일 구성

- `server.c` : 멀티스레드 채팅 서버  
- `client.c` : 채팅 클라이언트

---

## 🛠️ 컴파일 방법

리눅스 환경 기준, gcc로 빌드:

```sh
gcc server.c -o server -lpthread
gcc client.c -o client -lpthread
