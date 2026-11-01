#include "LottoGuiCore.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <algorithm>
#include <set>

TEST(StructuralStrategies, CombinesConstraintsOnTheSameRows)
{
	LottoGuiCore::StructuralOptions options;
	options.decades = true;
	options.endings = true;
	options.primes = true;
	options.balanced = true;
	options.avoidRuns = true;
	options.minimumGap = 2;
	std::mt19937 generator(135);
	for (int sample = 0; sample < 20; ++sample)
	{
		std::vector<int> row;
		ASSERT_EQ(LottoGuiCore::StructuralResult::Success,
			LottoGuiCore::GenerateStructuralLine(40, 7, options, generator, row));
		ASSERT_EQ(7u, row.size());
		std::set<int> groups;
		int endings[10] = {};
		int primes = 0;
		int evens = 0;
		int small = 0;
		for (std::size_t i = 0; i < row.size(); ++i)
		{
			EXPECT_GE(row[i], 1);
			EXPECT_LE(row[i], 40);
			if (i > 0) EXPECT_GE(row[i] - row[i - 1], 2);
			groups.insert((row[i] - 1) / 10);
			EXPECT_LE(++endings[row[i] % 10], 2);
			if (std::set<int>{2,3,5,7,11,13,17,19,23,29,31,37}.count(row[i])) ++primes;
			if (row[i] % 2 == 0) ++evens;
			if (row[i] <= 20) ++small;
		}
		EXPECT_EQ(4u, groups.size());
		EXPECT_GE(primes, 1);
		EXPECT_LE(primes, 3);
		EXPECT_GE(evens, 3);
		EXPECT_LE(evens, 4);
		EXPECT_GE(small, 3);
		EXPECT_LE(small, 4);
	}
}

TEST(StructuralStrategies, AdaptsGroupsAndSpacingToGameSize)
{
	std::mt19937 generator(7);
	std::vector<int> row;
	LottoGuiCore::StructuralOptions options;
	options.decades = true;
	ASSERT_EQ(LottoGuiCore::StructuralResult::Success,
		LottoGuiCore::GenerateStructuralLine(70, 2, options, generator, row));
	ASSERT_EQ(2u, row.size());
	EXPECT_NE((row[0] - 1) / 10, (row[1] - 1) / 10);

	options.minimumGap = 2;
	ASSERT_EQ(LottoGuiCore::StructuralResult::Success,
		LottoGuiCore::GenerateStructuralLine(24, 12, options, generator, row));
	ASSERT_EQ(12u, row.size());
	for (std::size_t i = 1; i < row.size(); ++i)
		EXPECT_GE(row[i] - row[i - 1], 2);
	options.minimumGap = 3;
	EXPECT_EQ(LottoGuiCore::StructuralResult::NoSolution,
		LottoGuiCore::GenerateStructuralLine(24, 12, options, generator, row));
	EXPECT_TRUE(row.empty());
}

TEST(StructuralStrategies, RejectsInvalidOptionsAndLimitsConsecutiveRuns)
{
	std::mt19937 generator(15);
	std::vector<int> row{99};
	LottoGuiCore::StructuralOptions options;
	options.minimumGap = 0;
	EXPECT_EQ(LottoGuiCore::StructuralResult::InvalidOptions,
		LottoGuiCore::GenerateStructuralLine(40, 7, options, generator, row));
	EXPECT_TRUE(row.empty());
	options.minimumGap = 1;
	options.avoidRuns = true;
	for (int i = 0; i < 50; ++i)
	{
		ASSERT_EQ(LottoGuiCore::StructuralResult::Success,
			LottoGuiCore::GenerateStructuralLine(24, 12, options, generator, row));
		int run = 1;
		for (std::size_t j = 1; j < row.size(); ++j)
		{
			run = row[j] == row[j - 1] + 1 ? run + 1 : 1;
			EXPECT_LE(run, 3);
		}
	}
}

