#pragma once

#include <string>
#include <vector>

namespace LottoGuiCore
{
	enum class LotteryResult
	{
		NoPrize,
		Prize,
		Jackpot
	};

	bool ParseNumberList(const std::wstring& input, std::vector<int>& values);
	bool ParseActualResults(
		const std::wstring& input,
		std::vector<int>& mainNumbers,
		std::vector<int>& bonusNumbers);
	bool ParsePredictionRow(
		const std::wstring& line,
		std::vector<int>& mainNumbers,
		std::vector<int>& bonusNumbers);
	void FindMatches(
		const std::vector<int>& predicted,
		const std::vector<int>& actual,
		std::vector<int>& matches);
	void FindPositionalMatches(
		const std::vector<int>& predicted,
		const std::vector<int>& actual,
		std::vector<int>& matches);
	bool IsValidJokeriDraw(const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers);
	bool IsJokeriPrizeDoubled(
		bool doubleSelected,
		const std::vector<int>& predicted,
		const std::vector<int>& actual);
	std::wstring FormatEurojackpotMatches(
		const std::vector<int>& mainMatches,
		const std::vector<int>& starMatches,
		bool finnish);
	LotteryResult ClassifyFinnishOrVikingLottoResult(
		const std::wstring& gameType,
		int mainMatches,
		int bonusMatches,
		int mainCount);
	LotteryResult ClassifyEurojackpotResult(int mainMatches, int starMatches);
	int GetJokeriPrizeCents(int correctMatches, bool doubled);
	int GetMilliPrizeCents(int correctMatches);
	int GetKenoPrizeCents(int level, int correctMatches, int stakeCents, bool kunkkuSelected);
	int GetKtemPrizeCents(int correctMatches, int stakeCents);
}
