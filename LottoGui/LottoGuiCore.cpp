#include "LottoGuiCore.h"

#include <map>
#include <sstream>

namespace
{
	bool StartsWith(const std::wstring& text, const std::wstring& prefix)
	{
		return text.compare(0, prefix.length(), prefix) == 0;
	}
}

namespace LottoGuiCore
{
	bool ParseNumberList(const std::wstring& input, std::vector<int>& values)
	{
		values.clear();
		std::wstring normalized(input);
		for (size_t index = 0; index < normalized.length(); ++index)
		{
			if (normalized[index] == L',' || normalized[index] == L';' || normalized[index] == L'\t')
			{
				normalized[index] = L' ';
			}
		}

		std::wstringstream stream(normalized);
		std::wstring token;
		while (stream >> token)
		{
			std::wstringstream tokenStream(token);
			int value = 0;
			wchar_t trailingCharacter = 0;
			if (!(tokenStream >> value) || (tokenStream >> trailingCharacter))
			{
				values.clear();
				return false;
			}
			values.push_back(value);
		}

		return !values.empty();
	}

	bool ParseActualResults(
		const std::wstring& input,
		std::vector<int>& mainNumbers,
		std::vector<int>& bonusNumbers)
	{
		mainNumbers.clear();
		bonusNumbers.clear();

		const size_t separatorPosition = input.find(L'|');
		if (separatorPosition == std::wstring::npos)
		{
			return ParseNumberList(input, mainNumbers);
		}

		if (input.find(L'|', separatorPosition + 1) != std::wstring::npos ||
			!ParseNumberList(input.substr(0, separatorPosition), mainNumbers) ||
			!ParseNumberList(input.substr(separatorPosition + 1), bonusNumbers))
		{
			mainNumbers.clear();
			bonusNumbers.clear();
			return false;
		}

		return true;
	}

	bool ParsePredictionRow(
		const std::wstring& line,
		std::vector<int>& mainNumbers,
		std::vector<int>& bonusNumbers)
	{
		mainNumbers.clear();
		bonusNumbers.clear();

		if (!StartsWith(line, L"Prediction ") &&
			!StartsWith(line, L"Ennuste ") &&
			!StartsWith(line, L"Profile prediction ") &&
			!StartsWith(line, L"Profiiliennuste ") &&
			!StartsWith(line, L"Combination ") &&
			!StartsWith(line, L"Yhdistelmä ") &&
			!StartsWith(line, L"Line ") &&
			!StartsWith(line, L"Rivi "))
		{
			return false;
		}

		const size_t colonPosition = line.find(L':');
		if (colonPosition == std::wstring::npos)
		{
			return false;
		}

		const std::wstring values = line.substr(colonPosition + 1);
		const size_t bonusSeparator = values.find(L'|');
		std::wstring mainText = values.substr(0, bonusSeparator);
		const size_t mainNote = mainText.find(L'(');
		if (mainNote != std::wstring::npos)
		{
			mainText = mainText.substr(0, mainNote);
		}
		if (!ParseNumberList(mainText, mainNumbers))
		{
			mainNumbers.clear();
			return false;
		}

		if (bonusSeparator != std::wstring::npos)
		{
			std::wstring bonusText = values.substr(bonusSeparator + 1);
			const size_t bonusLabel = bonusText.find(L':');
			if (bonusLabel != std::wstring::npos)
			{
				bonusText = bonusText.substr(bonusLabel + 1);
			}
			const size_t notePosition = bonusText.find(L'(');
			if (notePosition != std::wstring::npos)
			{
				bonusText = bonusText.substr(0, notePosition);
			}
			if (!ParseNumberList(bonusText, bonusNumbers))
			{
				mainNumbers.clear();
				bonusNumbers.clear();
				return false;
			}
		}

		return true;
	}

	void FindMatches(
		const std::vector<int>& predicted,
		const std::vector<int>& actual,
		std::vector<int>& matches)
	{
		std::map<int, int> remaining;
		for (size_t index = 0; index < actual.size(); ++index)
		{
			++remaining[actual[index]];
		}

		for (size_t index = 0; index < predicted.size(); ++index)
		{
			std::map<int, int>::iterator found = remaining.find(predicted[index]);
			if (found != remaining.end() && found->second > 0)
			{
				matches.push_back(predicted[index]);
				--found->second;
			}
		}
	}

	void FindPositionalMatches(
		const std::vector<int>& predicted,
		const std::vector<int>& actual,
		std::vector<int>& matches)
	{
		for (size_t index = 0; index < predicted.size() && index < actual.size(); ++index)
		{
			if (predicted[index] == actual[index])
			{
				matches.push_back(predicted[index]);
			}
		}
	}

	bool IsJokeriPrizeDoubled(
		bool doubleSelected,
		const std::vector<int>& predicted,
		const std::vector<int>& actual)
	{
		return doubleSelected && predicted.size() == 7 && actual.size() == 7 &&
			predicted[6] == actual[6];
	}