TEST(FrequencyBacktest, UsesOnlyEarlierDrawsAndSlidesTheWindow)
{
	const std::vector<std::vector<int>> rows{
		{ 1, 2 }, { 3, 4 }, { 1, 2 }, { 3, 4 }
	};
	LottoGuiCore::BacktestResult result;
	ASSERT_TRUE(LottoGuiCore::BacktestFrequency(rows, 4, 2, 2, 1, 3, 42, result));
	EXPECT_EQ(3u, result.draws);
	EXPECT_EQ(0, result.frequencyMatches);
	EXPECT_LE(result.randomMatches, 6);

	LottoGuiCore::BacktestResult repeat;
	ASSERT_TRUE(LottoGuiCore::BacktestFrequency(rows, 4, 2, 2, 1, 3, 42, repeat));
	EXPECT_EQ(result.frequencyMatches, repeat.frequencyMatches);
	EXPECT_EQ(result.randomMatches, repeat.randomMatches);
}

TEST(FrequencyBacktest, UsesOnlyTheRequestedTailAndAllowsExtraColumns)
{
	const std::vector<std::vector<int>> rows{
		{ 99 }, { 1, 2, 8 }, { 1, 2, 9 }, { 1, 2, 10 }
	};
	LottoGuiCore::BacktestResult result;
	ASSERT_TRUE(LottoGuiCore::BacktestFrequency(rows, 4, 2, 2, 1, 2, 5, result));
	EXPECT_EQ(2u, result.draws);
	EXPECT_EQ(4, result.frequencyMatches);
}

TEST(FrequencyBacktest, RejectsInvalidDrawsAndInsufficientHistory)
{
	LottoGuiCore::BacktestResult result;
	EXPECT_FALSE(LottoGuiCore::BacktestFrequency({ { 1, 2 } }, 4, 2, 2, 1, 2, 5, result));
	EXPECT_FALSE(LottoGuiCore::BacktestFrequency({ { 1, 1 }, { 2, 3 } }, 4, 2, 2, 1, 1, 5, result));
	EXPECT_EQ(1u, result.invalidRow);
	EXPECT_FALSE(LottoGuiCore::BacktestFrequency({ { 1, 2 }, { 2, 5 } }, 4, 2, 2, 1, 1, 5, result));
	EXPECT_EQ(2u, result.invalidRow);
	EXPECT_FALSE(LottoGuiCore::BacktestFrequency({ { 1, 2 }, { 3 } }, 4, 2, 2, 1, 1, 5, result));
	EXPECT_EQ(2u, result.invalidRow);
}

TEST(ActualResultsParsing, AcceptsSupportedSeparatorsAndBonusNumbers)
{
	std::vector<int> mainNumbers;
	std::vector<int> bonusNumbers;

	ASSERT_TRUE(LottoGuiCore::ParseActualResults(L"11, 17;20\t22 35 | 4;8", mainNumbers, bonusNumbers));

	const std::vector<int> expectedMain{ 11, 17, 20, 22, 35 };
	const std::vector<int> expectedBonus{ 4, 8 };
	EXPECT_EQ(expectedMain, mainNumbers);
	EXPECT_EQ(expectedBonus, bonusNumbers);
}

TEST(ActualResultsParsing, AllowsResultsWithoutBonusNumbers)
{
	std::vector<int> mainNumbers;
	std::vector<int> bonusNumbers{ 99 };

	ASSERT_TRUE(LottoGuiCore::ParseActualResults(L"3 5,8", mainNumbers, bonusNumbers));

	const std::vector<int> expectedMain{ 3, 5, 8 };
	EXPECT_EQ(expectedMain, mainNumbers);
	EXPECT_TRUE(bonusNumbers.empty());
}

