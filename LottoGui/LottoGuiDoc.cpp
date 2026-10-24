
// LottoGuiDoc.cpp : implementation of the CLottoGuiDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LottoGui.h"
#endif

#include "LottoGuiDoc.h"
#include "LottoGuiCore.h"
#include "LottoDialog.h"
#include "CheckResultsDlg.h"
#include "SuomenLottoDlg.h"
#include "MilliDlg.h"
#include "KtemDlg.h"
#include "KenoDlg.h"
#include "EurojackpotDlg.h"
#include "VikingLottoDlg.h"
#include "JokeriDlg.h"
#include "LotteryPrices.h"

#include <propkey.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	bool IsFinnishLanguage()
	{
		return PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH;
	}

	CString LocalizeText(LPCTSTR englishText, LPCTSTR finnishText)
	{
		return IsFinnishLanguage() ? finnishText : englishText;
	}

	wstring GetPredictionPrefix()
	{
		return IsFinnishLanguage() ? L"Ennuste " : L"Prediction ";
	}

	struct ColumnStats
	{
		CString label;
		int minValue;
		int maxValue;
		int modeValue;
		double mean;
		double median;
		double harmonicMean;
		double geometricMean;
		double rootMeanSquare;
		double standardDeviation;
		double lowerQuartile;
		double upperQuartile;
	};

	CString FormatDouble(double value)
	{
		if (!std::isfinite(value))
		{
			return IsFinnishLanguage() ? L"Ei tietoa" : L"N/A";
		}

		CString text;
		text.Format(L"%.2f", value);

		while (text.GetLength() > 0 && text.Right(1) == L"0")
		{
			text = text.Left(text.GetLength() - 1);
		}

		while (text.GetLength() > 0 && (text.Right(1) == L"." || text.Right(1) == L","))
		{
			text = text.Left(text.GetLength() - 1);
		}

		if (IsFinnishLanguage())
		{
			text.Replace(L'.', L',');
		}

		return text;
	}

	bool ParsePredictionRow(const CString& line, vector<int>& mainNumbers, vector<int>& bonusNumbers);

	wstring ToWideString(const CString& text)
	{
		return wstring(text.GetString(), static_cast<size_t>(text.GetLength()));
	}

	int ReadPricePreference(LPCTSTR name, int defaultValue, int minimumValue, int maximumValue)
	{
		const int value = AfxGetApp()->GetProfileInt(_T("Ticket prices"), name, defaultValue);
		return (value < minimumValue || value > maximumValue) ? defaultValue : value;
	}

	void WritePricePreference(LPCTSTR name, int value)
	{
		AfxGetApp()->WriteProfileInt(_T("Ticket prices"), name, value);
	}

	void AppendTicketCost(CString& summary, const CStringArray& output, int centsPerRow)
	{
		int ticketCount = 0;
		for (INT_PTR index = 0; index < output.GetSize(); ++index)
		{
			vector<int> mainNumbers;
			vector<int> bonusNumbers;
			if (ParsePredictionRow(output.GetAt(index), mainNumbers, bonusNumbers))
			{
				++ticketCount;
			}
		}

		const CString rowPrice = LotteryPrices::FormatCents(centsPerRow);
		const CString totalPrice = LotteryPrices::FormatCents(ticketCount * centsPerRow);
		CString costText;
		if (PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
		{
			costText.Format(L" | Rivihinta: %s | Rivien yhteishinta (%d riviä): %s",
				rowPrice.GetString(), ticketCount, totalPrice.GetString());
		}
		else
		{
			costText.Format(L" | Price per row: %s | Total ticket cost (%d rows): %s",
				rowPrice.GetString(), ticketCount, totalPrice.GetString());
		}
		summary += costText;
	}

	vector<int> ParseNumbersFromLine(CString line)
	{
		vector<int> values;
		line.Trim();
		line.Replace(L';', L' ');
		line.Replace(L',', L' ');
		line.Replace(L'\t', L' ');

		wstringstream stream(wstring(line.GetString()));
		int value = 0;
		while (stream >> value)
		{
			values.push_back(value);
		}

		return values;
	}

	bool ParseActualResults(const CString& input, vector<int>& mainNumbers, vector<int>& bonusNumbers)
	{
		return LottoGuiCore::ParseActualResults(ToWideString(input), mainNumbers, bonusNumbers);
	}

	bool ParsePredictionRow(const CString& line, vector<int>& mainNumbers, vector<int>& bonusNumbers)
	{
		return LottoGuiCore::ParsePredictionRow(ToWideString(line), mainNumbers, bonusNumbers);
	}

	void FindMatches(const vector<int>& predicted, const vector<int>& actual, vector<int>& matches)
	{
		LottoGuiCore::FindMatches(predicted, actual, matches);
	}

	bool HasPredictionRows(const CStringArray& lines)
	{
		for (INT_PTR index = 0; index < lines.GetSize(); ++index)
		{
			vector<int> mainNumbers;
			vector<int> bonusNumbers;
			if (ParsePredictionRow(lines.GetAt(index), mainNumbers, bonusNumbers))
			{
				return true;
			}
		}
		return false;
	}

	// Include fixed payouts for games with known prize tables and summarize other game tiers.
	CString FormatPrizeTier(int mainMatches, int bonusMatches)
	{
		CString text;
		text.Format(IsFinnishLanguage() ? L"%d+%d oikein" : L"%d+%d correct",
			mainMatches, bonusMatches);
		return text;
	}

	CString FormatCorrectCount(int count)
	{
		CString text;
		text.Format(IsFinnishLanguage() ? L"%d oikein!" : L"%d correct!", count);
		return text;
	}

	CString FormatPrizeAmount(long long prizeCents)
	{
		CString amount;
		amount.Format(L"%I64d,%02d €", prizeCents / 100,
			static_cast<int>(prizeCents % 100));
		const int decimalSeparator = amount.Find(L',');
		for (int position = decimalSeparator - 3; position > 0; position -= 3)
		{
			amount.Insert(position, L' ');
		}
		return amount;
	}

	CString FormatPrizeWithAmount(const CString& label, int prizeCents)
	{
		CString text;
		const CString amount = FormatPrizeAmount(prizeCents);
		text.Format(L"%s (%s)", label.GetString(), amount.GetString());
		return text;
	}

	CString DescribePrize(
		const CString& gameType,
		int correctMatches,
		int bonusMatches,
		int mainCount,
		BOOL jokeriDoubled,
		int kenoPrizeCents,
		int ktemPrizeCents)
	{
		if (gameType.IsEmpty())
		{
			return L"";
		}

		if (gameType.Find(L"Eurojackpot") >= 0)
		{
			const LottoGuiCore::LotteryResult result =
				LottoGuiCore::ClassifyEurojackpotResult(correctMatches, bonusMatches);
			if (result == LottoGuiCore::LotteryResult::Jackpot)
			{
				return LocalizeText(L"Jackpot!", L"Päävoitto!");
			}
			if (result == LottoGuiCore::LotteryResult::NoPrize)
			{
				return LocalizeText(L"No win.", L"Ei voittoa.");
			}

			if (correctMatches == 5 && bonusMatches == 1)
			{
				return FormatPrizeTier(5, 1);
			}
			if (correctMatches == 5)
			{
				return FormatCorrectCount(5);
			}
			if (correctMatches == 4 && bonusMatches == 2)
			{
				return FormatPrizeTier(4, 2);
			}
			if (correctMatches == 4 && bonusMatches == 1)
			{
				return FormatPrizeTier(4, 1);
			}
			if (correctMatches == 4)
			{
				return FormatCorrectCount(4);
			}
			if (correctMatches == 3 && bonusMatches == 2)
			{
				return FormatPrizeTier(3, 2);
			}
			if (correctMatches == 3 && bonusMatches == 1)
			{
				return FormatPrizeTier(3, 1);
			}
			if (correctMatches == 3)
			{
				return FormatCorrectCount(3);
			}
			if (correctMatches == 2 && bonusMatches == 2)
			{
				return FormatPrizeTier(2, 2);
			}
			if (correctMatches == 2 && bonusMatches == 1)
			{
				return FormatPrizeTier(2, 1);
			}
			if (correctMatches == 1 && bonusMatches == 2)
			{
				return FormatPrizeTier(1, 2);
			}
		}

		if (gameType.Find(L"Suomen Lotto") >= 0 || gameType.Find(L"Viking Lotto") >= 0)
		{
			const LottoGuiCore::LotteryResult result =
				LottoGuiCore::ClassifyFinnishOrVikingLottoResult(
					ToWideString(gameType), correctMatches, bonusMatches, mainCount);
			if (result == LottoGuiCore::LotteryResult::Jackpot)
			{
				return LocalizeText(L"Jackpot!", L"Päävoitto!");
			}
			if (result == LottoGuiCore::LotteryResult::Prize)
			{
				return FormatCorrectCount(correctMatches);
			}
			return LocalizeText(L"No win.", L"Ei voittoa.");
		}

		if (gameType.Find(L"Jokeri") >= 0)
		{
			const int prizeCents = LottoGuiCore::GetJokeriPrizeCents(
				correctMatches, jokeriDoubled != FALSE);
			if (prizeCents == 0)
			{
				return LocalizeText(L"No win.", L"Ei voittoa.");
			}

			if (correctMatches == 7)
			{
				return FormatPrizeWithAmount(
					LocalizeText(L"Jackpot!", L"Päävoitto!"), prizeCents);
			}
			return FormatPrizeWithAmount(FormatCorrectCount(correctMatches), prizeCents);
		}

		if (gameType.Find(L"Milli") >= 0)
		{
			const int prizeCents = LottoGuiCore::GetMilliPrizeCents(correctMatches);
			if (prizeCents == 0)
			{
				return LocalizeText(L"No win.", L"Ei voittoa.");
			}

			if (correctMatches == 6)
			{
				return FormatPrizeWithAmount(
					LocalizeText(L"Jackpot!", L"Päävoitto!"), prizeCents);
			}
			return FormatPrizeWithAmount(FormatCorrectCount(correctMatches), prizeCents);
		}

		if (gameType.Find(L"Keno") >= 0)
		{
			if (kenoPrizeCents > 0)
			{
				return FormatPrizeWithAmount(FormatCorrectCount(correctMatches), kenoPrizeCents);
			}
			return LocalizeText(L"No win.", L"Ei voittoa.");
		}

		if (gameType.Find(L"KTEM") >= 0)
		{
			if (ktemPrizeCents > 0)
			{
				const CString prizeLabel = (correctMatches == 0 || correctMatches == 12)
					? LocalizeText(L"Jackpot!", L"Päävoitto!")
					: FormatCorrectCount(correctMatches);
				return FormatPrizeWithAmount(prizeLabel, ktemPrizeCents);
			}
			return LocalizeText(L"No win.", L"Ei voittoa.");
		}

		return L"";
	}

	bool LoadCsvRows(const CString& csvFileName, vector<vector<int> >& rows, CString& usedPath)
	{
		rows.clear();
		usedPath.Empty();

		TCHAR modulePath[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, modulePath, MAX_PATH);
		CString exeFolder(modulePath);
		const int slashPos = exeFolder.ReverseFind(L'\\');
		if (slashPos >= 0)
		{
			exeFolder = exeFolder.Left(slashPos);
		}

		CStringArray candidates;
		candidates.Add(exeFolder + L"\\data\\" + csvFileName);
		candidates.Add(exeFolder + L"\\..\\data\\" + csvFileName);
		candidates.Add(exeFolder + L"\\..\\..\\data\\" + csvFileName);
		candidates.Add(L"data\\" + csvFileName);

		for (INT_PTR index = 0; index < candidates.GetSize(); ++index)
		{
			CStdioFile file;
			if (!file.Open(candidates.GetAt(index), CFile::modeRead | CFile::typeText))
			{
				continue;
			}

			CString line;
			while (file.ReadString(line))
			{
				vector<int> row = ParseNumbersFromLine(line);
				if (!row.empty())
				{
					rows.push_back(row);
				}
			}
			file.Close();

			if (!rows.empty())
			{
				usedPath = candidates.GetAt(index);
				return true;
			}
		}

		return false;
	}

	bool LoadSuomenLottoRows(vector<vector<int> >& rows, CString& usedPath)
	{
		return LoadCsvRows(L"SuomenLottoData.csv", rows, usedPath);
	}

	bool LoadMilliRows(vector<vector<int> >& rows, CString& usedPath)
	{
		vector<vector<int> > rawRows;
		if (!LoadCsvRows(L"MilliData.csv", rawRows, usedPath))
			return false;

		rows.clear();
		for (size_t rowIndex = 0; rowIndex < rawRows.size(); ++rowIndex)
		{
			vector<int> cleanedRow;
			for (size_t valueIndex = 0; valueIndex < rawRows[rowIndex].size(); ++valueIndex)
			{
				const int value = rawRows[rowIndex][valueIndex];
				if (value >= 1 && value <= 40)
					cleanedRow.push_back(value);
			}
			if (!cleanedRow.empty())
				rows.push_back(cleanedRow);
		}
		return !rows.empty();
	}

	bool LoadKtemRows(vector<vector<int> >& rows, CString& usedPath)
	{
		return LoadCsvRows(L"KTEM.csv", rows, usedPath);
	}

	bool LoadKenoRows(vector<vector<int> >& rows, CString& usedPath)
	{
		vector<vector<int> > rawRows;
		if (!LoadCsvRows(L"KenoData.csv", rawRows, usedPath))
		{
			return false;
		}

		rows.clear();
		for (size_t rowIndex = 0; rowIndex < rawRows.size(); ++rowIndex)
		{
			vector<int> cleanedRow;
			for (size_t valueIndex = 0; valueIndex < rawRows[rowIndex].size(); ++valueIndex)
			{
				const int value = rawRows[rowIndex][valueIndex];
				if (value >= 1 && value <= 70)
				{
					cleanedRow.push_back(value);
				}
			}

			if (!cleanedRow.empty())
			{
				rows.push_back(cleanedRow);
			}
		}

		return !rows.empty();
	}

	bool LoadEurojackpotRows(vector<vector<int> >& rows, CString& usedPath)
	{
		return LoadCsvRows(L"EurojackpotData.csv", rows, usedPath);
	}

	bool LoadVikingRows(vector<vector<int> >& rows, CString& usedPath)
	{
		return LoadCsvRows(L"VikingData.csv", rows, usedPath);
	}

	bool LoadJokeriRows(vector<vector<int> >& rows, CString& usedPath)
	{
		return LoadCsvRows(L"JokeriData.csv", rows, usedPath);
	}

	int DetectDominantColumnCount(const vector<vector<int> >& rows)
	{
		map<int, int> frequencies;
		for (size_t index = 0; index < rows.size(); ++index)
		{
			++frequencies[static_cast<int>(rows[index].size())];
		}

		int dominantCount = 0;
		int bestFrequency = -1;
		for (map<int, int>::const_iterator it = frequencies.begin(); it != frequencies.end(); ++it)
		{
			if (it->second > bestFrequency || (it->second == bestFrequency && it->first > dominantCount))
			{
				dominantCount = it->first;
				bestFrequency = it->second;
			}
		}

		return dominantCount;
	}

	int DetectMainNumberCount(int dominantColumnCount)
	{
		if (dominantColumnCount >= 7)
		{
			return 7;
		}

		return dominantColumnCount;
	}

	double CalculateMedian(const vector<int>& orderedValues)
	{
		if (orderedValues.empty())
		{
			return 0.0;
		}

		const size_t count = orderedValues.size();
		if ((count % 2U) == 1U)
		{
			return static_cast<double>(orderedValues[count / 2U]);
		}

		return (static_cast<double>(orderedValues[(count / 2U) - 1U]) + static_cast<double>(orderedValues[count / 2U])) / 2.0;
	}

	CString GetColumnLabel(int columnIndex, int mainCount, int dominantColumnCount)
	{
		if (columnIndex < mainCount)
		{
			CString label;
			label.Format(IsFinnishLanguage() ? L"Nro%d" : L"Nr%d", columnIndex + 1);
			return label;
		}

		const int extraIndex = columnIndex - mainCount;
		const int extraCount = dominantColumnCount - mainCount;

		if (extraCount <= 1)
		{
			return IsFinnishLanguage() ? L"Lisä" : L"Extra";
		}

		if (extraIndex == extraCount - 1)
		{
			return L"Plus";
		}

		if (extraIndex == 0)
		{
			return IsFinnishLanguage() ? L"Lisä" : L"Extra";
		}

		CString label;
		label.Format(IsFinnishLanguage() ? L"Lisä %d" : L"Extra %d", extraIndex + 1);
		return label;
	}

	CString ToStableCString(const wstring& text)
	{
		if (text.empty())
		{
			return CString();
		}

		CString stableText(text.c_str());
		return stableText;
	}

	void AddCStringArrayText(CStringArray& values, const wstring& text)
	{
		const wstring stableText = text;
		values.Add(ToStableCString(stableText));
	}

	CString BuildTableHeaderRow(int columnCount)
	{
		wostringstream output;
		output << left << setw(16) << L"";
		for (int index = 0; index < columnCount; ++index)
		{
			CString label;
			label.Format(IsFinnishLanguage() ? L"Nro%d" : L"Nr%d", index + 1);
			output << right << setw(10) << static_cast<LPCWSTR>(label);
		}
		const wstring text = output.str();
		return ToStableCString(text);
	}

	CString BuildTableHeaderRow(const vector<CString>& headers)
	{
		wostringstream output;
		output << left << setw(16) << L"";
		for (size_t index = 0; index < headers.size(); ++index)
		{
			output << right << setw(10) << static_cast<LPCWSTR>(headers[index]);
		}
		const wstring text = output.str();
		return ToStableCString(text);
	}

	CString BuildTableRow(const CString& metricName, const vector<CString>& values)
	{
		CString localizedMetricName = metricName;
		if (IsFinnishLanguage())
		{
			if (metricName == L"Mean") localizedMetricName = L"Keskiarvo";
			else if (metricName == L"Min") localizedMetricName = L"Minimi";
			else if (metricName == L"Max") localizedMetricName = L"Maksimi";
			else if (metricName == L"Median") localizedMetricName = L"Mediaani";
			else if (metricName == L"Mode") localizedMetricName = L"Moodi";
			else if (metricName == L"Harmonic") localizedMetricName = L"Harmoninen";
			else if (metricName == L"Geometric") localizedMetricName = L"Geometrinen";
			else if (metricName == L"StdDev") localizedMetricName = L"Keskihajonta";
			else if (metricName == L"LowerQ") localizedMetricName = L"Alempi kvartiili";
			else if (metricName == L"UpperQ") localizedMetricName = L"Ylempi kvartiili";
		}

		wostringstream output;
		output << left << setw(16) << static_cast<LPCWSTR>(localizedMetricName);
		for (size_t index = 0; index < values.size(); ++index)
		{
			output << right << setw(10) << static_cast<LPCWSTR>(values[index]);
		}
		const wstring text = output.str();
		return ToStableCString(text);
	}

	ColumnStats ComputeColumnStats(const vector<int>& values, const CString& label)
	{
		ColumnStats stats = {};
		stats.label = label;

		if (values.empty())
		{
			return stats;
		}

		vector<int> ordered(values);
		sort(ordered.begin(), ordered.end());

		stats.minValue = ordered.front();
		stats.maxValue = ordered.back();
		stats.median = CalculateMedian(ordered);

		vector<int> lowerHalf(ordered.begin(), ordered.begin() + (ordered.size() / 2U));
		vector<int> upperHalf(ordered.begin() + ((ordered.size() + 1U) / 2U), ordered.end());
		stats.lowerQuartile = lowerHalf.empty() ? stats.median : CalculateMedian(lowerHalf);
		stats.upperQuartile = upperHalf.empty() ? stats.median : CalculateMedian(upperHalf);

		map<int, int> counts;
		double reciprocalSum = 0.0;
		double logSum = 0.0;
		double squareSum = 0.0;
		double total = 0.0;
		bool hasZero = false;

		for (size_t index = 0; index < ordered.size(); ++index)
		{
			const double value = static_cast<double>(ordered[index]);
			total += value;
			if (value == 0.0)
			{
				hasZero = true;
			}
			else
			{
				reciprocalSum += (1.0 / value);
				logSum += log(value);
			}
			squareSum += value * value;
			++counts[ordered[index]];
		}

		stats.mean = total / static_cast<double>(ordered.size());
		stats.harmonicMean = hasZero ? numeric_limits<double>::quiet_NaN() : (static_cast<double>(ordered.size()) / reciprocalSum);
		stats.geometricMean = hasZero ? 0.0 : exp(logSum / static_cast<double>(ordered.size()));
		stats.rootMeanSquare = sqrt(squareSum / static_cast<double>(ordered.size()));

		int bestModeCount = -1;
		stats.modeValue = ordered.front();
		for (map<int, int>::const_iterator it = counts.begin(); it != counts.end(); ++it)
		{
			if (it->second > bestModeCount)
			{
				bestModeCount = it->second;
				stats.modeValue = it->first;
			}
		}

		double varianceSum = 0.0;
		for (size_t index = 0; index < ordered.size(); ++index)
		{
			const double diff = static_cast<double>(ordered[index]) - stats.mean;
			varianceSum += diff * diff;
		}
		stats.standardDeviation = sqrt(varianceSum / static_cast<double>(ordered.size()));

		return stats;
	}

	int ChooseWeightedNumber(const map<int, int>& frequencies, mt19937& generator)
	{
		if (frequencies.empty())
		{
			return 0;
		}

		int totalWeight = 0;
		for (map<int, int>::const_iterator it = frequencies.begin(); it != frequencies.end(); ++it)
		{
			totalWeight += (it->second > 0) ? it->second : 1;
		}

		uniform_int_distribution<int> distribution(1, totalWeight);
		int target = distribution(generator);
		for (map<int, int>::const_iterator it = frequencies.begin(); it != frequencies.end(); ++it)
		{
			target -= (it->second > 0) ? it->second : 1;
			if (target <= 0)
			{
				return it->first;
			}
		}

		return frequencies.begin()->first;
	}

	mt19937 CreateRandomGenerator()
	{
		random_device seedSource;
		return mt19937(seedSource());
	}

	vector<int> BuildPredictionLine(const map<int, int>& mainFrequencies, int mainCount, mt19937& generator)
	{
		vector<int> result;
		map<int, int> pool(mainFrequencies);

		while (!pool.empty() && static_cast<int>(result.size()) < mainCount)
		{
			const int selected = ChooseWeightedNumber(pool, generator);
			result.push_back(selected);
			pool.erase(selected);
		}

		sort(result.begin(), result.end());
		return result;
	}

	int CountSetDigits(int digitMask)
	{
		int count = 0;
		while (digitMask != 0)
		{
			count += digitMask & 1;
			digitMask >>= 1;
		}
		return count;
	}

	vector<int> BuildJokeriRepeatProfilePrediction(
		const vector<map<int, int> >& digitFrequencies,
		const map<int, int>& repeatProfileFrequencies,
		mt19937& generator)
	{
		const int digitCount = static_cast<int>(digitFrequencies.size());
		const int targetUniqueCount = ChooseWeightedNumber(repeatProfileFrequencies, generator);
		if (digitCount <= 0 || targetUniqueCount <= 0 || targetUniqueCount > digitCount)
		{
			return vector<int>();
		}

		vector<vector<double> > positionWeights(digitCount, vector<double>(10, 0.0));
		for (int position = 0; position < digitCount; ++position)
		{
			double totalWeight = 0.0;
			for (map<int, int>::const_iterator it = digitFrequencies[position].begin();
				it != digitFrequencies[position].end(); ++it)
			{
				if (it->first >= 0 && it->first <= 9 && it->second > 0)
				{
					totalWeight += it->second;
				}
			}

			if (totalWeight == 0.0)
			{
				positionWeights[position][0] = 1.0;
				continue;
			}

			for (map<int, int>::const_iterator it = digitFrequencies[position].begin();
				it != digitFrequencies[position].end(); ++it)
			{
				if (it->first >= 0 && it->first <= 9 && it->second > 0)
				{
					positionWeights[position][it->first] =
						static_cast<double>(it->second) / totalWeight;
				}
			}
		}

		const int maskCount = 1 << 10;
		vector<vector<double> > completionProbabilities(
			digitCount + 1, vector<double>(maskCount, 0.0));
		for (int mask = 0; mask < maskCount; ++mask)
		{
			if (CountSetDigits(mask) == targetUniqueCount)
			{
				completionProbabilities[digitCount][mask] = 1.0;
			}
		}

		for (int position = digitCount - 1; position >= 0; --position)
		{
			for (int mask = 0; mask < maskCount; ++mask)
			{
				if (CountSetDigits(mask) > targetUniqueCount)
				{
					continue;
				}

				double probability = 0.0;
				for (int digit = 0; digit <= 9; ++digit)
				{
					const int nextMask = mask | (1 << digit);
					probability += positionWeights[position][digit] *
						completionProbabilities[position + 1][nextMask];
				}
				completionProbabilities[position][mask] = probability;
			}
		}

		vector<int> prediction;
		int digitMask = 0;
		for (int position = 0; position < digitCount; ++position)
		{
			double totalProbability = 0.0;
			for (int digit = 0; digit <= 9; ++digit)
			{
				const int nextMask = digitMask | (1 << digit);
				totalProbability += positionWeights[position][digit] *
					completionProbabilities[position + 1][nextMask];
			}

			if (totalProbability <= 0.0)
			{
				return vector<int>();
			}

			uniform_real_distribution<double> distribution(0.0, totalProbability);
			const double targetProbability = distribution(generator);
			double cumulativeProbability = 0.0;
			int selectedDigit = -1;
			for (int digit = 0; digit <= 9; ++digit)
			{
				const int nextMask = digitMask | (1 << digit);
				const double probability = positionWeights[position][digit] *
					completionProbabilities[position + 1][nextMask];
				if (probability <= 0.0)
				{
					continue;
				}

				cumulativeProbability += probability;
				selectedDigit = digit;
				if (targetProbability <= cumulativeProbability)
				{
					break;
				}
			}

			if (selectedDigit < 0)
			{
				return vector<int>();
			}

			prediction.push_back(selectedDigit);
			digitMask |= 1 << selectedDigit;
		}

		return prediction;
	}

	CString FormatStatsLine(const ColumnStats& stats)
	{
		CString mean = FormatDouble(stats.mean);
		CString median = FormatDouble(stats.median);
		CString harmonic = FormatDouble(stats.harmonicMean);
		CString geometric = FormatDouble(stats.geometricMean);
		CString rms = FormatDouble(stats.rootMeanSquare);
		CString stdDev = FormatDouble(stats.standardDeviation);
		CString q1 = FormatDouble(stats.lowerQuartile);
		CString q3 = FormatDouble(stats.upperQuartile);

		CString line;
		line.Format(L"%s | Mean=%s | Min=%d | Max=%d | Median=%s | Mode=%d | Harmonic=%s | Geometric=%s | RMS=%s | StdDev=%s | LowerQ=%s | UpperQ=%s",
			stats.label.GetString(),
			mean.GetString(),
			stats.minValue,
			stats.maxValue,
			median.GetString(),
			stats.modeValue,
			harmonic.GetString(),
			geometric.GetString(),
			rms.GetString(),
			stdDev.GetString(),
			q1.GetString(),
			q3.GetString());
		return line;
	}

	// --- Strategy: Balanced selection (near-equal small/large and even/odd) ---
	vector<int> BuildBalancedPrediction(const map<int, int>& freqs, int mainCount, mt19937& rng)
	{
		if (freqs.empty() || mainCount <= 0)
			return vector<int>();

		const int minNum = freqs.begin()->first;
		const int maxNum = freqs.rbegin()->first;
		const int midPoint = minNum + (maxNum - minNum) / 2;
		const int targetSmall = (mainCount + 1) / 2;
		const int targetEven = (mainCount + 1) / 2;
		vector<int> result;
		map<int, int> remaining(freqs);
		int smallCount = 0;
		int evenCount = 0;

		while (static_cast<int>(result.size()) < mainCount && !remaining.empty())
		{
			int bestScore = -1;
			map<int, int> bestCandidates;
			for (map<int, int>::const_iterator it = remaining.begin(); it != remaining.end(); ++it)
			{
				const bool isSmall = it->first <= midPoint;
				const bool isEven = (it->first % 2) == 0;
				const int score =
					((isSmall && smallCount < targetSmall) ? 1 : 0) +
					((isEven && evenCount < targetEven) ? 1 : 0);
				if (score > bestScore)
				{
					bestScore = score;
					bestCandidates.clear();
				}
				if (score == bestScore)
				{
					bestCandidates[it->first] = it->second;
				}
			}

			const int n = ChooseWeightedNumber(bestCandidates, rng);
			result.push_back(n);
			remaining.erase(n);
			if (n <= midPoint)
				++smallCount;
			if ((n % 2) == 0)
				++evenCount;
		}

		sort(result.begin(), result.end());
		return result;
	}

	// --- Strategy: Delta technique ---
	vector<int> BuildDeltaPrediction(int mainCount, int minNum, int maxNum, mt19937& rng)
	{
		vector<int> result;
		if (mainCount <= 0 || maxNum - minNum + 1 < mainCount)
			return result;

		uniform_int_distribution<int> startDist(minNum, maxNum - mainCount + 1);
		int current = startDist(rng);
		result.push_back(current);

		for (int remaining = mainCount - 1; remaining > 0; --remaining)
		{
			uniform_int_distribution<int> deltaDist(1, maxNum - current - remaining + 1);
			current += deltaDist(rng);
			result.push_back(current);
		}

		return result;
	}

	// --- Strategy: Wheel system (all C(n,mainCount) combinations) ---
	vector<vector<int> > BuildWheelPredictions(const vector<int>& wheelNums, int mainCount, mt19937& rng)
	{
		vector<vector<int> > result;
		const int n = static_cast<int>(wheelNums.size());
		if (n < mainCount)
			return result;

		size_t combinationCount = 1;
		const int selectionCount = min(mainCount, n - mainCount);
		bool hasMoreThanLimit = false;
		for (int index = 1; index <= selectionCount; ++index)
		{
			combinationCount = (combinationCount * static_cast<size_t>(n - selectionCount + index)) /
				static_cast<size_t>(index);
			if (combinationCount > 100U)
			{
				hasMoreThanLimit = true;
				break;
			}
		}

		if (hasMoreThanLimit)
		{
			set<vector<int> > selectedCombinations;
			vector<int> shuffledNumbers(wheelNums);
			while (result.size() < 100U)
			{
				shuffle(shuffledNumbers.begin(), shuffledNumbers.end(), rng);
				vector<int> combination(shuffledNumbers.begin(), shuffledNumbers.begin() + mainCount);
				sort(combination.begin(), combination.end());
				if (selectedCombinations.insert(combination).second)
				{
					result.push_back(combination);
				}
			}
			return result;
		}

		vector<int> indices(mainCount);
		for (int i = 0; i < mainCount; ++i)
			indices[i] = i;

		while (true)
		{
			vector<int> combo;
			for (int i = 0; i < mainCount; ++i)
				combo.push_back(wheelNums[indices[i]]);
			sort(combo.begin(), combo.end());
			result.push_back(combo);

			int pos = mainCount - 1;
			while (pos >= 0 && indices[pos] == pos + n - mainCount)
				--pos;
			if (pos < 0)
				break;
			++indices[pos];
			for (int j = pos + 1; j < mainCount; ++j)
				indices[j] = indices[j - 1] + 1;
		}

		return result;
	}

	vector<int> BuildCoveragePrediction(
		const map<int, int>& frequencies,
		int numberCount,
		const map<int, int>& previousExposure,
		mt19937& rng)
	{
		map<int, int> remaining(frequencies);
		vector<int> result;
		while (!remaining.empty() && static_cast<int>(result.size()) < numberCount)
		{
			vector<int> candidates;
			vector<double> weights;
			for (map<int, int>::const_iterator it = remaining.begin(); it != remaining.end(); ++it)
			{
				map<int, int>::const_iterator exposure = previousExposure.find(it->first);
				const int exposureCount = (exposure == previousExposure.end()) ? 0 : exposure->second;
				const double historicalWeight = (it->second > 0) ? it->second : 1;
				candidates.push_back(it->first);
				weights.push_back(historicalWeight / (1.0 + exposureCount));
			}

			discrete_distribution<size_t> distribution(weights.begin(), weights.end());
			const int selected = candidates[distribution(rng)];
			result.push_back(selected);
			remaining.erase(selected);
		}

		sort(result.begin(), result.end());
		return result;
	}

	// --- Strategy: Unusual numbers (boost weight for numbers > 31) ---
	vector<int> BuildUnusualPrediction(const map<int, int>& freqs, int mainCount, mt19937& rng)
	{
		const int threshold = 31;
		map<int, int> adjusted;
		for (map<int, int>::const_iterator it = freqs.begin(); it != freqs.end(); ++it)
		{
			int weight = (it->second > 0) ? it->second : 1;
			if (it->first > threshold)
				weight *= 3;
			adjusted[it->first] = weight;
		}
		return BuildPredictionLine(adjusted, mainCount, rng);
	}

	// --- Strategy: Consecutive pair (ensure at least one consecutive pair when possible) ---
	vector<int> BuildConsecutivePrediction(const map<int, int>& freqs, int mainCount, mt19937& rng)
	{
		if (mainCount < 2 || static_cast<int>(freqs.size()) < mainCount)
			return BuildPredictionLine(freqs, mainCount, rng);

		vector<pair<int, int> > adjacentPairs;
		vector<double> pairWeights;
		for (map<int, int>::const_iterator it = freqs.begin(); it != freqs.end(); ++it)
		{
			map<int, int>::const_iterator next = freqs.find(it->first + 1);
			if (next != freqs.end())
			{
				adjacentPairs.push_back(make_pair(it->first, next->first));
				const double firstWeight = (it->second > 0) ? it->second : 1;
				const double secondWeight = (next->second > 0) ? next->second : 1;
				pairWeights.push_back(firstWeight * secondWeight);
			}
		}

		if (adjacentPairs.empty())
			return BuildPredictionLine(freqs, mainCount, rng);

		discrete_distribution<size_t> pairDistribution(pairWeights.begin(), pairWeights.end());
		const pair<int, int>& pair = adjacentPairs[pairDistribution(rng)];
		vector<int> result;
		result.push_back(pair.first);
		result.push_back(pair.second);

		map<int, int> remaining(freqs);
		remaining.erase(pair.first);
		remaining.erase(pair.second);
		while (static_cast<int>(result.size()) < mainCount && !remaining.empty())
		{
			const int n = ChooseWeightedNumber(remaining, rng);
			result.push_back(n);
			remaining.erase(n);
		}

		sort(result.begin(), result.end());
		return result;
	}

	// --- Strategy: Magic sum (adaptive range based on game) ---
	vector<int> BuildMagicSumPrediction(const map<int, int>& freqs, int mainCount, mt19937& rng, int minSum, int maxSum)
	{
		vector<int> closestPrediction;
		int closestDistance = (numeric_limits<int>::max)();
		for (int attempt = 0; attempt < 300; ++attempt)
		{
			vector<int> pred = BuildPredictionLine(freqs, mainCount, rng);
			int total = 0;
			for (size_t i = 0; i < pred.size(); ++i)
				total += pred[i];
			if (total >= minSum && total <= maxSum)
				return pred;

			const int distance = (total < minSum) ? minSum - total : total - maxSum;
			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestPrediction = pred;
			}
		}
		return closestPrediction;
	}

	// --- Strategy: Matrix technique (3x3 matrix row/col/diagonal sums) ---
	vector<int> BuildMatrixPrediction(
		const map<int, int>& frequencies,
		int mainCount,
		int minNum,
		int maxNum,
		mt19937& rng)
	{
		const int cellMax = max(1, maxNum / 3);
		uniform_int_distribution<int> cellDist(1, cellMax);

		for (int attempt = 0; attempt < 100; ++attempt)
		{
			int matrix[3][3];
			for (int r = 0; r < 3; ++r)
				for (int c = 0; c < 3; ++c)
					matrix[r][c] = cellDist(rng);

			vector<int> candidates;
			for (int r = 0; r < 3; ++r)
			{
				const int s = matrix[r][0] + matrix[r][1] + matrix[r][2];
				if (s >= minNum && s <= maxNum)
					candidates.push_back(s);
			}
			for (int c = 0; c < 3; ++c)
			{
				const int s = matrix[0][c] + matrix[1][c] + matrix[2][c];
				if (s >= minNum && s <= maxNum)
					candidates.push_back(s);
			}
			const int d1 = matrix[0][0] + matrix[1][1] + matrix[2][2];
			const int d2 = matrix[0][2] + matrix[1][1] + matrix[2][0];
			if (d1 >= minNum && d1 <= maxNum) candidates.push_back(d1);
			if (d2 >= minNum && d2 <= maxNum) candidates.push_back(d2);

			sort(candidates.begin(), candidates.end());
			candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());

			if (static_cast<int>(candidates.size()) >= mainCount)
			{
				shuffle(candidates.begin(), candidates.end(), rng);
				vector<int> result(candidates.begin(), candidates.begin() + mainCount);
				sort(result.begin(), result.end());
				return result;
			}

			map<int, int> remaining(frequencies);
			for (size_t index = 0; index < candidates.size(); ++index)
			{
				remaining.erase(candidates[index]);
			}
			vector<int> result(candidates);
			while (static_cast<int>(result.size()) < mainCount && !remaining.empty())
			{
				const int selected = ChooseWeightedNumber(remaining, rng);
				result.push_back(selected);
				remaining.erase(selected);
			}
			if (static_cast<int>(result.size()) >= mainCount)
			{
				sort(result.begin(), result.end());
				return result;
			}
		}

		return BuildPredictionLine(frequencies, mainCount, rng);
	}

	// --- Shared strategy dispatcher called by all game handlers ---
	void ApplyStrategyPredictions(
		CStringArray& vstrOutput,
		const map<int, int>& mainFreqs,
		int predCount,
		int predRows,
		BOOL bBalanced, BOOL bDelta, BOOL bWheel, const CString& strWheelNums,
		BOOL bUnusual, BOOL bConsecutive, BOOL bMagicSum, BOOL bMatrix, BOOL bCoverage,
		mt19937& rng,
		const map<int, int>* starFreqs = NULL,
		int starCount = 0,
		bool includeVikingNumber = false)
	{
		if (mainFreqs.empty())
			return;

		const bool hasStars = (starFreqs != NULL && starCount > 0 && !starFreqs->empty());
		const int maxNum = mainFreqs.rbegin()->first;
		double totalFrequency = 0.0;
		double weightedNumberTotal = 0.0;
		for (map<int, int>::const_iterator it = mainFreqs.begin(); it != mainFreqs.end(); ++it)
		{
			const double frequency = (it->second > 0) ? it->second : 1;
			totalFrequency += frequency;
			weightedNumberTotal += it->first * frequency;
		}
		const double expectedSum = (totalFrequency > 0.0)
			? (predCount * weightedNumberTotal / totalFrequency)
			: 0.0;
		const int magicSumMin = max(predCount, static_cast<int>(floor(expectedSum * 0.8)));
		const int magicSumMax = static_cast<int>(ceil(expectedSum * 1.2));
		const auto appendVikingSuffix = [&](wostringstream& ws)
		{
			if (!includeVikingNumber)
				return;
			uniform_int_distribution<int> vikingDistribution(1, 5);
			const int vikingNumber = vikingDistribution(rng);
			const CString numberLabel = LocalizeText(L"Viking number: ", L"Viking-numero: ");
			const CString selectionNote = LocalizeText(L"randomly selected for the prediction",
				L"arvottu ennusteeseen");
			ws << L" | " << numberLabel.GetString()
				<< vikingNumber << L" ("
				<< selectionNote.GetString() << L")";
		};

		CString strategyLog = LocalizeText(L"Active strategies:", L"Käytössä olevat strategiat:");
		bool anyActive = false;

		auto appendMainResult = [&](int index, const vector<int>& pred, const vector<int>& starPred = vector<int>())
		{
			wostringstream ws;
			ws << GetPredictionPrefix() << (index + 1) << L": ";
			for (size_t j = 0; j < pred.size(); ++j) { if (j > 0) ws << L", "; ws << pred[j]; }
			if (!starPred.empty())
			{
				const CString starLabel = LocalizeText(L"Stars: ", L"Tähdet: ");
				ws << L" | " << starLabel.GetString();
				for (size_t j = 0; j < starPred.size(); ++j) { if (j > 0) ws << L", "; ws << starPred[j]; }
			}
			appendVikingSuffix(ws);
			AddCStringArrayText(vstrOutput, ws.str());
		};

		if (bBalanced)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Balanced,", L" Tasapainotettu,");
			vstrOutput.Add(L" ");
			vstrOutput.Add(LocalizeText(L"Balanced selection (50% small/large, 50% even/odd):",
				L"Tasapainotettu valinta (50% pieniä/suuria, 50% parillisia/parittomia):"));
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred = BuildBalancedPrediction(mainFreqs, predCount, rng);
				vector<int> starPred = hasStars ? BuildPredictionLine(*starFreqs, starCount, rng) : vector<int>();
				appendMainResult(i, pred, starPred);
			}
		}

		if (bDelta)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Delta,", L" Delta,");
			vstrOutput.Add(L" ");
			vstrOutput.Add(LocalizeText(L"Delta technique:", L"Delta-tekniikka:"));
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred = BuildDeltaPrediction(predCount, mainFreqs.begin()->first, maxNum, rng);
				vector<int> starPred = hasStars ? BuildPredictionLine(*starFreqs, starCount, rng) : vector<int>();
				appendMainResult(i, pred, starPred);
			}
		}

		if (bWheel)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Wheel,", L" Onnenpyörä,");
			vector<int> wheelNums = ParseNumbersFromLine(strWheelNums);
			vector<int> validWheel;
			for (size_t i = 0; i < wheelNums.size(); ++i)
			{
				if (wheelNums[i] >= 1 && wheelNums[i] <= maxNum)
					validWheel.push_back(wheelNums[i]);
			}
			sort(validWheel.begin(), validWheel.end());
			validWheel.erase(unique(validWheel.begin(), validWheel.end()), validWheel.end());
			vstrOutput.Add(L" ");
			if (static_cast<int>(validWheel.size()) <= predCount)
			{
				CString msg;
				msg.Format(LocalizeText(L"Wheel system: need at least %d numbers (got %d).",
					L"Onnenpyörä: tarvitsee vähintään %d numeroa (saatiin %d)."),
					predCount + 1, static_cast<int>(validWheel.size()));
				vstrOutput.Add(msg);
			}
			else
			{
				vstrOutput.Add(LocalizeText(L"Wheel system (up to 100 unique combinations; random subset when needed):",
					L"Onnenpyörä (enintään 100 uniikkia yhdistelmää; tarvittaessa satunnainen osajoukko):"));
				vector<vector<int> > combos = BuildWheelPredictions(validWheel, predCount, rng);
				for (size_t i = 0; i < combos.size(); ++i)
				{
					wostringstream ws;
					const CString combinationLabel = LocalizeText(L"Combination ", L"Yhdistelmä ");
					ws << combinationLabel.GetString() << (i + 1) << L": ";
					for (size_t j = 0; j < combos[i].size(); ++j) { if (j > 0) ws << L", "; ws << combos[i][j]; }
					if (hasStars)
					{
						vector<int> starPred = BuildPredictionLine(*starFreqs, starCount, rng);
						const CString starLabel = LocalizeText(L"Stars:", L"Tähdet:");
						ws << L" | " << starLabel.GetString() << L" ";
						for (size_t j = 0; j < starPred.size(); ++j) { if (j > 0) ws << L", "; ws << starPred[j]; }
					}
					appendVikingSuffix(ws);
					AddCStringArrayText(vstrOutput, ws.str());
				}
			}
		}

		if (bUnusual)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Unusual,", L" Epätavalliset numerot,");
			vstrOutput.Add(L" ");
			vstrOutput.Add(LocalizeText(L"Unusual numbers strategy (prefer >31):",
				L"Epätavallisten numeroiden strategia (suosi >31):"));
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred = BuildUnusualPrediction(mainFreqs, predCount, rng);
				vector<int> starPred = hasStars ? BuildPredictionLine(*starFreqs, starCount, rng) : vector<int>();
				appendMainResult(i, pred, starPred);
			}
		}

		if (bConsecutive)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Consecutive,", L" Peräkkäiset numerot,");
			vstrOutput.Add(L" ");
			vstrOutput.Add(LocalizeText(L"Consecutive pair strategy:", L"Peräkkäisten parien strategia:"));
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred = BuildConsecutivePrediction(mainFreqs, predCount, rng);
				vector<int> starPred = hasStars ? BuildPredictionLine(*starFreqs, starCount, rng) : vector<int>();
				appendMainResult(i, pred, starPred);
			}
		}

		if (bMagicSum)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" MagicSum,", L" Maaginen summa,");
			vstrOutput.Add(L" ");
			CString header;
			const CString headerFormat = LocalizeText(L"Magic sum strategy (%d\u2013%d):",
				L"Maaginen summa -strategia (%d\u2013%d):");
			header.Format(headerFormat.GetString(), magicSumMin, magicSumMax);
			vstrOutput.Add(header);
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred = BuildMagicSumPrediction(mainFreqs, predCount, rng, magicSumMin, magicSumMax);
				int total = 0;
				for (size_t j = 0; j < pred.size(); ++j) total += pred[j];
				vector<int> starPred = hasStars ? BuildPredictionLine(*starFreqs, starCount, rng) : vector<int>();
				wostringstream ws;
				ws << GetPredictionPrefix() << (i + 1) << L": ";
				for (size_t j = 0; j < pred.size(); ++j) { if (j > 0) ws << L", "; ws << pred[j]; }
				if (!starPred.empty())
				{
					const CString starLabel = LocalizeText(L"Stars:", L"Tähdet:");
					ws << L" | " << starLabel.GetString() << L" ";
					for (size_t j = 0; j < starPred.size(); ++j) { if (j > 0) ws << L", "; ws << starPred[j]; }
				}
				appendVikingSuffix(ws);
				const CString sumLabel = LocalizeText(L"sum=", L"summa=");
				ws << L" (" << sumLabel.GetString() << total << L")";
				if (total < magicSumMin || total > magicSumMax)
				{
					const CString missedTargetNote = LocalizeText(
						L" [target missed; closest sampled row]",
						L" [tavoite jäi osumatta; lähin näyte] ");
					ws << missedTargetNote.GetString();
				}
				AddCStringArrayText(vstrOutput, ws.str());
			}
		}

		if (bMatrix)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Matrix,", L" Matriisi,");
			vstrOutput.Add(L" ");
			vstrOutput.Add(LocalizeText(L"Matrix technique:", L"Matriisitekniikka:"));
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred = BuildMatrixPrediction(mainFreqs, predCount, mainFreqs.begin()->first, maxNum, rng);
				vector<int> starPred = hasStars ? BuildPredictionLine(*starFreqs, starCount, rng) : vector<int>();
				appendMainResult(i, pred, starPred);
			}
		}

		if (bCoverage)
		{
			anyActive = true;
			strategyLog += LocalizeText(L" Coverage,", L" Rivien hajautus,");
			vstrOutput.Add(L" ");
			vstrOutput.Add(LocalizeText(L"Portfolio coverage (reduce overlap across rows):",
				L"Rivien hajautus (vähennä päällekkäisyyttä riveillä):"));

			map<int, int> mainExposure;
			map<int, int> starExposure;
			set<vector<int> > usedMainRows;
			for (int i = 0; i < predRows; ++i)
			{
				vector<int> pred;
				for (int attempt = 0; attempt < 2048; ++attempt)
				{
					pred = BuildCoveragePrediction(mainFreqs, predCount, mainExposure, rng);
					if (usedMainRows.insert(pred).second)
					{
						break;
					}
				}

				for (size_t numberIndex = 0; numberIndex < pred.size(); ++numberIndex)
				{
					++mainExposure[pred[numberIndex]];
				}

				vector<int> starPred;
				if (hasStars)
				{
					starPred = BuildCoveragePrediction(*starFreqs, starCount, starExposure, rng);
					for (size_t starIndex = 0; starIndex < starPred.size(); ++starIndex)
					{
						++starExposure[starPred[starIndex]];
					}
				}
				appendMainResult(i, pred, starPred);
			}
		}

		vstrOutput.Add(L" ");
		if (anyActive)
		{
			if (strategyLog.Right(1) == L",")
				strategyLog = strategyLog.Left(strategyLog.GetLength() - 1);
			vstrOutput.Add(strategyLog);
		}
		else
		{
			vstrOutput.Add(LocalizeText(L"Active strategies: none (frequency analysis only)",
				L"Käytössä olevat strategiat: ei valintoja (vain frekvenssianalyysi)"));
		}
	}
}

