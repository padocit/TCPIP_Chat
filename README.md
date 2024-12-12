# TCPIP_Chat

## Chat Program - C++ (Boost lib)
- TCP IP
- Server-Clients
- Async Chatting

> C++ boost library(boost::asio::ip::tcp)

## 구조

boost lib 공식 doc 보다 단순하게 작동하는 구현 예제.

**서버-클라이언트**
- 예시: 열린 서버에 클라이언트 3명이 접속
- 클라이언트는 각각 서버에 메시지를 보내고, 서버는 수신자를 제외한 나머지 클라이언트들에게 메시지를 전달하는 구조
- (아직 서로 다른 기기와의 연결 실험 X)

![tcpip스샷1](https://github.com/user-attachments/assets/6c79a3a0-dc7b-4e58-b8e8-959106383943)


## 보완할 점

1. 멀티스레딩 적용?
2. 클라이언트 입장에서 가독성 문제
  1) Me: (공백)
  2) 클라이언트 간의 식별 불가능
3. 클라이언트 종료 처리 서버에서 미구현: 에러처럼 보임
![tcpip스샷2](https://github.com/user-attachments/assets/ed3550d4-d930-4caa-8a63-a16ef8016c77)

## 참고 자료

* 따배씨++ (홍정모 - 인프런)
* https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/examples.html
* https://theboostcpplibraries.com/boost.asio-io-services-and-io-objects
