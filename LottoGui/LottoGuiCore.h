#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <random>

namespace LottoGuiCore
{
	enum class LotteryResult
	{
		NoPrize,
		Prize,
		Jackpot
	};

	struct BacktestResult
	{
		std::size_t draws = 0;
		long long frequencyMatches = 0;
		long long randomMatches = 0;
		std::size_t invalidRow = 0;
	};

	// Rows are oldest first. Only the first drawSize columns are main numbers.
	bool BacktestFrequency(
		const std::vector<std::vector<int>>& rows, int maximumNumber,
		int drawSize, int picks, std::size_t window, std::size_t testDraws,
		std::uint32_t seed, BacktestResult& result);

	struct StructuralOptions
	{
		bool decades = false;
		bool endings = false;
		bool primes = false;
		bool balanced = false;
		bool avoidRuns = false;
		int minimumGap = 1;
	};

	enum class StructuralResult
	{
		Success,
		InvalidOptions,
		NoSolution,
		SearchLimit
	};

	StructuralResult GenerateStructuralLine(int maximumNumber, int picks,
		const StructuralOptions& options, std::mt19937& generator, std::vector<int>& line);

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
