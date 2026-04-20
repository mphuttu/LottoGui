
// LottoGuiDoc.cpp : implementation of the CLottoGuiDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LottoGui.h"
#endif

#include "LottoGuiDoc.h"
#include "LottoDialog.h"
#include "SuomenLottoDlg.h"
#include "MilliDlg.h"
#include "KtemDlg.h"
#include "EurojackpotDlg.h"
#include "VikingLottoDlg.h"
#include "JokeriDlg.h"

#include <propkey.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
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
			return L"N/A";
		}

		CString text;
		text.Format(L"%.2f", value);

		while (text.GetLength() > 0 && text.Right(1) == L"0")
		{
			text = text.Left(text.GetLength() - 1);
		}

		while (text.GetLength() > 0 && text.Right(1) == L".")
		{
			text = text.Left(text.GetLength() - 1);
		}

		return text;
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
		return LoadCsvRows(L"MilliData.csv", rows, usedPath);
	}

	bool LoadKtemRows(vector<vector<int> >& rows, CString& usedPath)
	{
		return LoadCsvRows(L"KTEM.csv", rows, usedPath);
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
			label.Format(L"Nr%d", columnIndex + 1);
			return label;
		}

		const int extraIndex = columnIndex - mainCount;
		const int extraCount = dominantColumnCount - mainCount;

		if (extraCount <= 1)
		{
			return L"Extra";
		}

		if (extraIndex == extraCount - 1)
		{
			return L"Plus";
		}

		if (extraIndex == 0)
		{
			return L"Extra";
		}

		CString label;
		label.Format(L"Extra %d", extraIndex + 1);
		return label;
	}

	CString BuildTableHeaderRow(int columnCount)
	{
		wostringstream output;
		output << left << setw(16) << L"";
		for (int index = 0; index < columnCount; ++index)
		{
			CString label;
			label.Format(L"Nr%d", index + 1);
			output << right << setw(10) << static_cast<LPCWSTR>(label);
		}
		return output.str().c_str();
	}

	CString BuildTableHeaderRow(const vector<CString>& headers)
	{
		wostringstream output;
		output << left << setw(16) << L"";
		for (size_t index = 0; index < headers.size(); ++index)
		{
			output << right << setw(10) << static_cast<LPCWSTR>(headers[index]);
		}
		return output.str().c_str();
	}

	CString BuildTableRow(const CString& metricName, const vector<CString>& values)
	{
		wostringstream output;
		output << left << setw(16) << static_cast<LPCWSTR>(metricName);
		for (size_t index = 0; index < values.size(); ++index)
		{
			output << right << setw(10) << static_cast<LPCWSTR>(values[index]);
		}
		return output.str().c_str();
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
}

// CLottoGuiDoc

IMPLEMENT_DYNCREATE(CLottoGuiDoc, CDocument)