// CLottoGuiDoc

IMPLEMENT_DYNCREATE(CLottoGuiDoc, CDocument)

BEGIN_MESSAGE_MAP(CLottoGuiDoc, CDocument)
	ON_COMMAND(ID_LOTTOOPTIONS, &CLottoGuiDoc::OnLottooptions)
	ON_COMMAND(ID_LOTTOOPTIONS_CHECKRESULTS, &CLottoGuiDoc::OnCheckResults)
	ON_UPDATE_COMMAND_UI(ID_LOTTOOPTIONS_CHECKRESULTS, &CLottoGuiDoc::OnUpdateCheckResults)
	ON_COMMAND(ID_LOTTOOPTIONS_SUOMENLOTTO, &CLottoGuiDoc::OnSuomenlotto)
	ON_COMMAND(ID_LOTTOOPTIONS_MILLICSV, &CLottoGuiDoc::OnMilli)
	ON_COMMAND(ID_LOTTOOPTIONS_KTEMCSV, &CLottoGuiDoc::OnKtem)
	ON_COMMAND(ID_LOTTOOPTIONS_KENOCSV, &CLottoGuiDoc::OnKeno)
	ON_COMMAND(ID_LOTTOOPTIONS_EUROJACKPOTCSV, &CLottoGuiDoc::OnEurojackpot)
	ON_COMMAND(ID_LOTTOOPTIONS_VIKINGLOTTOCSV, &CLottoGuiDoc::OnVikinglotto)
	ON_COMMAND(ID_LOTTOOPTIONS_JOKERICSV, &CLottoGuiDoc::OnJokeri)