TEST(ActualResultsParsing, RejectsMalformedListsAndClearsOutputs)
{
	const std::vector<std::wstring> invalidInputs{
		L"",
		L"1 2 nope",
		L"1 2 |",
		L"| 3",
		L"1 | 2 | 3"
	};

	for (size_t index = 0; index < invalidInputs.size(); ++index)
	{
		std::vector<int> mainNumbers{ 99 };
		std::vector<int> bonusNumbers{ 98 };
		EXPECT_FALSE(LottoGuiCore::ParseActualResults(invalidInputs[index], mainNumbers, bonusNumbers))
			<< "input index " << index;
		EXPECT_TRUE(mainNumbers.empty());
		EXPECT_TRUE(bonusNumbers.empty());
	}
}

TEST(PredictionParsing, SupportsAllApplicationRowPrefixesAndNotes)
{
	struct Example
	{
		std::wstring line;
		std::vector<int> mainNumbers;
		std::vector<int> bonusNumbers;
	};

	const std::vector<Example> examples{
		{ L"Prediction 1: 1 2 3 (Balanced) | Star numbers: 4 5", { 1, 2, 3 }, { 4, 5 } },
		{ L"Ennuste 1: 2 4 6 | Tähtinumerot: 1 3", { 2, 4, 6 }, { 1, 3 } },
		{ L"Ennuste 2: 2, 11, 34 | Viking-numero: 5", { 2, 11, 34 }, { 5 } },
		{ L"Profile prediction 1: 6;7;8", { 6, 7, 8 }, {} },
		{ L"Profiiliennuste 1: 8 8 1 (eri numeroita: 2)", { 8, 8, 1 }, {} },
		{ L"Combination 1: 9,10 11", { 9, 10, 11 }, {} },
		{ L"Yhdistelmä 1: 10 11 12", { 10, 11, 12 }, {} },
		{ L"Line 1: 12 13 14", { 12, 13, 14 }, {} },
		{ L"Rivi 1: 15 16 17", { 15, 16, 17 }, {} }
	};

	for (size_t index = 0; index < examples.size(); ++index)
	{
		std::vector<int> mainNumbers;
		std::vector<int> bonusNumbers;
		ASSERT_TRUE(LottoGuiCore::ParsePredictionRow(examples[index].line, mainNumbers, bonusNumbers))
			<< "input index " << index;
		EXPECT_EQ(examples[index].mainNumbers, mainNumbers);
		EXPECT_EQ(examples[index].bonusNumbers, bonusNumbers);
	}
}

TEST(PredictionParsing, RejectsSummariesAndMalformedMainNumbers)
{
	const std::vector<std::wstring> invalidRows{
		L"Active strategies: Balanced, Delta",
		L"Prediction without separator",
		L"Prediction 1: 3 nope 5",
		L"Ennuste 1: 3 5 | Virheellinen: bonus"
	};

	for (size_t index = 0; index < invalidRows.size(); ++index)
	{
		std::vector<int> mainNumbers{ 99 };
		std::vector<int> bonusNumbers{ 98 };
		EXPECT_FALSE(LottoGuiCore::ParsePredictionRow(invalidRows[index], mainNumbers, bonusNumbers))
			<< "input index " << index;
		EXPECT_TRUE(mainNumbers.empty());
		EXPECT_TRUE(bonusNumbers.empty());
	}
}

TEST(ResultMatching, CountsRepeatedDigitsOnlyUpToDrawMultiplicity)
{
	const std::vector<int> predicted{ 0, 2, 2, 2, 9 };
	const std::vector<int> actual{ 2, 2, 3, 9 };
	std::vector<int> matches;

	LottoGuiCore::FindMatches(predicted, actual, matches);

	const std::vector<int> expected{ 2, 2, 9 };
	EXPECT_EQ(expected, matches);
}

TEST(ResultMatching, JokeriMatchesOnlyDigitsInTheSamePosition)
{
	const std::vector<int> predicted{ 1, 2, 2, 4, 5, 6, 7 };
	const std::vector<int> actual{ 2, 2, 1, 4, 6, 5, 7 };
	std::vector<int> matches;
	LottoGuiCore::FindPositionalMatches(predicted, actual, matches);
	EXPECT_EQ((std::vector<int>{ 2, 4, 7 }), matches);
	EXPECT_EQ(600, LottoGuiCore::GetJokeriPrizeCents(static_cast<int>(matches.size()), false));
}

