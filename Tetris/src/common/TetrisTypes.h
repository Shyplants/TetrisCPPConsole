#pragma once
#include <array>
#include <cstdint>

namespace Tetris
{
    // --------------------------------------------------------------------
    //  테트리스 미노 타입 (전역 정의 - 모든 시스템에서 사용 가능)
    //  [0..7] 값 유지: 네트워크 직렬화와 호환됨
    // --------------------------------------------------------------------
    enum class TetrominoType : int32_t
    {
        None = 0,
        I,
        O,
        T,
        L,
        J,
        S,
        Z
    };

    // --------------------------------------------------------------------
    //  회전 정보 (전역 정의)
    // --------------------------------------------------------------------
    enum class Rotation : int32_t
    {
        R0 = 0,
        R90,
        R180,
        R270
    };

    // --------------------------------------------------------------------
    //  움직임 방향 (입력 패킷 용도)
    // --------------------------------------------------------------------
    enum class MoveDir : int32_t
    {
        Left = -1,
        None = 0,
        Right = 1
    };

    // --------------------------------------------------------------------
    //  색상 (TetrominoType → 콘솔 출력 색 변환용)
    //  콘솔 색상 값은 프로젝트의 Color.h에 맞게 변경 가능
    // --------------------------------------------------------------------
    enum ColorCode : int32_t
    {
        COLOR_I = 9,
        COLOR_O = 6,
        COLOR_T = 5,
        COLOR_L = 12,
        COLOR_J = 1,
        COLOR_S = 2,
        COLOR_Z = 4,
        COLOR_EMPTY = 0
    };

    // --------------------------------------------------------------------
    //  미노 개수/회전 개수/미리보기 개수/보드 크기 기본 상수
    // --------------------------------------------------------------------
    constexpr int MINO_COUNT = 4;
    constexpr int ROTATION_COUNT = 4;
    constexpr int MINO_PREVIEW_COUNT = 5;
    constexpr int MINO_TYPE_COUNT = 7;

    // --------------------------------------------------------------------
    //  TetrominoType → 색상 변환
    // --------------------------------------------------------------------
    inline int ColorFromType(TetrominoType type)
    {
        switch (type)
        {
        case TetrominoType::I: return COLOR_I;
        case TetrominoType::O: return COLOR_O;
        case TetrominoType::T: return COLOR_T;
        case TetrominoType::L: return COLOR_L;
        case TetrominoType::J: return COLOR_J;
        case TetrominoType::S: return COLOR_S;
        case TetrominoType::Z: return COLOR_Z;
        default:               return COLOR_EMPTY;
        }
    }

    // --------------------------------------------------------------------
    //  회전 헬퍼 함수
    // --------------------------------------------------------------------
    inline Rotation NextCW(Rotation r)
    {
        switch (r)
        {
        case Rotation::R0:   return Rotation::R90;
        case Rotation::R90:  return Rotation::R180;
        case Rotation::R180: return Rotation::R270;
        case Rotation::R270: return Rotation::R0;
        }
        return Rotation::R0;
    }

    inline Rotation NextCCW(Rotation r)
    {
        switch (r)
        {
        case Rotation::R0:   return Rotation::R270;
        case Rotation::R90:  return Rotation::R0;
        case Rotation::R180: return Rotation::R90;
        case Rotation::R270: return Rotation::R180;
        }
        return Rotation::R0;
    }

} // namespace Tetris