#include "LottoGuiCore.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

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

TEST(LotteryResults, SuomenLottoRequiresFourMatchesAndSevenIsJackpot)
{
	using LottoGuiCore::LotteryResult;

	EXPECT_EQ(LotteryResult::NoPrize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 3, 0, 7));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 4, 0, 7));
	EXPECT_EQ(LotteryResult::Prize,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 6, 0, 7));
	EXPECT_EQ(LotteryResult::Jackpot,
		LottoGuiCore::ClassifyFinnishOrVikingLottoResult(L"Suomen Lotto", 7, 0, 7));
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