TEST(ResultMatching, JokeriDoubleRequiresSelectionAndCorrectLastDigit)
{
	const std::vector<int> predicted{ 1, 2, 3, 4, 5, 6, 7 };
	EXPECT_TRUE(LottoGuiCore::IsJokeriPrizeDoubled(
		true, predicted, { 9, 2, 3, 4, 5, 6, 7 }));
	EXPECT_FALSE(LottoGuiCore::IsJokeriPrizeDoubled(
		false, predicted, { 9, 2, 3, 4, 5, 6, 7 }));
	EXPECT_FALSE(LottoGuiCore::IsJokeriPrizeDoubled(
		true, predicted, { 1, 2, 3, 4, 5, 6, 9 }));
	EXPECT_FALSE(LottoGuiCore::IsJokeriPrizeDoubled(true, predicted, { 7 }));
}

TEST(ResultMatching, JokeriDrawRequiresSevenDigitsAndNoBonus)
{
	EXPECT_TRUE(LottoGuiCore::IsValidJokeriDraw({ 0, 0, 1, 2, 3, 4, 9 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidJokeriDraw({ 0, 1 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidJokeriDraw({ 0, 1, 2, 3, 4, 5, 10 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidJokeriDraw({ 0, 1, 2, 3, 4, 5, 6 }, { 7 }));
}

TEST(ResultMatching, EurojackpotSeparatesMainAndStarMatches)
{
	std::vector<int> mainMatches;
	std::vector<int> starMatches;
	LottoGuiCore::FindMatches({ 40, 42, 46, 47, 48 }, { 21, 26, 37, 39, 42 }, mainMatches);
	LottoGuiCore::FindMatches({ 5, 9 }, { 5, 7 }, starMatches);
	EXPECT_EQ(L"1 + 1 oikein (osumat: 42 (päänumerot); 5 (tähtinumerot))",
		LottoGuiCore::FormatEurojackpotMatches(mainMatches, starMatches, true));
	EXPECT_EQ(L"1 + 1 correct (matches: 42 (main numbers); 5 (star numbers))",
		LottoGuiCore::FormatEurojackpotMatches(mainMatches, starMatches, false));
}

TEST(ResultMatching, EurojackpotFormatsEmptyAndOverlappingNumberGroups)
{
	EXPECT_EQ(L"0 + 0 oikein (osumat: ei osumia (päänumerot); ei osumia (tähtinumerot))",
		LottoGuiCore::FormatEurojackpotMatches({}, {}, true));
	EXPECT_EQ(L"2 + 1 correct (matches: 5, 42 (main numbers); 5 (star numbers))",
		LottoGuiCore::FormatEurojackpotMatches({ 5, 42 }, { 5 }, false));
	EXPECT_EQ(L"0 + 1 oikein (osumat: ei osumia (päänumerot); 7 (tähtinumerot))",
		LottoGuiCore::FormatEurojackpotMatches({}, { 7 }, true));
}

TEST(LotteryResults, SuomenLottoRecognizesBonusTiersAndSevenIsJackpot)
{
	using LottoGuiCore::LotteryResult;

	EXPECT_EQ(LotteryResult::NoPrize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 3, 0, 7));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 3, 1, 7));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 4, 0, 7));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 6, 0, 7));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 6, 1, 7));
	EXPECT_EQ(LotteryResult::Jackpot,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 7, 0, 7));
}

