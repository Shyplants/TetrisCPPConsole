# 🎮 Tetris Clone (C++ / Console / Multiplayer)

C++ 기반으로 제작한 클래식 테트리스 콘솔 게임입니다.  
싱글 플레이와 멀티 플레이(1vs1)를 지원하고 있습니다.  

현재 개발 진행 중인 프로젝트이며, 지속적으로 개선되고 있습니다.  

---

## 📌 Features


### 🎯 Single Play
- 7-Bag 랜덤 미노 생성
- Soft Drop / Hard Drop
- 미노 홀드 기능
- 고스트 미노 표시
- 미리보기 5개 미노 표시
- 콤보 & 레벨 시스템
- 게임 오버 후 Title로 복귀

### 🆚 Multiplayer (1 vs 1)
- 커스텀 TCP 네트워크 엔진(based on ASIO)
- 방 참여(Room Join)
- 시드 공유 기반 완전 동기화된 7-Bag 생성
- 상대 미노 실시간 동기화
- 상대 필드 실시간 렌더링
- 승패 판정 및 GameOver Transition

### 🧠 Architecture & Code Structure
- State Machine 기반 화면 전환  
- 메시지 기반 패킷 설계 (`GameMsg`)
- 직렬화 연산자 기반 패킷 처리 (`<<`, `>>`)
- 객체지향 기반 Tetromino, Board, BagRandom 구조
- Timer / Renderer / Selector UI 등 독립 모듈화

---

## 📂 Project Structure

```
Tetris-Project/
├─ NetCommon/ # 클라이언트 & 서버가 공유하는 네트워크 모듈
│ ├─ src/ # 메시지, 연결, tsqueue 등 네트워크 핵심 로직
│ ├─ thirdparty/asio/ # Standalone ASIO (헤더 전용)
│ └─ licenses/ # ASIO 및 외부 라이선스
│
├─ Tetris/ # 게임 클라이언트 (싱글/멀티)
│ ├─ src/
│ │ ├─ states/ # Title, RoomJoin, SinglePlay, MultiPlay, GameOver
│ │ ├─ network/ # TetrisClient, 패킷 처리
│ │ ├─ audio/ # FMOD 기반 사운드
│ │ ├─ inputs/ # 키보드 입력
│ │ ├─ common/ & utils/ # 공통 타입, 로거, 타이머 등
│ ├─ assets/audio/ # 게임 사운드 리소스
│ ├─ thirdparty/
│ │ ├─ fmod/ # FMOD SDK
│ │ ├─ asio/ # Standalone ASIO
│ │ └─ nlohmann/ # JSON 라이브러리
│ └─ licenses/
│
├─TetrisServer/ # 멀티플레이 서버
│ ├─ src/
│ │ ├─ common/
│ │ │ └─ PacketProtocol.h # 클라이언트·서버 공통 패킷 구조 정의
│ │ │
│ │ ├─ TetrisServer.h # 서버 클래스 선언
│ │ ├─ TetrisServer.cpp # 서버 로직 구현
│ │ └─ main.cpp # 서버 실행 엔트리 포인트
│ │
│ ├─ thirdparty/
│ │ ├─ asio/ # standalone Asio (header-only)
│ │ ├─ nlohmann/ # JSON 라이브러리 (header-only)
│ │
│ └─ licenses/
│  ├─ ASIO_LICENSE.txt
│  ├─ NLOHMANN_LICENSE.txt
│
└─ x64/Debug/ # 빌드 아웃풋 (클라이언트/서버 실행 파일 + 리소스)
```

---

## 🎥 Screenshots (추가예정)

---

## 🧾 License
- 이 프로젝트는 MIT License를 따릅니다.
- 포함된 외부 라이브러리 라이선스는 각 프로젝트 ```/licenses``` 폴더에 포함되어 있습니다.
    - ASIO (Boost Software License)
    - nlohmann/json (MIT)
    - OneLoneCoder OLC-3 (참고하여 수정한 코드 포함 시)

---

## 📌 Dev Log  
개발 과정을 개인 블로그에서 기록하고 있습니다.  

🔗 **[Tetris Clone DevLog 링크](https://shy-plants.tistory.com/category/%EA%B0%9C%EB%B0%9C/%ED%85%8C%ED%8A%B8%EB%A6%AC%EC%8A%A4)**  
(프로젝트가 진행되면 추가될 예정)  

- [DevLog #1] 프로젝트 시작
- [DevLog #2] 버그 수정 + 콤보/사운드 시스템 추가
- [DevLog #3] 멀티플레이 기능 추가
- [DevLog #4] 싱글/멀티 플레이 게임오버 로직 및 상태머신 개선
- ...
