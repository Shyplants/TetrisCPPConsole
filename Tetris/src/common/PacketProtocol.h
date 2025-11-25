#pragma once

#include <sp_net.h>

enum class GameMsg : uint32_t
{
    // ------------------------------
    // Connection Handshake (공통)
    // ------------------------------
    Client_Accepted,
    Client_AssignID,
    Client_RegisterWithServer,
    Client_UnregisterWithServer,

    Client_Ping,
    Server_Ping,
    Client_Pong,
    Server_Pong,

    // ------------------------------
    // Lobby / Room Join Flow
    // ------------------------------
    Client_RequestRoomJoin,     // 클라이언트 → 서버: 룸 참가 요청
    Server_RoomJoinAccepted,    // 서버 → 클라: 룸 참가 승인
    Server_RoomJoinDenied,      // 서버 → 클라: 룸 참가 불가(인원 초과 등)
    Server_RoomPlayerList,      // 서버 → 클라: 현재 방 참여자 목록 갱신
    Server_RoomPlayerJoined,    // 서버 → 클라: 새로운 플레이어 입장
    Server_RoomPlayerLeft,      // 서버 → 클라: 플레이어 퇴장

    // ------------------------------
    // Ready / Start Game 
    // ------------------------------
    Client_Ready,               // 클라 → 서버: 준비 완료 (옵션)
    Client_CancelReady,         // 클라 → 서버: 준비 취소 (옵션)
    Server_AllPlayersReady,     // 서버 → 클라: 모든 플레이어 준비됨 (MultiPlayState로 전이)

    // ------------------------------
    // Game
    // ------------------------------
    Game_AddPlayer,
    Game_RemovePlayer,
    Game_UpdatePlayer,

    Game_Start,                 // 서버 → 클라: 게임 시작 신호
    Game_SendBagSeed,

    Game_BoardState,
    Game_CurMinoState,
    Game_HoldMinoState,
    Game_PreviewMinoState,
    Game_PlayerDead,

    Server_GameOver,
};

struct sPlayerDescription
{
    uint32_t nUniqueID = 0;
    uint32_t nAvatarID = 0;

    // TODO : 플레이어 필요한 필드 채우기
};

struct sMinoState
{
    int32_t type = 0;  // Tetrominotype [1..7]
    int32_t x = 0;
    int32_t y = 0;
    int32_t rot = 0;   // rotation enum [0..3]
};


constexpr uint16_t NET_PREVIEW_MINO_COUNT = 5;
struct sPreviewMinoState
{
    std::array< int32_t, NET_PREVIEW_MINO_COUNT> previewTypes{};
};

constexpr uint16_t NET_BOARD_WIDTH = 10;
constexpr uint16_t NET_BOARD_HEIGHT = 3 + 20;
constexpr uint16_t NET_BOARD_CELLS = NET_BOARD_WIDTH * NET_BOARD_HEIGHT;

struct sBoardState
{
    std::array<int, NET_BOARD_CELLS> cells{};
};

struct sGameOverInfo
{
    uint32_t nWinnerID = 0;
    uint32_t nLoserID = 0;
};