TEST(ResultMatching, SuomenLottoMatchesDrawnBonusAgainstPredictionMainNumbers)
{
	const std::vector<int> prediction{ 1, 2, 3, 8, 9, 10, 11 };
	const std::vector<int> draw{ 1, 2, 3, 4, 5, 6, 7 };
	std::vector<int> mainMatches;
	std::vector<int> bonusMatches;
	LottoGuiCore::FindResultMatches(L"Suomen Lotto", prediction, {}, draw, { 8 },
		mainMatches, bonusMatches);
	EXPECT_EQ((std::vector<int>{ 1, 2, 3 }), mainMatches);
	EXPECT_EQ((std::vector<int>{ 8 }), bonusMatches);
	EXPECT_EQ(L"3 + 1 oikein (osumat: 1, 2, 3 (päänumerot); 8 (lisänumero))",
		LottoGuiCore::FormatFinnishLottoMatches(mainMatches, bonusMatches, true));
	EXPECT_EQ(LottoGuiCore::LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto",
			static_cast<int>(mainMatches.size()), static_cast<int>(bonusMatches.size()), 7));
	LottoGuiCore::FindResultMatches(L"Suomen Lotto", prediction, {}, draw, { 12 },
		mainMatches, bonusMatches);
	EXPECT_TRUE(bonusMatches.empty());
	EXPECT_EQ(LottoGuiCore::LotteryResult::NoPrize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto",
			static_cast<int>(mainMatches.size()), static_cast<int>(bonusMatches.size()), 7));
}

TEST(LotteryResults, VikingLottoRequiresThreeMatchesAndVikingNumberForJackpot)
{
	using LottoGuiCore::LotteryResult;

	EXPECT_EQ(LotteryResult::NoPrize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Viking Lotto", 2, 0, 6));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Viking Lotto", 3, 0, 6));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Viking Lotto", 6, 0, 6));
	EXPECT_EQ(LotteryResult::Jackpot,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Viking Lotto", 6, 1, 6));
}

TEST(LotteryResults, EurojackpotOnlyRecognizesPublishedWinningCombinations)
{
	using LottoGuiCore::LotteryResult;

	const int winningCombinations[][2] =
	{
		{ 5, 2 }, { 5, 1 }, { 5, 0 }, { 4, 2 }, { 4, 1 }, { 4, 0 },
		{ 3, 2 }, { 3, 1 }, { 3, 0 }, { 2, 2 }, { 2, 1 }, { 1, 2 }
	};
	for (size_t index = 0; index < sizeof(winningCombinations) / sizeof(winningCombinations[0]); ++index)
	{
		const LotteryResult expected = (winningCombinations[index][0] == 5 &&
			winningCombinations[index][1] == 2)
			? LotteryResult::Jackpot
			: LotteryResult::Prize;
		EXPECT_EQ(expected, LottoGuiCore::ClassifyEurojackpotResult(
			winningCombinations[index][0], winningCombinations[index][1]));
	}

	const int losingCombinations[][2] =
	{
		{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 2, 0 }, { 3, 3 }, { 4, 3 },
		{ 5, 3 }, { 6, 2 }, { -1, 2 }, { 5, -1 }
	};
	for (size_t index = 0; index < sizeof(losingCombinations) / sizeof(losingCombinations[0]); ++index)
	{
		EXPECT_EQ(LotteryResult::NoPrize, LottoGuiCore::ClassifyEurojackpotResult(
			losingCombinations[index][0], losingCombinations[index][1]));
	}
}

TEST(LotteryResults, JokeriUsesFixedAndOptionalDoubledPrizeAmounts)
{
	const int expectedRegularPrizes[]{ 0, 0, 300, 600, 5000, 50000, 2000000, 100000000 };
	const int expectedDoubledPrizes[]{ 0, 0, 600, 1200, 10000, 100000, 4000000, 200000000 };

	for (int correctMatches = 0; correctMatches <= 7; ++correctMatches)
	{
		EXPECT_EQ(expectedRegularPrizes[correctMatches],
			LottoGuiCore::GetJokeriPrizeCents(correctMatches, false))
			<< correctMatches << " matches";
		EXPECT_EQ(expectedDoubledPrizes[correctMatches],
			LottoGuiCore::GetJokeriPrizeCents(correctMatches, true))
			<< correctMatches << " doubled matches";
	}
	EXPECT_EQ(0, LottoGuiCore::GetJokeriPrizeCents(8, true));
}

