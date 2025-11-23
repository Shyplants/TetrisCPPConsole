#pragma once

#include <vector>
#include "./utils/Types.h"
#include "./utils/Colors.h"
#include "./common/TetrisTypes.h"

class Console;
class Tetromino;
struct sBoardState;

class Board
{
public:
	explicit Board(int width = BOARD_WIDTH, int height = BOARD_HEIGHT) noexcept;
	~Board() = default;

	const int GetWidth() const { return m_Width; }
	const int GetHeight() const { return m_Height; }

	const bool OOB(int x, int y) const { return x < 0 || x >= m_Width || y < 0 || y >= m_Height; }

	// 셀 접근 (0이면 빈 칸)
	const int Get(int x, int y) const;
	void Set(int x, int y, int val);
	void Clear();
	const std::vector<int>& GetAll() const { return m_Cells; }

	// 미노를 (dx, dy, rot)만큼 이동/회전 했을 때 충돌하는지 여부 반환
	const bool IsCollide(const Tetromino& t, int dx, int dy, Tetris::Rotation rot) const;
	const bool IsCollide(const Tetromino& t, int dx, int dy) const;
	
	// 미노를 보드에 고정
	void Lock(const Tetromino& t);

	// 가득 찬 라인 제거 및 위에서 당기기. 제거한 라인 수 반환.
	const int ClearFullLines();

	// (x,y) 칸의 색상 반환
	const int GetCellColor(int x, int y) const;

	// 콘솔에 보드 그리기
	void Draw(Console& console, int left, int top);

	// 직렬화용 export 함수
	sBoardState ToPacket() const;

public:
	// 현재 보드 위에 미노를 그릴 때 사용할 블록 좌표(회전 반영)
	static std::array<Vec2, ROTATION_COUNT> GetBlocks(Tetris::TetrominoType type, Tetris::Rotation rot);

private:
	int m_Width{ BOARD_WIDTH }, m_Height{ BOARD_HEIGHT };
	int m_TypeSize{ MINO_TYPE_COUNT };
	std::vector<int> m_Cells; // 0: 비어있는 상태, [1..TetrominoType::Z]: 블럭 타입
};