END_MESSAGE_MAP()


// CLottoGuiDoc construction/destruction

CLottoGuiDoc::CLottoGuiDoc()
{
	// TODO: add one-time construction code here
	m_nAmount = 7;
	m_nRounds = 1;
	m_strNums = _T("1 3 12 14 16 17 18 23 24 26 30 37");
	m_strAllotedNums = _T("1 3 12 17 23 26 37");
	m_nMinNum = 1;
	m_nMaxNum = 40;
	m_strSampleNums = _T("");
	m_strCurrentGameType = _T("");
	m_bJokeriDouble = FALSE;
	m_nKenoLevel = 0;
	m_nKenoStakeCents = 0;
	m_bKenoKunkkukerroin = FALSE;
	m_strKenoTotalPrize.Empty();
	m_nKtemStakeCents = 0;
	m_strKtemTotalPrize.Empty();
	m_nLines = 0;
}

CLottoGuiDoc::~CLottoGuiDoc()
{
}

BOOL CLottoGuiDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CLottoGuiDoc serialization

void CLottoGuiDoc::Serialize(CArchive& ar)
{
	 m_vstrNums.Serialize(ar);

	if (ar.IsStoring())
	{
		// TODO: add storing code here
		ar << m_strSampleNums;
		ar << m_nLines;
		// ar << m_vstrNums;
	}
	else
	{
		// TODO: add loading code here
		ar >> m_strSampleNums;
		ar >> m_nLines;
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CLottoGuiDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CLottoGuiDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CLottoGuiDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CLottoGuiDoc diagnostics

#ifdef _DEBUG
void CLottoGuiDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLottoGuiDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLottoGuiDoc commands


void CLottoGuiDoc::OnLottooptions()
{
	CLottoDialog aDlg;

	aDlg.m_nMinNum = m_nMinNum;
	aDlg.m_nMaxNum = m_nMaxNum;
	aDlg.m_nAmount = m_nAmount;
	aDlg.m_nRounds = m_nRounds;
	aDlg.m_strNums = m_strNums;

	if (aDlg.DoModal() != IDOK)
	{
		return;
	}

	m_nMinNum = aDlg.m_nMinNum;
	m_nMaxNum = aDlg.m_nMaxNum;
	m_nAmount = aDlg.m_nAmount;
	m_nRounds = aDlg.m_nRounds;
	m_strNums = aDlg.m_strNums;
	m_vstrNums.RemoveAll();
	m_strCurrentGameType = L"Custom";
	ClearResultCheck();

	vector<int> input;
	if (aDlg.m_bGivenNumber == TRUE)
	{
		extractIntegerWords(m_strNums, input);
		m_strSampleNums = (PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
			? L"Esimerkkinumerot: "
			: L"Sample numbers: ";
		m_strSampleNums += aDlg.m_strNums;
	}
	else
	{
		m_strSampleNums = (PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
			? L"Väli: "
			: L"Range: ";
		for (int value = m_nMinNum; value <= m_nMaxNum; ++value)
		{
			input.push_back(value);
			CString numberText;
			numberText.Format(L"%d ", value);
			m_strSampleNums += numberText;
		}
	}

	sort(input.begin(), input.end());
	input.erase(unique(input.begin(), input.end()), input.end());

	const int n = static_cast<int>(input.size());
	if (n <= 0)
	{
		AfxMessageBox((PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
			? L"Anna vähintään yksi kelvollinen numero."
			: L"Please provide at least one valid number.");
		return;
	}

	if (m_nAmount > n)
	{
		m_nAmount = n;
	}

	if (m_nAmount <= 0)
	{
		AfxMessageBox((PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
			? L"Valitse vähintään yksi numero per rivi."
			: L"Please select at least one number per line.");
		return;
	}

	mt19937 generator = CreateRandomGenerator();
	vector<int> lottoline;

	for (int roundIndex = 0; roundIndex < m_nRounds; ++roundIndex)
	{
		CString lineText;
		CString roundNumber;
		roundNumber.Format(L"%d", roundIndex + 1);
		lineText = _T("Line ") + roundNumber + _T(": ");

		shuffle(input.begin(), input.end(), generator);
		lottoline.assign(input.begin(), input.begin() + m_nAmount);
		sort(lottoline.begin(), lottoline.end());
		ostringstream stream;
		if (!lottoline.empty())
		{
			copy(lottoline.begin(), lottoline.end() - 1, ostream_iterator<int>(stream, ", "));
			stream << lottoline.back();
		}

		const string streamText = stream.str();
		lineText += ToStableCString(wstring(streamText.begin(), streamText.end()));
		m_vstrNums.Add(lineText);
		lottoline.clear();
	}

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 4;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnCheckResults()
{
	if (!HasPredictionRows(m_vstrNums))
	{
		AfxMessageBox(LocalizeText(L"Generate predictions before checking the results.",
			L"Luo ennusteet ennen tulosten tarkistamista."));
		return;
	}

	CCheckResultsDlg dialog;
	dialog.m_strActualNumbers = m_strActualResults;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	CString actualResults = dialog.m_strActualNumbers;
	actualResults.Trim();
	if (actualResults.IsEmpty())
	{
		AfxMessageBox(LocalizeText(L"Enter the drawn numbers first.",
			L"Syötä ensin oikeat tulosnumerot."));
		return;
	}

	vector<int> actualMainNumbers;
	vector<int> actualBonusNumbers;
	if (!ParseActualResults(actualResults, actualMainNumbers, actualBonusNumbers))
	{
		AfxMessageBox(LocalizeText(
			L"Enter whole numbers separated by spaces, commas, or semicolons. Separate bonus numbers with |.",
			L"Syötä kokonaisluvut välilyönnillä, pilkulla tai puolipisteellä erotettuina. Erota lisänumerot merkillä |."));
		return;
	}

	m_strActualResults = actualResults;
	m_vResultCounts.clear();
	m_vMatchedNumbers.clear();
	m_vPrizeTexts.clear();
	m_strKenoTotalPrize.Empty();
	m_strKtemTotalPrize.Empty();
	const bool checkingKeno = m_strCurrentGameType.Find(L"Keno") >= 0;
	const bool checkingKtem = m_strCurrentGameType.Find(L"KTEM") >= 0;
	long long totalKenoPrizeCents = 0;
	long long totalKtemPrizeCents = 0;
	for (INT_PTR index = 0; index < m_vstrNums.GetSize(); ++index)
	{
		vector<int> predictedMainNumbers;
		vector<int> predictedBonusNumbers;
		if (!ParsePredictionRow(m_vstrNums.GetAt(index), predictedMainNumbers, predictedBonusNumbers))
		{
			m_vResultCounts.push_back(-1);
			m_vMatchedNumbers.push_back(vector<int>());
			m_vPrizeTexts.push_back(L"");
			continue;
		}

		vector<int> matches;
		vector<int> bonusMatches;
		FindMatches(predictedMainNumbers, actualMainNumbers, matches);
		FindMatches(predictedBonusNumbers, actualBonusNumbers, bonusMatches);
		const int mainMatches = static_cast<int>(matches.size());
		const int bonusMatchCount = static_cast<int>(bonusMatches.size());
		const int correctMatches = mainMatches + bonusMatchCount;
		const int kenoPrizeCents = checkingKeno
			? LottoGuiCore::GetKenoPrizeCents(
				m_nKenoLevel, mainMatches, m_nKenoStakeCents, m_bKenoKunkkukerroin != FALSE)
			: 0;
		const int ktemPrizeCents = checkingKtem
			? LottoGuiCore::GetKtemPrizeCents(mainMatches, m_nKtemStakeCents)
			: 0;
		totalKenoPrizeCents += kenoPrizeCents;
		totalKtemPrizeCents += ktemPrizeCents;
		m_vResultCounts.push_back(correctMatches);
		m_vMatchedNumbers.push_back(matches);
		for (size_t index = 0; index < bonusMatches.size(); ++index)
		{
			m_vMatchedNumbers.back().push_back(bonusMatches[index]);
		}
		m_vPrizeTexts.push_back(DescribePrize(
			m_strCurrentGameType, mainMatches, bonusMatchCount, m_nAmount,
			m_bJokeriDouble, kenoPrizeCents, ktemPrizeCents));
	}

	if (checkingKeno)
	{
		m_strKenoTotalPrize =
			LocalizeText(L"Total Keno winnings: ", L"Kenon voittoyhteensä: ") +
			FormatPrizeAmount(totalKenoPrizeCents);
	}
	if (checkingKtem)
	{
		m_strKtemTotalPrize =
			LocalizeText(L"Total KTEM winnings: ", L"KTEM-voittojen yhteissumma: ") +
			FormatPrizeAmount(totalKtemPrizeCents);
	}

	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnUpdateCheckResults(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasPredictionRows(m_vstrNums));
}

void CLottoGuiDoc::ClearResultCheck()
{
	m_strActualResults.Empty();
	m_vResultCounts.clear();
	m_vMatchedNumbers.clear();
	m_vPrizeTexts.clear();
	m_strKenoTotalPrize.Empty();
	m_strKtemTotalPrize.Empty();
}

void CLottoGuiDoc::OnSuomenlotto()
{
	CSuomenLottoDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;
	dialog.m_bPlus = ReadPricePreference(_T("SuomenLottoPlus"), FALSE, FALSE, TRUE);

	if (dialog.DoModal() != IDOK)
	{
		return;
	}
	WritePricePreference(_T("SuomenLottoPlus"), dialog.m_bPlus);

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadSuomenLottoRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\SuomenLottoData.csv.",
			L"Tiedostoa data\\SuomenLottoData.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = DetectMainNumberCount(dominantColumnCount);
	const int extraCount = max(0, dominantColumnCount - mainCount);

	map<int, int> mainFrequencies;
	vector<map<int, int> > extraFrequencies;
	extraFrequencies.resize((extraCount > 0) ? extraCount : 0);

	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++mainFrequencies[row[columnIndex]];
		}

		for (int extraIndex = 0; extraIndex < extraCount; ++extraIndex)
		{
			const int column = mainCount + extraIndex;
			if (column < static_cast<int>(row.size()))
			{
				++extraFrequencies[extraIndex][row[column]];
			}
		}
	}

	mt19937 generator = CreateRandomGenerator();
	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;
	m_strCurrentGameType = L"Suomen Lotto";

	m_strSampleNums.Format(LocalizeText(
		L"Suomen Lotto CSV: %s | Prediction rows: %d | Data rows: %d | Main-number columns detected: %d",
		L"Suomen Lotto CSV: %s | Ennustusrivejä: %d | Data-rivejä: %d | Päänumero-sarakkeita havaittu: %d"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:", L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		const wstring outputText = output.str();

		m_vstrNums.Add(ToStableCString(outputText));
	}

	ApplyStrategyPredictions(m_vstrNums, mainFrequencies, mainCount, dialog.m_nPredictionRows,
		dialog.m_bBalanced, dialog.m_bDelta, dialog.m_bWheel, dialog.m_strWheelNums,
		dialog.m_bUnusual, dialog.m_bConsecutive, dialog.m_bMagicSum, dialog.m_bMatrix, dialog.m_bCoverage,
		generator);
	AppendTicketCost(m_strSampleNums, m_vstrNums,
		LotteryPrices::kSuomenLottoBaseCents +
		(dialog.m_bPlus ? LotteryPrices::kSuomenLottoPlusCents : 0));

	vector<ColumnStats> allStats;
	for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][columnIndex]);
			}
		}

		allStats.push_back(ComputeColumnStats(columnValues, GetColumnLabel(columnIndex, mainCount, dominantColumnCount)));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(mainCount));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Mean", L"Keskiarvo"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Min", L"Minimi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Max", L"Maksimi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Median", L"Mediaani"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Mode", L"Moodi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Harmonic", L"Harmoninen"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Geometric", L"Geometrinen"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"RMS", L"RMS"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"StdDev", L"Keskihajonta"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"LowerQ", L"Alempi kvartiili"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"UpperQ", L"Ylempi kvartiili"), values));

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnMilli()
{
	CMilliDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadMilliRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\MilliData.csv.",
			L"Tiedostoa data\\MilliData.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 6) ? 6 : dominantColumnCount;

	map<int, int> mainFrequencies;
	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++mainFrequencies[row[columnIndex]];
		}
	}

	mt19937 generator = CreateRandomGenerator();
	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;
	m_strCurrentGameType = L"Milli";

	m_strSampleNums.Format(LocalizeText(
		L"Milli CSV: %s | Prediction rows: %d | Data rows: %d | Number columns detected: %d",
		L"Milli CSV: %s | Ennustusrivejä: %d | Data-rivejä: %d | Numero-sarakkeita havaittu: %d"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:", L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		const wstring outputText = output.str();

		m_vstrNums.Add(ToStableCString(outputText));
	}

	ApplyStrategyPredictions(m_vstrNums, mainFrequencies, mainCount, dialog.m_nPredictionRows,
		dialog.m_bBalanced, dialog.m_bDelta, dialog.m_bWheel, dialog.m_strWheelNums,
		dialog.m_bUnusual, dialog.m_bConsecutive, dialog.m_bMagicSum, dialog.m_bMatrix, dialog.m_bCoverage,
		generator);
	AppendTicketCost(m_strSampleNums, m_vstrNums, LotteryPrices::kMilliBaseCents);

	vector<ColumnStats> allStats;
	for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][columnIndex]);
			}
		}

		CString label;
		label.Format(IsFinnishLanguage() ? L"Nro%d" : L"Nr%d", columnIndex + 1);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(mainCount));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Mean", L"Keskiarvo"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Min", L"Minimi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Max", L"Maksimi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Median", L"Mediaani"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Mode", L"Moodi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Harmonic", L"Harmoninen"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Geometric", L"Geometrinen"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"RMS", L"RMS"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"StdDev", L"Keskihajonta"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"LowerQ", L"Alempi kvartiili"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"UpperQ", L"Ylempi kvartiili"), values));

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnKtem()
{
	CKtemDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;
	const int ktemDefaultStakeIndex = (LotteryPrices::kKtemDefaultStakeCents -
		LotteryPrices::kKtemMinimumStakeCents) / LotteryPrices::kStakeStepCents;
	dialog.m_nStakeIndex = ReadPricePreference(_T("KtemStakeIndex"), ktemDefaultStakeIndex, 0,
		(LotteryPrices::kKtemMaximumStakeCents - LotteryPrices::kKtemMinimumStakeCents) /
		LotteryPrices::kStakeStepCents);
	dialog.m_bOnnenapila = ReadPricePreference(_T("KtemOnnenapila"), FALSE, FALSE, TRUE);

	if (dialog.DoModal() != IDOK)
	{
		return;
	}
	WritePricePreference(_T("KtemStakeIndex"), dialog.m_nStakeIndex);
	WritePricePreference(_T("KtemOnnenapila"), dialog.m_bOnnenapila);

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadKtemRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\KTEM.csv.",
			L"Tiedostoa data\\KTEM.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 12) ? 12 : dominantColumnCount;
	const int ktemStakeCents = LotteryPrices::kKtemMinimumStakeCents +
		(dialog.m_nStakeIndex * LotteryPrices::kStakeStepCents);

	map<int, int> mainFrequencies;
	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++mainFrequencies[row[columnIndex]];
		}
	}

	mt19937 generator = CreateRandomGenerator();
	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;
	m_strCurrentGameType = L"KTEM";
	m_nKtemStakeCents = ktemStakeCents;

	m_strSampleNums.Format(LocalizeText(
		L"KTEM CSV: %s | Prediction rows: %d | Data rows: %d | Number columns detected: %d",
		L"KTEM CSV: %s | Ennustusrivejä: %d | Data-rivejä: %d | Numero-sarakkeita havaittu: %d"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:", L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		const wstring outputText = output.str();

		m_vstrNums.Add(ToStableCString(outputText));
	}

	vector<ColumnStats> allStats;
	for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][columnIndex]);
			}
		}

		CString label;
		label.Format(IsFinnishLanguage() ? L"Nro%d" : L"Nr%d", columnIndex + 1);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(mainCount));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Mean", L"Keskiarvo"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Min", L"Minimi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Max", L"Maksimi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Median", L"Mediaani"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Mode", L"Moodi"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Harmonic", L"Harmoninen"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"Geometric", L"Geometrinen"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"RMS", L"RMS"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"StdDev", L"Keskihajonta"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"LowerQ", L"Alempi kvartiili"), values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(LocalizeText(L"UpperQ", L"Ylempi kvartiili"), values));

	ApplyStrategyPredictions(m_vstrNums, mainFrequencies, mainCount, dialog.m_nPredictionRows,
		dialog.m_bBalanced, dialog.m_bDelta, dialog.m_bWheel, dialog.m_strWheelNums,
		dialog.m_bUnusual, dialog.m_bConsecutive, dialog.m_bMagicSum, dialog.m_bMatrix, dialog.m_bCoverage,
		generator);
	AppendTicketCost(m_strSampleNums, m_vstrNums,
		ktemStakeCents * (dialog.m_bOnnenapila ? 2 : 1));

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnKeno()
{
	CKenoDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;
	const int kenoDefaultLevelIndex = LotteryPrices::kKenoMaximumLevel -
		LotteryPrices::kKenoMinimumLevel;
	const int kenoDefaultStakeIndex = (LotteryPrices::kKenoDefaultStakeCents -
		LotteryPrices::kKenoMinimumStakeCents) / LotteryPrices::kStakeStepCents;
	dialog.m_nLevelIndex = ReadPricePreference(_T("KenoLevelIndex"), kenoDefaultLevelIndex, 0,
		kenoDefaultLevelIndex);
	dialog.m_nStakeIndex = ReadPricePreference(_T("KenoStakeIndex"), kenoDefaultStakeIndex, 0,
		(LotteryPrices::kKenoMaximumStakeCents - LotteryPrices::kKenoMinimumStakeCents) /
		LotteryPrices::kStakeStepCents);
	dialog.m_bKunkkukerroin = ReadPricePreference(_T("KenoKunkkukerroin"), FALSE, FALSE, TRUE);

	if (dialog.DoModal() != IDOK)
	{
		return;
	}
	WritePricePreference(_T("KenoLevelIndex"), dialog.m_nLevelIndex);
	WritePricePreference(_T("KenoStakeIndex"), dialog.m_nStakeIndex);
	WritePricePreference(_T("KenoKunkkukerroin"), dialog.m_bKunkkukerroin);

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadKenoRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\KenoData.csv.",
			L"Tiedostoa data\\KenoData.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int statColumnCount = (dominantColumnCount >= 20) ? 20 : dominantColumnCount;
	const int predictionCount = LotteryPrices::kKenoMinimumLevel + dialog.m_nLevelIndex;
	const int kenoStakeCents = LotteryPrices::kKenoMinimumStakeCents +
		(dialog.m_nStakeIndex * LotteryPrices::kStakeStepCents);

	map<int, int> mainFrequencies;
	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < statColumnCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			const int value = row[columnIndex];
			if (value >= 1 && value <= 70)
			{
				++mainFrequencies[value];
			}
		}
	}

	if (mainFrequencies.empty())
	{
		AfxMessageBox(LocalizeText(
			L"No valid Keno numbers in the range 1 to 70 were found in data\\KenoData.csv.",
			L"Tiedostosta data\\KenoData.csv ei löytynyt kelvollisia Keno-numeroita väliltä 1–70."));
		return;
	}

	mt19937 generator = CreateRandomGenerator();
	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = predictionCount;
	m_strCurrentGameType = L"Keno";
	m_nKenoLevel = predictionCount;
	m_nKenoStakeCents = kenoStakeCents;
	m_bKenoKunkkukerroin = dialog.m_bKunkkukerroin;

	m_strSampleNums.Format(LocalizeText(
		L"Keno CSV: %s | Prediction rows: %d | Data rows: %d | Draw columns detected: %d | Level: %d",
		L"Keno CSV: %s | Ennusterivejä: %d | Arvontarivejä: %d | Arvontanumeroiden sarakkeita: %d | Taso: %d"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		statColumnCount,
		predictionCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:",
		L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, predictionCount, generator);
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		const wstring outputText = output.str();

		m_vstrNums.Add(ToStableCString(outputText));
	}

	vector<ColumnStats> allStats;
	for (int columnIndex = 0; columnIndex < statColumnCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				const int value = rows[rowIndex][columnIndex];
				if (value >= 1 && value <= 70)
				{
					columnValues.push_back(value);
				}
			}
		}

		CString label;
		label.Format(IsFinnishLanguage() ? L"Nro%d" : L"Nr%d", columnIndex + 1);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(statColumnCount));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(L"Mean", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Min", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Max", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(L"Median", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Mode", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(L"Harmonic", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(L"Geometric", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(L"RMS", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(L"StdDev", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(L"LowerQ", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(L"UpperQ", values));

	ApplyStrategyPredictions(m_vstrNums, mainFrequencies, predictionCount, dialog.m_nPredictionRows,
		dialog.m_bBalanced, dialog.m_bDelta, dialog.m_bWheel, dialog.m_strWheelNums,
		dialog.m_bUnusual, dialog.m_bConsecutive, dialog.m_bMagicSum, dialog.m_bMatrix, dialog.m_bCoverage,
		generator);
	AppendTicketCost(m_strSampleNums, m_vstrNums,
		kenoStakeCents * (dialog.m_bKunkkukerroin ? 2 : 1));

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnEurojackpot()
{
	CEurojackpotDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadEurojackpotRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\EurojackpotData.csv.",
			L"Tiedostoa data\\EurojackpotData.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 5) ? 5 : dominantColumnCount;
	const int starCount = (dominantColumnCount > mainCount) ? min(2, dominantColumnCount - mainCount) : 0;

	map<int, int> mainFrequencies;
	map<int, int> starFrequencies;
	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++mainFrequencies[row[columnIndex]];
		}

		for (int starIndex = 0; starIndex < starCount; ++starIndex)
		{
			const int column = mainCount + starIndex;
			if (column < static_cast<int>(row.size()))
			{
				++starFrequencies[row[column]];
			}
		}
	}

	mt19937 generator = CreateRandomGenerator();
	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;
	m_strCurrentGameType = L"Eurojackpot";

	m_strSampleNums.Format(LocalizeText(
		L"Eurojackpot CSV: %s | Prediction rows: %d | Data rows: %d | Main numbers: %d | Stars: %d",
		L"Eurojackpot CSV: %s | Ennusterivejä: %d | Arvontarivejä: %d | Päänumeroita: %d | Tähtinumeroita: %d"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount,
		starCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:",
		L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> mainPrediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		vector<int> starPrediction = BuildPredictionLine(starFrequencies, starCount, generator);
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < mainPrediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << mainPrediction[numberIndex];
		}

		if (!starPrediction.empty())
		{
			const CString starLabel = LocalizeText(L"Stars: ", L"Tähdet: ");
			output << L" | " << starLabel.GetString();
			for (size_t starIndex = 0; starIndex < starPrediction.size(); ++starIndex)
			{
				if (starIndex > 0)
				{
					output << L", ";
				}
				output << starPrediction[starIndex];
			}
		}

		const wstring outputText = output.str();

		m_vstrNums.Add(ToStableCString(outputText));
	}

	vector<ColumnStats> allStats;
	vector<CString> headers;
	for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][columnIndex]);
			}
		}

		CString label;
		label.Format(IsFinnishLanguage() ? L"Nro%d" : L"Nr%d", columnIndex + 1);
		headers.push_back(label);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	for (int starIndex = 0; starIndex < starCount; ++starIndex)
	{
		vector<int> columnValues;
		const int column = mainCount + starIndex;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (column < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][column]);
			}
		}

		CString label;
		label.Format(IsFinnishLanguage() ? L"Tähti%d" : L"Star%d", starIndex + 1);
		headers.push_back(label);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(headers));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(L"Mean", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Min", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Max", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(L"Median", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Mode", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(L"Harmonic", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(L"Geometric", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(L"RMS", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(L"StdDev", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(L"LowerQ", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(L"UpperQ", values));

	ApplyStrategyPredictions(m_vstrNums, mainFrequencies, mainCount, dialog.m_nPredictionRows,
		dialog.m_bBalanced, dialog.m_bDelta, dialog.m_bWheel, dialog.m_strWheelNums,
		dialog.m_bUnusual, dialog.m_bConsecutive, dialog.m_bMagicSum, dialog.m_bMatrix, dialog.m_bCoverage,
		generator, &starFrequencies, starCount);
	AppendTicketCost(m_strSampleNums, m_vstrNums, LotteryPrices::kEurojackpotBaseCents);

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnVikinglotto()
{
	CVikingLottoDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;
	dialog.m_bPlusNumber = ReadPricePreference(_T("VikingLottoPlusNumber"), FALSE, FALSE, TRUE);

	if (dialog.DoModal() != IDOK)
	{
		return;
	}
	WritePricePreference(_T("VikingLottoPlusNumber"), dialog.m_bPlusNumber);

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadVikingRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\VikingData.csv.",
			L"Tiedostoa data\\VikingData.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 6) ? 6 : dominantColumnCount;

	map<int, int> mainFrequencies;
	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++mainFrequencies[row[columnIndex]];
		}
	}

	mt19937 generator = CreateRandomGenerator();
	uniform_int_distribution<int> vikingDistribution(1, 5);

	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;
	m_strCurrentGameType = L"Viking Lotto";

	m_strSampleNums.Format(LocalizeText(
		L"Viking Lotto CSV: %s | Prediction rows: %d | Data rows: %d | Main numbers: %d | Viking number is randomly chosen from 1-5 because no historical Viking-number data exists",
		L"Viking Lotto CSV: %s | Ennusterivejä: %d | Arvontarivejä: %d | Päänumeroita: %d | Viking-numero arvotaan väliltä 1–5, koska historiatietoa ei ole"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:",
		L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		const int vikingNumber = vikingDistribution(generator);
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		const CString vikingNumberLabel = LocalizeText(L"Viking number: ", L"Viking-numero: ");
		const CString vikingSelectionNote = LocalizeText(
			L"randomly selected for the prediction", L"arvottu ennusteeseen");
		output << L" | " << vikingNumberLabel.GetString()
			<< vikingNumber << L" (" << vikingSelectionNote.GetString() << L")";
		const wstring outputText = output.str();
		m_vstrNums.Add(ToStableCString(outputText));
	}

	vector<ColumnStats> allStats;
	for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][columnIndex]);
			}
		}

		allStats.push_back(ComputeColumnStats(columnValues, GetColumnLabel(columnIndex, mainCount, mainCount)));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(mainCount));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(L"Mean", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Min", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Max", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(L"Median", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Mode", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(L"Harmonic", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(L"Geometric", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(L"RMS", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(L"StdDev", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(L"LowerQ", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(L"UpperQ", values));

	ApplyStrategyPredictions(m_vstrNums, mainFrequencies, mainCount, dialog.m_nPredictionRows,
		dialog.m_bBalanced, dialog.m_bDelta, dialog.m_bWheel, dialog.m_strWheelNums,
		dialog.m_bUnusual, dialog.m_bConsecutive, dialog.m_bMagicSum, dialog.m_bMatrix, dialog.m_bCoverage,
		generator, NULL, 0, true);
	AppendTicketCost(m_strSampleNums, m_vstrNums,
		LotteryPrices::kVikingLottoBaseCents +
		(dialog.m_bPlusNumber ? LotteryPrices::kVikingLottoPlusCents : 0));

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnJokeri()
{
	CJokeriDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;
	dialog.m_bDouble = ReadPricePreference(_T("JokeriDouble"), FALSE, FALSE, TRUE);

	if (dialog.DoModal() != IDOK)
	{
		return;
	}
	WritePricePreference(_T("JokeriDouble"), dialog.m_bDouble);

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadJokeriRows(rows, usedPath))
	{
		AfxMessageBox(LocalizeText(L"Could not open data\\JokeriData.csv.",
			L"Tiedostoa data\\JokeriData.csv ei voitu avata."));
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 7) ? 7 : dominantColumnCount;
	vector< map<int, int> > digitFrequencies(mainCount);
	map<int, int> repeatProfileFrequencies;

	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		bool completeValidRow = static_cast<int>(row.size()) >= mainCount;
		bool seenDigits[10] = {};
		int uniqueDigitCount = 0;
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			const int digit = row[columnIndex];
			if (digit < 0 || digit > 9)
			{
				completeValidRow = false;
				continue;
			}

			++digitFrequencies[columnIndex][digit];
			if (!seenDigits[digit])
			{
				seenDigits[digit] = true;
				++uniqueDigitCount;
			}
		}

		if (completeValidRow)
		{
			++repeatProfileFrequencies[uniqueDigitCount];
		}
	}

	mt19937 generator = CreateRandomGenerator();
	m_vstrNums.RemoveAll();
	ClearResultCheck();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;
	m_strCurrentGameType = L"Jokeri";
	m_bJokeriDouble = dialog.m_bDouble;

	m_strSampleNums.Format(LocalizeText(
		L"Jokeri CSV: %s | Prediction rows: %d | Data rows: %d | Digit columns: %d | Repeated digits are allowed and harmonic mean is shown as N/A when a column contains zero values",
		L"Jokeri CSV: %s | Ennusterivejä: %d | Arvontarivejä: %d | Numerosarakkeita: %d | Numerot voivat toistua; harmoninen keskiarvo on Ei tietoa, jos sarakkeessa on nollia"),
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(LocalizeText(L"Predictions based on historical frequency:",
		L"Ennusteet historiatiedon perusteella:"));
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		wostringstream output;
		output << GetPredictionPrefix() << (predictionIndex + 1) << L": ";
		for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
		{
			if (columnIndex > 0)
			{
				output << L" ";
			}
			output << ChooseWeightedNumber(digitFrequencies[columnIndex], generator);
		}
		const wstring outputText = output.str();
		m_vstrNums.Add(ToStableCString(outputText));
	}

	if (dialog.m_bRepeatProfile == TRUE)
	{
		m_vstrNums.Add(L" ");
		m_vstrNums.Add(LocalizeText(
			L"Predictions matching historical repeated-digit profiles:",
			L"Historiallisia numerotoistoprofiileja vastaavat ennusteet:"));
		if (repeatProfileFrequencies.empty())
		{
			m_vstrNums.Add(LocalizeText(
				L"Repeat-profile predictions unavailable: no complete valid historical rows.",
				L"Toistoprofiiliennusteita ei voida luoda: historiatiedoissa ei ole yhtään kokonaista kelvollista riviä."));
		}
		else
		{
			for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
			{
				vector<int> prediction = BuildJokeriRepeatProfilePrediction(
					digitFrequencies, repeatProfileFrequencies, generator);
				if (prediction.empty())
				{
					m_vstrNums.Add(LocalizeText(
						L"Repeat-profile prediction unavailable for this row.",
						L"Tälle riville ei voida luoda toistoprofiiliennustetta."));
					continue;
				}

				bool seenDigits[10] = {};
				int uniqueDigitCount = 0;
				wostringstream output;
				const CString predictionLabel = LocalizeText(L"Profile prediction ", L"Profiiliennuste ");
				output << predictionLabel.GetString() << (predictionIndex + 1) << L": ";
				for (size_t digitIndex = 0; digitIndex < prediction.size(); ++digitIndex)
				{
					if (digitIndex > 0)
					{
						output << L" ";
					}
					output << prediction[digitIndex];
					if (!seenDigits[prediction[digitIndex]])
					{
						seenDigits[prediction[digitIndex]] = true;
						++uniqueDigitCount;
					}
				}
				const CString distinctDigitLabel = LocalizeText(L"distinct digits: ", L"eri numeroita: ");
				output << L" (" << distinctDigitLabel.GetString() << uniqueDigitCount << L")";
				const wstring outputText = output.str();
				m_vstrNums.Add(ToStableCString(outputText));
			}
		}
	}

	vector<ColumnStats> allStats;
	for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
	{
		vector<int> columnValues;
		for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
		{
			if (columnIndex < static_cast<int>(rows[rowIndex].size()))
			{
				columnValues.push_back(rows[rowIndex][columnIndex]);
			}
		}

		allStats.push_back(ComputeColumnStats(columnValues, GetColumnLabel(columnIndex, mainCount, mainCount)));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(LocalizeText(L"Statistical analysis:", L"Tilastollinen analyysi:"));
	m_vstrNums.Add(BuildTableHeaderRow(mainCount));

	vector<CString> values;
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].mean)); } m_vstrNums.Add(BuildTableRow(L"Mean", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].minValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Min", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].maxValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Max", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].median)); } m_vstrNums.Add(BuildTableRow(L"Median", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { CString s; s.Format(L"%d", allStats[i].modeValue); values.push_back(s); } m_vstrNums.Add(BuildTableRow(L"Mode", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].harmonicMean)); } m_vstrNums.Add(BuildTableRow(L"Harmonic", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].geometricMean)); } m_vstrNums.Add(BuildTableRow(L"Geometric", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].rootMeanSquare)); } m_vstrNums.Add(BuildTableRow(L"RMS", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].standardDeviation)); } m_vstrNums.Add(BuildTableRow(L"StdDev", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].lowerQuartile)); } m_vstrNums.Add(BuildTableRow(L"LowerQ", values));
	values.clear(); for (size_t i = 0; i < allStats.size(); ++i) { values.push_back(FormatDouble(allStats[i].upperQuartile)); } m_vstrNums.Add(BuildTableRow(L"UpperQ", values));

	AppendTicketCost(m_strSampleNums, m_vstrNums,
		LotteryPrices::kJokeriBaseCents +
		(dialog.m_bDouble ? LotteryPrices::kJokeriDoubleCents : 0));
	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void extractIntegerWords(CString str, vector <int> &vec)
{
	stringstream ss;
	// Storing the whow string into string stream
	CT2CA pszConvertedAnsiString (str);
	string strStd (pszConvertedAnsiString); 
	ss << strStd;

	// running loop till the end of the strawm
	string temp;
	int found;
	while (!ss.eof() )
	{
		// extracting word by word from stream
		ss >> temp;
		// checking if the given word is integer or not
		if (stringstream(temp) >> found)
			vec.push_back(found);
	}

}

void CLottoGuiDoc::DeleteContents()
{
	m_nLines = 0;
	m_strSampleNums = L"";
	m_strAllotedNums = L"";
	m_vstrNums.RemoveAll();
	ClearResultCheck();

	CDocument::DeleteContents();
}
