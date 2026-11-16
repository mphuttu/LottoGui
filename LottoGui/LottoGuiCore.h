#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <random>
#include <map>

namespace LottoGuiCore
{
	enum class LotteryResult
	{
		NoPrize,
		Prize,
		Jackpot
	};

	struct ResultCheckContext
	{
		std::wstring game;
		int mainCount = 0;
		bool jokeriDouble = false;
		int kenoLevel = 0;
		int kenoStakeCents = 0;
		bool kenoKunkku = false;
		int ktemStakeCents = 0;
	};

	bool IsValidResultCheckContext(const ResultCheckContext& context);
	std::wstring EncodeResultCheckContext(const ResultCheckContext& context);
	bool DecodeResultCheckContext(const std::wstring& text, ResultCheckContext& context);
	bool IsPredictionCompatibleWithGame(const ResultCheckContext& context,
		const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers);
	bool IsValidActualDraw(const ResultCheckContext& context,
		const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers);

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
		bool excludeHistory = false;
		bool avoidSimilarHistory = false;
		bool historicalSum = false;
		bool historicalDecades = false;
		bool historicalNovelty = false;
		bool antiCrowd = false;
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

	struct HistoricalProfile
	{
		int maximumNumber = 0;
		int picks = 0;
		int drawSize = 0;
		bool positional = false;
		double mean = 0;
		double median = 0;
		double deviation = 0;
		double primeRatio = 0;
		std::vector<double> decades;
		std::map<std::vector<int>, std::size_t> decadePatterns;
		std::vector<std::vector<int>> rows;
		std::vector<std::uint64_t> lowMasks, highMasks;
	};

	bool BuildHistoricalProfile(const std::vector<std::vector<int>>& rows,
		int maximumNumber, int drawSize, int picks, bool positional, HistoricalProfile& profile);
	bool EvaluateHistoricalLine(const HistoricalProfile& profile, const std::vector<int>& line,
		const StructuralOptions& options, double& score);
	StructuralResult GenerateHistoricalLine(const HistoricalProfile& profile,
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
	void FindResultMatches(
		const std::wstring& game,
		const std::vector<int>& predictedMain,
		const std::vector<int>& predictedBonus,
		const std::vector<int>& actualMain,
		const std::vector<int>& actualBonus,
		std::vector<int>& mainMatches,
		std::vector<int>& bonusMatches);
	bool IsValidJokeriDraw(const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers);
	bool IsJokeriPrizeDoubled(
		bool doubleSelected,
		const std::vector<int>& predicted,
		const std::vector<int>& actual);
	std::wstring FormatEurojackpotMatches(
		const std::vector<int>& mainMatches,
		const std::vector<int>& starMatches,
		bool finnish);
	std::wstring FormatFinnishLottoMatches(
		const std::vector<int>& mainMatches,
		const std::vector<int>& bonusMatches,
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