TEST(LotteryResults, MilliUsesFixedPrizesForAvailableMainNumberMatches)
{
	const int expectedPrizes[]{ 0, 0, 100, 200, 1000, 50000, 100000000 };

	for (int correctMatches = 0; correctMatches <= 6; ++correctMatches)
	{
		EXPECT_EQ(expectedPrizes[correctMatches],
			LottoGuiCore::GetMilliPrizeCents(correctMatches))
			<< correctMatches << " matches";
	}
	EXPECT_EQ(0, LottoGuiCore::GetMilliPrizeCents(7));
}

TEST(LotteryResults, KenoUsesLevelAndSelectedStakeForStandardPrizes)
{
	const int stakeCents = 50;
	EXPECT_EQ(10000000, LottoGuiCore::GetKenoPrizeCents(10, 10, stakeCents, false));
	EXPECT_EQ(250000, LottoGuiCore::GetKenoPrizeCents(10, 9, stakeCents, false));
	EXPECT_EQ(10000, LottoGuiCore::GetKenoPrizeCents(10, 8, stakeCents, false));
	EXPECT_EQ(stakeCents, LottoGuiCore::GetKenoPrizeCents(10, 0, stakeCents, false));
	EXPECT_EQ(350, LottoGuiCore::GetKenoPrizeCents(2, 2, stakeCents, false));
	EXPECT_EQ(0, LottoGuiCore::GetKenoPrizeCents(9, 0, stakeCents, false));
}

TEST(LotteryResults, KenoUsesKunkkuTableWithoutDoublingSelectedStake)
{
	const int stakeCents = 50;
	EXPECT_EQ(25000000, LottoGuiCore::GetKenoPrizeCents(10, 10, stakeCents, true));
	EXPECT_EQ(500000, LottoGuiCore::GetKenoPrizeCents(10, 9, stakeCents, true));
	EXPECT_EQ(100, LottoGuiCore::GetKenoPrizeCents(10, 2, stakeCents, true));
	EXPECT_EQ(2500, LottoGuiCore::GetKenoPrizeCents(2, 2, stakeCents, true));
	EXPECT_EQ(500, LottoGuiCore::GetKenoPrizeCents(2, 1, stakeCents, true));
	EXPECT_EQ(0, LottoGuiCore::GetKenoPrizeCents(10, 1, stakeCents, true));
	EXPECT_EQ(0, LottoGuiCore::GetKenoPrizeCents(11, 10, stakeCents, true));
	EXPECT_EQ(0, LottoGuiCore::GetKenoPrizeCents(10, 10, 0, true));
}

TEST(LotteryResults, KenoPayoutsMatchAllPublishedTierMultipliers)
{
	const int regularMultipliers[9][11] =
	{
		{ 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 1, 18, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 1, 2, 32, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 9, 200, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 1, 3, 20, 420, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 1, 10, 100, 2400, 0, 0, 0 },
		{ 0, 0, 0, 0, 1, 3, 20, 240, 10000, 0, 0 },
		{ 0, 0, 0, 0, 0, 2, 10, 50, 1200, 50000, 0 },
		{ 1, 0, 0, 0, 0, 1, 4, 20, 200, 5000, 200000 }
	};
	const int kunkkuMultipliers[9][11] =
	{
		{ 0, 10, 50, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 6, 10, 100, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 6, 6, 14, 200, 0, 0, 0, 0, 0, 0 },
		{ 0, 6, 4, 6, 34, 600, 0, 0, 0, 0, 0 },
		{ 0, 6, 2, 4, 10, 90, 2000, 0, 0, 0, 0 },
		{ 0, 6, 2, 2, 6, 30, 300, 6000, 0, 0, 0 },
		{ 0, 6, 2, 2, 4, 6, 60, 800, 30000, 0, 0 },
		{ 0, 4, 2, 2, 2, 6, 20, 100, 3000, 150000, 0 },
		{ 0, 0, 2, 2, 2, 4, 10, 50, 500, 10000, 500000 }
	};
	const int stakeCents = 50;

	for (int levelIndex = 0; levelIndex < 9; ++levelIndex)
	{
		for (int matches = 0; matches <= 10; ++matches)
		{
			EXPECT_EQ(regularMultipliers[levelIndex][matches] * stakeCents,
				LottoGuiCore::GetKenoPrizeCents(levelIndex + 2, matches, stakeCents, false))
				<< "regular, level " << levelIndex + 2 << ", matches " << matches;
			EXPECT_EQ(kunkkuMultipliers[levelIndex][matches] * stakeCents,
				LottoGuiCore::GetKenoPrizeCents(levelIndex + 2, matches, stakeCents, true))
				<< "Kunkku, level " << levelIndex + 2 << ", matches " << matches;
		}
	}
}