BEGIN_MESSAGE_MAP(CLottoGuiDoc, CDocument)
	ON_COMMAND(ID_LOTTOOPTIONS, &CLottoGuiDoc::OnLottooptions)
	ON_COMMAND(ID_LOTTOOPTIONS_SUOMENLOTTO, &CLottoGuiDoc::OnSuomenlotto)
	ON_COMMAND(ID_LOTTOOPTIONS_MILLICSV, &CLottoGuiDoc::OnMilli)
	ON_COMMAND(ID_LOTTOOPTIONS_KTEMCSV, &CLottoGuiDoc::OnKtem)
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

	vector<int> input;
	if (aDlg.m_bGivenNumber == TRUE)
	{
		extractIntegerWords(m_strNums, input);
		m_strSampleNums = L"Sample numbers: ";
		m_strSampleNums += aDlg.m_strNums;
	}
	else
	{
		m_strSampleNums = L"Range: ";
		for (int value = m_nMinNum; value <= m_nMaxNum; ++value)
		{
			input.push_back(value);
			CString numberText;
			numberText.Format(L"%d ", value);
			m_strSampleNums += numberText;
		}
	}

	const int n = static_cast<int>(input.size());
	if (n <= 0)
	{
		AfxMessageBox(L"Please provide at least one valid number.");
		return;
	}

	if (m_nAmount > n)
	{
		m_nAmount = n;
	}

	vector<int> lottoline;
	srand(static_cast<unsigned int>(time(0)));

	for (int roundIndex = 0; roundIndex < m_nRounds; ++roundIndex)
	{
		CString lineText;
		CString roundNumber;
		roundNumber.Format(L"%d", roundIndex + 1);
		lineText = _T("Line ") + roundNumber + _T(": ");

		while (static_cast<int>(lottoline.size()) < m_nAmount)
		{
			const int numberIndex = rand() % n;
			if (find(lottoline.begin(), lottoline.end(), input[numberIndex]) == lottoline.end())
			{
				lottoline.push_back(input[numberIndex]);
			}
		}

		sort(lottoline.begin(), lottoline.end());
		ostringstream stream;
		if (!lottoline.empty())
		{
			copy(lottoline.begin(), lottoline.end() - 1, ostream_iterator<int>(stream, ", "));
			stream << lottoline.back();
		}

		lineText += stream.str().c_str();
		m_vstrNums.Add(lineText);
		lottoline.clear();
	}

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 4;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnSuomenlotto()
{
	CSuomenLottoDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadSuomenLottoRows(rows, usedPath))
	{
		AfxMessageBox(L"Could not open data\\SuomenLottoData.csv.");
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

	mt19937 generator(static_cast<unsigned int>(time(NULL)));
	m_vstrNums.RemoveAll();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;

	m_strSampleNums.Format(L"Suomen Lotto CSV: %s | Prediction rows: %d | Data rows: %d | Main-number columns detected: %d",
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(L"Predictions based on historical frequency:");
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		wostringstream output;
		output << L"Prediction " << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		m_vstrNums.Add(output.str().c_str());
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

		allStats.push_back(ComputeColumnStats(columnValues, GetColumnLabel(columnIndex, mainCount, dominantColumnCount)));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(L"Statistical analysis:");
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
		AfxMessageBox(L"Could not open data\\MilliData.csv.");
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

	mt19937 generator(static_cast<unsigned int>(time(NULL)));
	m_vstrNums.RemoveAll();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;

	m_strSampleNums.Format(L"Milli CSV: %s | Prediction rows: %d | Data rows: %d | Number columns detected: %d",
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(L"Predictions based on historical frequency:");
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		wostringstream output;
		output << L"Prediction " << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		m_vstrNums.Add(output.str().c_str());
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
		label.Format(L"Nr%d", columnIndex + 1);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(L"Statistical analysis:");
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

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnKtem()
{
	CKtemDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadKtemRows(rows, usedPath))
	{
		AfxMessageBox(L"Could not open data\\KTEM.csv.");
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 12) ? 12 : dominantColumnCount;

	map<int, int> mainFrequencies;
	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++mainFrequencies[row[columnIndex]];
		}
	}

	mt19937 generator(static_cast<unsigned int>(time(NULL)));
	m_vstrNums.RemoveAll();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;

	m_strSampleNums.Format(L"KTEM CSV: %s | Prediction rows: %d | Data rows: %d | Number columns detected: %d",
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(L"Predictions based on historical frequency:");
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		wostringstream output;
		output << L"Prediction " << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		m_vstrNums.Add(output.str().c_str());
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
		label.Format(L"Nr%d", columnIndex + 1);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(L"Statistical analysis:");
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
		AfxMessageBox(L"Could not open data\\EurojackpotData.csv.");
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

	mt19937 generator(static_cast<unsigned int>(time(NULL)));
	m_vstrNums.RemoveAll();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;

	m_strSampleNums.Format(L"Eurojackpot CSV: %s | Prediction rows: %d | Data rows: %d | Main numbers: %d | Stars: %d",
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount,
		starCount);

	m_vstrNums.Add(L"Predictions based on historical frequency:");
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> mainPrediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		vector<int> starPrediction = BuildPredictionLine(starFrequencies, starCount, generator);
		wostringstream output;
		output << L"Prediction " << (predictionIndex + 1) << L": ";

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
			output << L" | Stars: ";
			for (size_t starIndex = 0; starIndex < starPrediction.size(); ++starIndex)
			{
				if (starIndex > 0)
				{
					output << L", ";
				}
				output << starPrediction[starIndex];
			}
		}

		m_vstrNums.Add(output.str().c_str());
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
		label.Format(L"Nr%d", columnIndex + 1);
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
		label.Format(L"Star%d", starIndex + 1);
		headers.push_back(label);
		allStats.push_back(ComputeColumnStats(columnValues, label));
	}

	m_vstrNums.Add(L" ");
	m_vstrNums.Add(L"Statistical analysis:");
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

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnVikinglotto()
{
	CVikingLottoDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadVikingRows(rows, usedPath))
	{
		AfxMessageBox(L"Could not open data\\VikingData.csv.");
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

	mt19937 generator(static_cast<unsigned int>(time(NULL)));
	uniform_int_distribution<int> vikingDistribution(1, 5);

	m_vstrNums.RemoveAll();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;

	m_strSampleNums.Format(L"Viking Lotto CSV: %s | Prediction rows: %d | Data rows: %d | Main numbers: %d | Viking number is randomly chosen from 1-5 because no historical Viking-number data exists",
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(L"Predictions based on historical frequency:");
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		vector<int> prediction = BuildPredictionLine(mainFrequencies, mainCount, generator);
		const int vikingNumber = vikingDistribution(generator);
		wostringstream output;
		output << L"Prediction " << (predictionIndex + 1) << L": ";

		for (size_t numberIndex = 0; numberIndex < prediction.size(); ++numberIndex)
		{
			if (numberIndex > 0)
			{
				output << L", ";
			}
			output << prediction[numberIndex];
		}

		output << L" | Viking number: " << vikingNumber << L" (randomly selected for the prediction)";
		m_vstrNums.Add(output.str().c_str());
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
	m_vstrNums.Add(L"Statistical analysis:");
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

	m_nLines = static_cast<int>(m_vstrNums.GetSize()) + 6;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CLottoGuiDoc::OnJokeri()
{
	CJokeriDlg dialog;
	dialog.m_nPredictionRows = (m_nRounds > 0) ? m_nRounds : 5;

	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	vector<vector<int> > rows;
	CString usedPath;
	if (!LoadJokeriRows(rows, usedPath))
	{
		AfxMessageBox(L"Could not open data\\JokeriData.csv.");
		return;
	}

	const int dominantColumnCount = DetectDominantColumnCount(rows);
	const int mainCount = (dominantColumnCount >= 7) ? 7 : dominantColumnCount;
	vector< map<int, int> > digitFrequencies(mainCount);

	for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
	{
		const vector<int>& row = rows[rowIndex];
		for (int columnIndex = 0; columnIndex < mainCount && columnIndex < static_cast<int>(row.size()); ++columnIndex)
		{
			++digitFrequencies[columnIndex][row[columnIndex]];
		}
	}

	mt19937 generator(static_cast<unsigned int>(time(NULL)));
	m_vstrNums.RemoveAll();
	m_nRounds = dialog.m_nPredictionRows;
	m_nAmount = mainCount;

	m_strSampleNums.Format(L"Jokeri CSV: %s | Prediction rows: %d | Data rows: %d | Digit columns: %d | Repeated digits are allowed and harmonic mean is shown as N/A when a column contains zero values",
		usedPath.GetString(),
		dialog.m_nPredictionRows,
		static_cast<int>(rows.size()),
		mainCount);

	m_vstrNums.Add(L"Predictions based on historical frequency:");
	for (int predictionIndex = 0; predictionIndex < dialog.m_nPredictionRows; ++predictionIndex)
	{
		wostringstream output;
		output << L"Prediction " << (predictionIndex + 1) << L": ";
		for (int columnIndex = 0; columnIndex < mainCount; ++columnIndex)
		{
			if (columnIndex > 0)
			{
				output << L" ";
			}
			output << ChooseWeightedNumber(digitFrequencies[columnIndex], generator);
		}
		m_vstrNums.Add(output.str().c_str());
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
	m_vstrNums.Add(L"Statistical analysis:");
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

	CDocument::DeleteContents();
}