	bool IsValidJokeriDraw(const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers)
	{
		if (mainNumbers.size() != 7 || !bonusNumbers.empty())
		{
			return false;
		}
		for (size_t index = 0; index < mainNumbers.size(); ++index)
		{
			if (mainNumbers[index] < 0 || mainNumbers[index] > 9)
			{
				return false;
			}
		}
		return true;
	}

	std::wstring FormatEurojackpotMatches(
		const std::vector<int>& mainMatches,
		const std::vector<int>& starMatches,
		bool finnish)
	{
		std::wostringstream output;
		output << mainMatches.size() << L" + " << starMatches.size()
			<< (finnish ? L" oikein (osumat: " : L" correct (matches: ");
		for (size_t index = 0; index < mainMatches.size(); ++index)
		{
			if (index > 0) output << L", ";
			output << mainMatches[index];
		}
		if (mainMatches.empty()) output << (finnish ? L"ei osumia" : L"none");
		output << (finnish ? L" (päänumerot); " : L" (main numbers); ");
		for (size_t index = 0; index < starMatches.size(); ++index)
		{
			if (index > 0) output << L", ";
			output << starMatches[index];
		}
		if (starMatches.empty()) output << (finnish ? L"ei osumia" : L"none");
		output << (finnish ? L" (tähtinumerot))" : L" (star numbers))");
		return output.str();
	}

	LotteryResult ClassifyFinnishOrVikingLottoResult(
		const std::wstring& gameType,
		int mainMatches,
		int bonusMatches,
		int mainCount)
	{
		if (StartsWith(gameType, L"Suomen Lotto"))
		{
			if (mainMatches == mainCount)
			{
				return LotteryResult::Jackpot;
			}
			return mainMatches >= 4 ? LotteryResult::Prize : LotteryResult::NoPrize;
		}

		if (StartsWith(gameType, L"Viking Lotto"))
		{
			if (mainMatches == mainCount && bonusMatches > 0)
			{
				return LotteryResult::Jackpot;
			}
			return mainMatches >= 3 ? LotteryResult::Prize : LotteryResult::NoPrize;
		}

		return LotteryResult::NoPrize;
	}

	LotteryResult ClassifyEurojackpotResult(int mainMatches, int starMatches)
	{
		if (mainMatches < 0 || mainMatches > 5 || starMatches < 0 || starMatches > 2)
		{
			return LotteryResult::NoPrize;
		}

		if (mainMatches == 5 && starMatches == 2)
		{
			return LotteryResult::Jackpot;
		}

		if ((mainMatches == 5 && starMatches <= 1) ||
			(mainMatches == 4 && starMatches <= 2) ||
			(mainMatches == 3 && starMatches <= 2) ||
			(mainMatches == 2 && starMatches >= 1 && starMatches <= 2) ||
			(mainMatches == 1 && starMatches == 2))
		{
			return LotteryResult::Prize;
		}

		return LotteryResult::NoPrize;
	}

	int GetJokeriPrizeCents(int correctMatches, bool doubled)
	{
		int prizeCents = 0;
		switch (correctMatches)
		{
		case 2: prizeCents = 300; break;
		case 3: prizeCents = 600; break;
		case 4: prizeCents = 5000; break;
		case 5: prizeCents = 50000; break;
		case 6: prizeCents = 2000000; break;
		case 7: prizeCents = 100000000; break;
		default: return 0;
		}

		return doubled ? prizeCents * 2 : prizeCents;
	}

	int GetMilliPrizeCents(int correctMatches)
	{
		switch (correctMatches)
		{
		case 2: return 100;
		case 3: return 200;
		case 4: return 1000;
		case 5: return 50000;
		case 6: return 100000000;
		default: return 0;
		}
	}

	int GetKenoPrizeCents(int level, int correctMatches, int stakeCents, bool kunkkuSelected)
	{
		static const int regularMultipliers[9][11] =
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
		static const int kunkkuMultipliers[9][11] =
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

		if (level < 2 || level > 10 || correctMatches < 0 || correctMatches > 10 || stakeCents <= 0)
		{
			return 0;
		}

		const int multiplier = kunkkuSelected
			? kunkkuMultipliers[level - 2][correctMatches]
			: regularMultipliers[level - 2][correctMatches];
		return multiplier * stakeCents;
	}

	int GetKtemPrizeCents(int correctMatches, int stakeCents)
	{
		static const int multipliers[13] =
		{
			125000, 250, 25, 5, 1, 0, 0, 0, 1, 5, 25, 250, 125000
		};

		if (correctMatches < 0 || correctMatches >= 13 || stakeCents <= 0)
		{
			return 0;
		}

		return multipliers[correctMatches] * stakeCents;
	}
}