TEST(LotteryResults, KtemUsesSymmetricFixedMultipliersWithoutOnnenapila)
{
	const int multipliers[13] =
	{
		125000, 250, 25, 5, 1, 0, 0, 0, 1, 5, 25, 250, 125000
	};
	const int stakeCents = 100;

	for (int correctMatches = 0; correctMatches <= 12; ++correctMatches)
	{
		EXPECT_EQ(multipliers[correctMatches] * stakeCents,
			LottoGuiCore::GetKtemPrizeCents(correctMatches, stakeCents))
			<< correctMatches << " matches";
	}
	EXPECT_EQ(0, LottoGuiCore::GetKtemPrizeCents(-1, stakeCents));
	EXPECT_EQ(0, LottoGuiCore::GetKtemPrizeCents(13, stakeCents));
	EXPECT_EQ(0, LottoGuiCore::GetKtemPrizeCents(12, 0));
}

TEST(ResultCheckContext, RoundTripsEveryGameAndItsPrizeSettings)
{
	const std::vector<std::pair<std::wstring, int>> games{
		{ L"Custom", 9 }, { L"Suomen Lotto", 7 }, { L"Milli", 6 },
		{ L"KTEM", 12 }, { L"Keno", 10 }, { L"Eurojackpot", 5 },
		{ L"Viking Lotto", 6 }, { L"Jokeri", 7 }
	};
	for (const auto& game : games)
	{
		LottoGuiCore::ResultCheckContext original;
		original.game = game.first;
		original.mainCount = game.second;
		original.jokeriDouble = true;
		original.kenoLevel = 10;
		original.kenoStakeCents = 250;
		original.kenoKunkku = true;
		original.ktemStakeCents = 150;
		const std::wstring encoded = LottoGuiCore::EncodeResultCheckContext(original);
		ASSERT_FALSE(encoded.empty()) << game.first;

		LottoGuiCore::ResultCheckContext restored;
		ASSERT_TRUE(LottoGuiCore::DecodeResultCheckContext(encoded, restored)) << game.first;
		EXPECT_EQ(original.game, restored.game);
		EXPECT_EQ(original.mainCount, restored.mainCount);
		EXPECT_EQ(original.jokeriDouble, restored.jokeriDouble);
		EXPECT_EQ(original.kenoLevel, restored.kenoLevel);
		EXPECT_EQ(original.kenoStakeCents, restored.kenoStakeCents);
		EXPECT_EQ(original.kenoKunkku, restored.kenoKunkku);
		EXPECT_EQ(original.ktemStakeCents, restored.ktemStakeCents);
	}
}

TEST(ResultCheckContext, RejectsCorruptOrIncompleteContextWithoutRetainingOldGame)
{
	const std::vector<std::wstring> invalid{
		L"", L"Milli\n6 0 0 0 0", L"KTEM\n12 0 0 0 0 0",
		L"Keno\n10 0 10 0 0 0", L"Jokeri\n7 2 0 0 0 0",
		L"Milli\n6 0 0 0 0 0 extra", L"Unknown\n6 0 0 0 0 0"
	};
	for (const auto& input : invalid)
	{
		LottoGuiCore::ResultCheckContext context;
		context.game = L"Milli";
		context.mainCount = 6;
		EXPECT_FALSE(LottoGuiCore::DecodeResultCheckContext(input, context));
		EXPECT_TRUE(context.game.empty());
	}
}

