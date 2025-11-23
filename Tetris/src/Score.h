#pragma once

class Score
{
public:
	Score();
	~Score() = default;

	void Reset();

	// 한 턴에서 라인 클리어 결과 전달
	void OnLinesCleared(int cleared, bool isTSpin = false);

	// 소프트/하드 드롭 계산
	void AddSoftDrop(int cells);
	void AddHardDrop(int cells);

	const int GetScore() const { return m_Score; }
	const int GetLines() const { return m_Lines; }
	const int GetLevel() const { return m_Level; }
	const int GetCombo() const { return m_Combo; }
	const int IsBackToBack() const { return m_bBackToBack; }

private:
	void AddLineScore(int cleared, bool isTSpin);
	void AddComboBonus(int cleared);
	void HandleBackToBack(bool isTSpin, bool isTetris);
	void UpdateLevel();


private:
	int m_Score{ 0 };
	int m_Lines{ 0 };
	int m_Level{ 1 };

	int m_Combo{ 0 };
	bool m_bBackToBack{ false };
};