TEST(ResultCheckContext, RestoredKtemSixMatchesDoNotUseMilliJackpot)
{
	LottoGuiCore::ResultCheckContext context;
	ASSERT_TRUE(LottoGuiCore::DecodeResultCheckContext(L"KTEM\n12 0 0 0 0 100", context));
	const std::vector<int> prediction{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
	const std::vector<int> draw{ 1, 2, 3, 4, 5, 6, 13, 14, 15, 16, 17, 18 };
	ASSERT_TRUE(LottoGuiCore::IsPredictionCompatibleWithGame(context, prediction, {}));
	ASSERT_TRUE(LottoGuiCore::IsValidActualDraw(context, draw, {}));
	std::vector<int> matches;
	LottoGuiCore::FindMatches(prediction, draw, matches);
	ASSERT_EQ(6u, matches.size());
	EXPECT_EQ(0, LottoGuiCore::GetKtemPrizeCents(static_cast<int>(matches.size()), context.ktemStakeCents));
	EXPECT_EQ(100000000, LottoGuiCore::GetMilliPrizeCents(static_cast<int>(matches.size())));
	EXPECT_EQ(12500000, LottoGuiCore::GetKtemPrizeCents(0, context.ktemStakeCents));
	EXPECT_EQ(12500000, LottoGuiCore::GetKtemPrizeCents(12, context.ktemStakeCents));
}

TEST(ResultCheckContext, ChecksGameSpecificPredictionAndDrawShapes)
{
	LottoGuiCore::ResultCheckContext context;
	context.game = L"KTEM";
	context.mainCount = 12;
	context.ktemStakeCents = 100;
	EXPECT_FALSE(LottoGuiCore::IsPredictionCompatibleWithGame(context, { 1, 2, 3, 4, 5, 6 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context,
		{ 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 25 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }, { 1 }));

	context.game = L"Keno";
	context.mainCount = 2;
	context.kenoLevel = 2;
	context.kenoStakeCents = 50;
	EXPECT_TRUE(LottoGuiCore::IsPredictionCompatibleWithGame(context, { 1, 70 }, {}));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 70 }, {}));
	EXPECT_TRUE(LottoGuiCore::IsValidActualDraw(context,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 }, {}));

	context.game = L"Eurojackpot";
	context.mainCount = 5;
	EXPECT_TRUE(LottoGuiCore::IsPredictionCompatibleWithGame(context, { 1, 2, 3, 4, 5 }, { 1, 2 }));
	EXPECT_FALSE(LottoGuiCore::IsPredictionCompatibleWithGame(context, { 1, 2, 3, 4, 5 }, { 1 }));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5 }, {}));
	EXPECT_TRUE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5 }, { 1, 2 }));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5 }, { 2, 2 }));

	context.game = L"Viking Lotto";
	context.mainCount = 6;
	EXPECT_TRUE(LottoGuiCore::IsPredictionCompatibleWithGame(context, { 1, 2, 3, 4, 5, 6 }, { 1 }));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5, 6 }, {}));
	EXPECT_TRUE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5, 6 }, { 1 }));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5, 6 }, { 6 }));

	context.game = L"Suomen Lotto";
	context.mainCount = 7;
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5, 6, 7 }, {}));
	EXPECT_TRUE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5, 6, 7 }, { 8 }));
	EXPECT_FALSE(LottoGuiCore::IsValidActualDraw(context, { 1, 2, 3, 4, 5, 6, 7 }, { 7 }));

	context.game = L"Jokeri";
	context.mainCount = 7;
	EXPECT_TRUE(LottoGuiCore::IsPredictionCompatibleWithGame(context, { 1, 1, 1, 2, 2, 3, 3 }, {}));
	EXPECT_TRUE(LottoGuiCore::IsValidActualDraw(context, { 0, 0, 1, 2, 3, 4, 9 }, {}));
}
