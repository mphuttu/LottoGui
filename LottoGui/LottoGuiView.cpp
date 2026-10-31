
// LottoGuiView.cpp : implementation of the CLottoGuiView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LottoGui.h"
#endif

#include "LottoGuiDoc.h"
#include "LottoGuiCore.h"
#include "LottoGuiView.h"

#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	const int kA4PortraitWidth = 827;
	const int kA4PortraitHeight = 1169;
	const int kReportMargin = 48;

	enum ReportBlockType
	{
		ReportSpacer,
		ReportSection,
		ReportPrediction,
		ReportStatisticsHeader,
		ReportStatisticsRow,
		ReportParagraph
	};

	struct ReportBlock
	{
		ReportBlockType type;
		CString text;
		CString label;
		CString detail;
		CString result;
		std::vector<CString> cells;
		int textHeight;
		int labelHeight;
		int detailHeight;
		int resultHeight;
		int height;
		int gapAfter;
		int top;
		int bottom;
		int labelY;
		int textY;
		int detailY;
		int resultY;
		int ruleY;
		int labelOffset;
		int textOffset;
		int detailOffset;
		int resultOffset;
		bool repeated;
		bool alternate;
		bool isWin;
	};

	struct ReportPage
	{
		std::vector<ReportBlock> blocks;
		bool drawHeader;
		int footerY;
		int footerRuleY;
	};

	struct ReportLayout
	{
		int pageLeft;
		int pageWidth;
		int pageHeight;
		int contentLeft;
		int contentWidth;
		int metadataTop;
		int metadataTextTop;
		int metadataCardBottom;
		int metadataHeight;
		int metadataPadding;
		int headerBandHeight;
		int headerHeight;
		int bodyTop;
		int headerKickerTop;
		int headerTitleTop;
		int headerGameTop;
		int headerRuleY;
		int sectionRuleOffset;
		int statisticsLabelWidth;
		int statisticsColumnWidth;
		int statisticsColumnCount;
		int pageCount;
		int footerReserve;
		std::vector<ReportPage> pages;
	};

	struct ReportMetrics
	{
		int titleLine;
		int sectionLine;
		int subsectionLine;
		int bodyLine;
		int predictionLine;
		int detailLine;
		int resultLine;
		int statisticsLine;
		int smallGap;
		int blockGap;
		int sectionGap;
		int cardPadding;
		int footerReserve;
	};

	struct ReportFonts
	{
		CFont title;
		CFont section;
		CFont subsection;
		CFont body;
		CFont prediction;
		CFont detail;
		CFont result;
		CFont statistics;
		CFont statisticsHeader;
	};

	bool StartsWith(const CString& text, LPCTSTR prefix)
	{
		const int prefixLength = static_cast<int>(_tcslen(prefix));
		return text.GetLength() >= prefixLength && text.Left(prefixLength) == prefix;
	}

	bool IsPredictionLine(const CString& text)
	{
		return StartsWith(text, L"Prediction ") ||
			StartsWith(text, L"Ennuste ") ||
			StartsWith(text, L"Profile prediction ") ||
			StartsWith(text, L"Profiiliennuste ") ||
			StartsWith(text, L"Combination ") ||
			StartsWith(text, L"Yhdistelmä ") ||
			StartsWith(text, L"Line ") ||
			StartsWith(text, L"Rivi ");
	}

	bool IsPredictionSection(const CString& text)
	{
		return StartsWith(text, L"Predictions based on historical") ||
			StartsWith(text, L"Ennusteet historiatiedon perusteella");
	}

	bool IsStatisticsHeading(const CString& text)
	{
		return StartsWith(text, L"Statistical analysis") ||
			StartsWith(text, L"Tilastollinen analyysi");
	}

	bool IsPredictionSummaryHeading(const CString& text)
	{
		return StartsWith(text, L"Prediction Summary") ||
			StartsWith(text, L"Ennusteyhteenveto");
	}

	bool IsSectionHeading(const CString& text)
	{
		return !text.IsEmpty() && text.Right(1) == L":";
	}

	void CreateReportFont(CFont& font, int pointSize, LPCTSTR faceName, bool bold, CDC* pDC)
	{
		font.CreatePointFont(pointSize * 10, faceName, pDC);
		if (bold)
		{
			LOGFONT logFont;
			font.GetLogFont(&logFont);
			font.DeleteObject();
			logFont.lfWeight = FW_BOLD;
			font.CreateFontIndirect(&logFont);
		}
	}

	int GetFontLineHeight(CDC* pDC, CFont* font)
	{
		CFont* previousFont = pDC->SelectObject(font);
		TEXTMETRIC metrics = {};
		const BOOL measured = pDC->GetTextMetrics(&metrics);
		pDC->SelectObject(previousFont);
		if (!measured)
		{
			return 1;
		}

		return max(1, metrics.tmHeight + metrics.tmExternalLeading);
	}

	ReportMetrics MeasureReportMetrics(
		CDC* pDC,
		CFont* titleFont,
		CFont* sectionFont,
		CFont* subsectionFont,
		CFont* bodyFont,
		CFont* predictionFont,
		CFont* detailFont,
		CFont* resultFont,
		CFont* statisticsFont)
	{
		ReportMetrics metrics = {};
		metrics.titleLine = GetFontLineHeight(pDC, titleFont);
		metrics.sectionLine = GetFontLineHeight(pDC, sectionFont);
		metrics.subsectionLine = GetFontLineHeight(pDC, subsectionFont);
		metrics.bodyLine = GetFontLineHeight(pDC, bodyFont);
		metrics.predictionLine = GetFontLineHeight(pDC, predictionFont);
		metrics.detailLine = GetFontLineHeight(pDC, detailFont);
		metrics.resultLine = GetFontLineHeight(pDC, resultFont);
		metrics.statisticsLine = GetFontLineHeight(pDC, statisticsFont);
		metrics.smallGap = max(2, metrics.bodyLine / 3);
		metrics.blockGap = max(metrics.smallGap, metrics.bodyLine / 2);
		metrics.sectionGap = max(metrics.blockGap, metrics.sectionLine / 2);
		metrics.cardPadding = max(metrics.smallGap, metrics.bodyLine / 2);
		metrics.footerReserve = metrics.detailLine + metrics.blockGap * 2;
		return metrics;
	}

	CString LocalizedPredictionHeading(bool isFinnish)
	{
			return isFinnish
				? L"Ennusteyhteenveto"
				: L"Prediction Summary";
	}

	CString LocalizedStatisticsHeading(bool isFinnish)
	{
			return isFinnish ? L"Tilastollinen analyysi" : L"Statistical Analysis";
	}

	std::vector<CString> SplitStatisticsCells(const CString& text)
	{
			std::vector<CString> cells;
			CString cell;
			int index = 0;
			while (index < text.GetLength())
			{
				if (text[index] == L' ')
				{
					int runLength = 1;
					while (index + runLength < text.GetLength() &&
						text[index + runLength] == L' ')
					{
						++runLength;
					}

					if (runLength >= 2)
					{
						cell.Trim();
						if (!cell.IsEmpty())
						{
							cells.push_back(cell);
						}
						cell.Empty();
						index += runLength;
						continue;
					}
				}

				cell += text[index];
				++index;
			}

			cell.Trim();
			if (!cell.IsEmpty())
			{
				cells.push_back(cell);
			}
			return cells;
	}

	int CountStatisticsColumns(CLottoGuiDoc* pDoc)
	{
			bool inStatistics = false;
			for (INT_PTR index = 0; index < pDoc->m_vstrNums.GetSize(); ++index)
			{
				CString text = pDoc->m_vstrNums.GetAt(index);
				text.Trim();
				if (IsStatisticsHeading(text))
				{
					inStatistics = true;
					continue;
				}
				if (text.IsEmpty())
				{
					inStatistics = false;
					continue;
				}
				if (inStatistics)
				{
					return static_cast<int>(SplitStatisticsCells(text).size());
				}
			}
			return 0;
	}

	void CreateReportFonts(ReportFonts& fonts, CDC* pDC, int statisticsColumnCount)
	{
			CreateReportFont(fonts.title, 21, L"Segoe UI", true, pDC);
			CreateReportFont(fonts.section, 13, L"Segoe UI", true, pDC);
			CreateReportFont(fonts.subsection, 10, L"Segoe UI", true, pDC);
			CreateReportFont(fonts.body, 9, L"Segoe UI", false, pDC);
			CreateReportFont(fonts.prediction, 11, L"Segoe UI", true, pDC);
			CreateReportFont(fonts.detail, 8, L"Segoe UI", false, pDC);
			CreateReportFont(fonts.result, 9, L"Segoe UI", true, pDC);

			const int statisticsPointSize = statisticsColumnCount >= 18
				? 6 : (statisticsColumnCount >= 10 ? 7 : 8);
			CreateReportFont(fonts.statistics, statisticsPointSize,
				L"Segoe UI", false, pDC);
			CreateReportFont(fonts.statisticsHeader, statisticsPointSize,
				L"Segoe UI", true, pDC);
	}

	int MeasureWrappedText(CDC* pDC, const CString& text, int width, int minimumHeight)
	{
			if (text.IsEmpty())
			{
			return 0;
		}

		CRect rect(0, 0, width, 10000);
		pDC->DrawText(text, &rect, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
		const int measuredHeight = (rect.Height() < 0) ? -rect.Height() : rect.Height();
		return max(minimumHeight, measuredHeight);
	}

	void DrawWrappedText(CDC* pDC, const CString& text, int left, int top, int width, int height)
	{
		CRect rect(left, -top, left + width, -top - height);
		pDC->DrawText(text, &rect, DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
	}

	void DrawSingleLineCell(
		CDC* pDC,
		const CString& text,
		int left,
		int top,
		int width,
		int height,
		UINT alignment)
	{
		CRect rect(left, -top, left + width, -top - height);
		pDC->DrawText(text, &rect,
			DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | alignment);
	}

	void DrawCenteredText(CDC* pDC, const CString& text, int left, int top, int width)
	{
		const CSize textSize = pDC->GetTextExtent(text);
		pDC->TextOutW(left + max(0, (width - textSize.cx) / 2), -top, text);
	}

	void BuildReportBlocks(
		CLottoGuiDoc* pDoc,
		CDC* pDC,
		CFont* bodyFont,
		CFont* predictionFont,
		CFont* subsectionFont,
		CFont* detailFont,
		CFont* resultFont,
		CFont* sectionFont,
		CFont* statisticsFont,
		CFont* statisticsHeaderFont,
		const ReportMetrics& metrics,
		int textWidth,
		bool isFinnish,
		std::vector<ReportBlock>& blocks,
		int& statisticsColumnCount)
	{
		bool inStatistics = false;
		bool statisticsHeaderPending = false;
		statisticsColumnCount = 0;

		for (INT_PTR index = 0; index < pDoc->m_vstrNums.GetSize(); ++index)
		{
			ReportBlock block = {};
			block.text = pDoc->m_vstrNums.GetAt(index);
			block.text.Trim();

			if (block.text.IsEmpty())
			{
				block.type = ReportSpacer;
				block.height = metrics.smallGap;
				block.gapAfter = 0;
				inStatistics = false;
				statisticsHeaderPending = false;
			}
			else if (IsSectionHeading(block.text))
			{
				block.type = ReportSection;
				block.gapAfter = 9;
				if (IsPredictionSection(block.text))
				{
					block.text = LocalizedPredictionHeading(isFinnish);
				}
				else if (IsStatisticsHeading(block.text))
				{
					block.text = LocalizedStatisticsHeading(isFinnish);
					inStatistics = true;
					statisticsHeaderPending = true;
				}
				else
				{
					inStatistics = false;
					statisticsHeaderPending = false;
				}
				pDC->SelectObject(sectionFont);
				block.textHeight = MeasureWrappedText(
					pDC, block.text, textWidth, metrics.sectionLine);
				block.textOffset = metrics.smallGap;
				block.height = block.textHeight + metrics.smallGap * 2;
				block.gapAfter = metrics.sectionGap;
			}
			else if (inStatistics)
			{
				block.cells = SplitStatisticsCells(block.text);
				block.type = statisticsHeaderPending
					? ReportStatisticsHeader : ReportStatisticsRow;
				statisticsHeaderPending = false;
				if (block.type == ReportStatisticsHeader)
				{
					statisticsColumnCount = static_cast<int>(block.cells.size());
					pDC->SelectObject(statisticsHeaderFont);
				}
				else
				{
					pDC->SelectObject(statisticsFont);
				}
				block.height = metrics.statisticsLine + metrics.smallGap * 2;
				block.gapAfter = 0;
			}
			else
			{
				const bool hasResult =
					pDoc->m_vResultCounts.size() > static_cast<size_t>(index) &&
					pDoc->m_vResultCounts[index] >= 0;
				const bool isPrediction = IsPredictionLine(block.text);
				block.type = (hasResult || isPrediction)
					? ReportPrediction : ReportParagraph;
				block.gapAfter = (block.type == ReportPrediction)
					? metrics.blockGap : metrics.smallGap;

				if (block.type == ReportPrediction)
				{
					const int separator = block.text.Find(L':');
					if (separator >= 0)
					{
						block.label = block.text.Left(separator);
						block.label.Trim();
						block.text = block.text.Mid(separator + 1);
						block.text.Trim();
					}
					block.labelOffset = metrics.cardPadding;
					pDC->SelectObject(subsectionFont);
					block.labelHeight = MeasureWrappedText(
						pDC, block.label, textWidth,
						block.label.IsEmpty() ? 0 : metrics.subsectionLine);
					block.textOffset = block.labelOffset + block.labelHeight;
					if (block.labelHeight > 0)
					{
						block.textOffset += metrics.smallGap;
					}
					pDC->SelectObject(predictionFont);
					block.textHeight = MeasureWrappedText(
						pDC, block.text, textWidth, metrics.predictionLine);
				}
				else
				{
					pDC->SelectObject(bodyFont);
					block.textOffset = metrics.smallGap;
					block.textHeight = MeasureWrappedText(
						pDC, block.text, textWidth, metrics.bodyLine);
				}

				if (hasResult)
				{
					CString matches;
					matches.Format(isFinnish ? L"%d oikein" : L"%d correct",
						pDoc->m_vResultCounts[index]);

					if (pDoc->m_strCurrentGameType.Find(L"Eurojackpot") >= 0 &&
						pDoc->m_vMatchedNumbers.size() > static_cast<size_t>(index) &&
						pDoc->m_vBonusMatchedNumbers.size() > static_cast<size_t>(index))
					{
						matches = LottoGuiCore::FormatEurojackpotMatches(
							pDoc->m_vMatchedNumbers[index],
							pDoc->m_vBonusMatchedNumbers[index], isFinnish).c_str();
					}
					else if (pDoc->m_vMatchedNumbers.size() > static_cast<size_t>(index) &&
						pDoc->m_vBonusMatchedNumbers.size() > static_cast<size_t>(index))
					{
						std::vector<int> allMatches = pDoc->m_vMatchedNumbers[index];
						allMatches.insert(allMatches.end(), pDoc->m_vBonusMatchedNumbers[index].begin(),
							pDoc->m_vBonusMatchedNumbers[index].end());
						CString matchedValues;
						for (size_t matchIndex = 0;
							matchIndex < allMatches.size(); ++matchIndex)
						{
							if (matchIndex > 0)
							{
								matchedValues += L", ";
							}
							CString valueText;
							valueText.Format(L"%d", allMatches[matchIndex]);
							matchedValues += valueText;
						}
						if (!allMatches.empty())
						{
							matches += isFinnish
								? L" (osumat: " + matchedValues + L")"
								: L" (matches: " + matchedValues + L")";
						}
					}
					block.detail = matches;
					pDC->SelectObject(detailFont);
					block.detailHeight = MeasureWrappedText(
						pDC, block.detail, textWidth, metrics.detailLine);

					if (pDoc->m_vPrizeTexts.size() > static_cast<size_t>(index))
					{
						block.result = pDoc->m_vPrizeTexts[index];
					}
					block.isWin = !block.result.IsEmpty() &&
						block.result.Find(L"Ei voittoa") < 0 &&
						block.result.Find(L"No win") < 0;
					if (!block.result.IsEmpty())
					{
						pDC->SelectObject(resultFont);
						block.resultHeight = MeasureWrappedText(
							pDC, block.result, textWidth, metrics.resultLine);
					}
				}

				if (block.type == ReportPrediction)
				{
					block.detailOffset = block.textOffset + block.textHeight + metrics.smallGap;
					block.resultOffset = block.detailOffset + block.detailHeight + metrics.smallGap;
					block.height = block.textOffset + block.textHeight;
					if (block.detailHeight > 0)
					{
						block.height = block.detailOffset + block.detailHeight;
					}
					if (block.resultHeight > 0)
					{
						block.height = block.resultOffset + block.resultHeight;
					}
					block.height += metrics.cardPadding;
				}
				else
				{
					block.height = block.textOffset + block.textHeight + metrics.smallGap;
				}
			}

			blocks.push_back(block);
		}
	}

	void AppendReportSection(
		std::vector<ReportBlock>& blocks,
		CDC* pDC,
		CFont* sectionFont,
		const ReportMetrics& metrics,
		const CString& text,
		int textWidth)
	{
		ReportBlock block = {};
		block.type = ReportSection;
		block.text = text;
		CFont* previousFont = pDC->SelectObject(sectionFont);
		block.textHeight = MeasureWrappedText(
			pDC, block.text, textWidth, metrics.sectionLine);
		pDC->SelectObject(previousFont);
		block.textOffset = metrics.smallGap;
		block.height = block.textHeight + metrics.smallGap * 2;
		block.gapAfter = metrics.sectionGap;
		blocks.push_back(block);
	}

	void AppendReportConclusion(
		std::vector<ReportBlock>& blocks,
		CDC* pDC,
		CFont* bodyFont,
		const ReportMetrics& metrics,
		const CString& text,
		int textWidth)
	{
		ReportBlock block = {};
		block.type = ReportParagraph;
		block.text = text;
		block.textOffset = metrics.smallGap;
		CFont* previousFont = pDC->SelectObject(bodyFont);
		block.textHeight = MeasureWrappedText(
			pDC, block.text, textWidth - 2 * metrics.smallGap, metrics.bodyLine);
		pDC->SelectObject(previousFont);
		block.height = block.textOffset + block.textHeight + metrics.smallGap;
		block.gapAfter = metrics.smallGap;
		blocks.push_back(block);
	}

	void RemoveOuterReportSpacers(std::vector<ReportBlock>& blocks)
	{
		while (!blocks.empty() && blocks.front().type == ReportSpacer)
		{
			blocks.erase(blocks.begin());
		}
		while (!blocks.empty() && blocks.back().type == ReportSpacer)
		{
			blocks.pop_back();
		}
	}

	bool ArrangeReportSections(
		std::vector<ReportBlock>& blocks,
		CDC* pDC,
		CFont* sectionFont,
		CFont* bodyFont,
		const ReportMetrics& metrics,
		bool isFinnish,
		int textWidth)
	{
		size_t summaryStart = blocks.size();
		size_t summaryEnd = blocks.size();
		size_t statisticsStart = blocks.size();
		size_t statisticsEnd = blocks.size();
		for (size_t index = 0; index < blocks.size(); ++index)
		{
			if (blocks[index].type != ReportSection)
			{
				continue;
			}

			size_t sectionEnd = index + 1;
			while (sectionEnd < blocks.size() &&
				blocks[sectionEnd].type != ReportSection)
			{
				++sectionEnd;
			}

			if (IsPredictionSummaryHeading(blocks[index].text))
			{
				summaryStart = index;
				summaryEnd = sectionEnd;
			}
			else if (IsStatisticsHeading(blocks[index].text))
			{
				statisticsStart = index;
				statisticsEnd = index + 1;
				if (statisticsEnd < blocks.size() &&
					blocks[statisticsEnd].type == ReportStatisticsHeader)
				{
					++statisticsEnd;
					while (statisticsEnd < blocks.size() &&
						blocks[statisticsEnd].type == ReportStatisticsRow)
					{
						++statisticsEnd;
					}
				}
			}
		}

		if (summaryStart == blocks.size() || statisticsStart == blocks.size())
		{
			return false;
		}

		std::vector<ReportBlock> summaryBlocks;
		std::vector<ReportBlock> statisticsBlocks;
		std::vector<ReportBlock> detailedBlocks;
		for (size_t index = 0; index < blocks.size(); ++index)
		{
			if (index >= summaryStart && index < summaryEnd)
			{
				summaryBlocks.push_back(blocks[index]);
			}
			else if (index >= statisticsStart && index < statisticsEnd)
			{
				statisticsBlocks.push_back(blocks[index]);
			}
			else
			{
				detailedBlocks.push_back(blocks[index]);
			}
		}

		RemoveOuterReportSpacers(summaryBlocks);
		RemoveOuterReportSpacers(statisticsBlocks);
		RemoveOuterReportSpacers(detailedBlocks);

		std::vector<ReportBlock> orderedBlocks;
		orderedBlocks.insert(
			orderedBlocks.end(), summaryBlocks.begin(), summaryBlocks.end());
		orderedBlocks.insert(
			orderedBlocks.end(), statisticsBlocks.begin(), statisticsBlocks.end());

		if (!detailedBlocks.empty())
		{
			AppendReportSection(orderedBlocks, pDC, sectionFont, metrics,
				isFinnish ? L"Yksityiskohtaiset ennusteet" : L"Detailed Predictions",
				textWidth);
			orderedBlocks.insert(
				orderedBlocks.end(), detailedBlocks.begin(), detailedBlocks.end());
		}

		AppendReportSection(orderedBlocks, pDC, sectionFont, metrics,
			isFinnish ? L"Johtopäätökset" : L"Conclusions",
			textWidth);
		AppendReportConclusion(orderedBlocks, pDC, bodyFont, metrics,
			isFinnish
				? L"Historialliset arvontatulokset kuvaavat menneitä arvontoja eivätkä takaa tai ennusta tulevia tuloksia."
				: L"Historical draw results describe past draws and do not guarantee or predict future outcomes.",
			textWidth);

		blocks.swap(orderedBlocks);
		return true;
	}

	void PositionReportBlock(ReportBlock& block, int top, int sectionRuleOffset)
	{
		block.top = top;
		block.bottom = top + block.height;
		block.labelY = top + block.labelOffset;
		block.textY = top + block.textOffset;
		block.detailY = top + block.detailOffset;
		block.resultY = top + block.resultOffset;
		block.ruleY = block.bottom - sectionRuleOffset;
	}

	int GetPrintPageLogicalHeight(CDC* pDC);

	ReportLayout BuildReportLayout(
		CDC* pDC,
		const CString& metadata,
		CFont* bodyFont,
		const ReportMetrics& metrics,
		std::vector<ReportBlock>& blocks,
		bool paginate)
	{
		ReportLayout layout = {};
		layout.pageWidth = kA4PortraitWidth;
		layout.contentWidth = kA4PortraitWidth - 2 * kReportMargin;
		layout.pageLeft = 0;
		layout.contentLeft = kReportMargin;
		layout.statisticsColumnCount = 0;
		for (size_t index = 0; index < blocks.size(); ++index)
		{
			if (blocks[index].type == ReportStatisticsHeader)
			{
				layout.statisticsColumnCount =
					static_cast<int>(blocks[index].cells.size());
				break;
			}
		}
		layout.statisticsLabelWidth = min(132, max(104, layout.contentWidth / 5));
		layout.statisticsColumnWidth = layout.statisticsColumnCount > 0
			? max(1, (layout.contentWidth - layout.statisticsLabelWidth) /
				layout.statisticsColumnCount)
			: layout.contentWidth - layout.statisticsLabelWidth;

		const int topPadding = kReportMargin / 2;
		layout.headerKickerTop = topPadding;
		layout.headerTitleTop = layout.headerKickerTop +
			metrics.detailLine + metrics.smallGap;
		layout.headerGameTop = layout.headerTitleTop +
			metrics.titleLine + metrics.smallGap;
		layout.headerRuleY = layout.headerGameTop +
			metrics.subsectionLine + metrics.smallGap;
		layout.headerBandHeight = layout.headerRuleY + topPadding;
		layout.metadataTop = layout.headerBandHeight + metrics.blockGap;
		layout.metadataPadding = metrics.cardPadding;
		pDC->SelectObject(bodyFont);
		layout.metadataHeight = MeasureWrappedText(pDC, metadata,
			layout.contentWidth - 2 * layout.metadataPadding, metrics.bodyLine);
		layout.metadataTextTop = layout.metadataTop + layout.metadataPadding;
		layout.metadataCardBottom = layout.metadataTop + layout.metadataHeight +
			2 * layout.metadataPadding;
		layout.headerHeight = layout.metadataTop + layout.metadataHeight +
			2 * layout.metadataPadding;
		layout.bodyTop = layout.headerHeight + metrics.sectionGap;
		layout.footerReserve = metrics.footerReserve + kReportMargin;
		layout.sectionRuleOffset = metrics.smallGap;

		if (!paginate)
		{
			int contentHeight = 0;
			for (size_t index = 0; index < blocks.size(); ++index)
			{
				contentHeight += blocks[index].height + blocks[index].gapAfter;
			}
			layout.pageHeight = max(kA4PortraitHeight,
				layout.bodyTop + contentHeight + layout.footerReserve);
			layout.pages.push_back(ReportPage());
			layout.pages[0].drawHeader = true;

			int top = layout.bodyTop;
			bool alternate = false;
			for (size_t index = 0; index < blocks.size(); ++index)
			{
				ReportBlock block = blocks[index];
				PositionReportBlock(block, top, layout.sectionRuleOffset);
				block.alternate = alternate;
				layout.pages[0].blocks.push_back(block);
				top += block.height + block.gapAfter;
				if (block.type == ReportPrediction ||
					block.type == ReportStatisticsRow)
				{
					alternate = !alternate;
				}
			}
			layout.pageCount = 1;
		}
		else
		{
			layout.pageHeight = GetPrintPageLogicalHeight(pDC);
			const int contentBottom = layout.pageHeight - layout.footerReserve;
			layout.pages.push_back(ReportPage());
			layout.pages[0].drawHeader = true;
			int pageIndex = 0;
			int top = layout.bodyTop;
			bool alternate = false;
			bool hasActiveSection = false;
			bool activeSectionIsStatistics = false;
			ReportBlock activeSection = {};
			ReportBlock activeStatisticsHeader = {};
			bool hasActiveStatisticsHeader = false;

			for (size_t index = 0; index < blocks.size(); ++index)
			{
				ReportBlock block = blocks[index];
				int requiredHeight = block.height + block.gapAfter;
				if ((block.type == ReportSection ||
					block.type == ReportStatisticsHeader) &&
					index + 1 < blocks.size() &&
					blocks[index + 1].type != ReportSpacer)
				{
					requiredHeight += blocks[index + 1].height +
						blocks[index + 1].gapAfter;
				}

				if (!layout.pages[pageIndex].blocks.empty() &&
					top + requiredHeight > contentBottom)
				{
					layout.pages.push_back(ReportPage());
					layout.pages.back().drawHeader = false;
					++pageIndex;
					top = kReportMargin;
					alternate = false;

					if (block.type != ReportSection && hasActiveSection)
					{
						ReportBlock repeatedSection = activeSection;
						PositionReportBlock(
							repeatedSection, top, layout.sectionRuleOffset);
						repeatedSection.repeated = true;
						repeatedSection.alternate = false;
						layout.pages[pageIndex].blocks.push_back(repeatedSection);
						top += repeatedSection.height + repeatedSection.gapAfter;

						if (activeSectionIsStatistics &&
							hasActiveStatisticsHeader &&
							block.type != ReportStatisticsHeader)
						{
							ReportBlock repeatedHeader = activeStatisticsHeader;
							PositionReportBlock(
								repeatedHeader, top, layout.sectionRuleOffset);
							repeatedHeader.repeated = true;
							repeatedHeader.alternate = false;
							layout.pages[pageIndex].blocks.push_back(repeatedHeader);
							top += repeatedHeader.height + repeatedHeader.gapAfter;
						}
					}
				}

				PositionReportBlock(block, top, layout.sectionRuleOffset);
				block.alternate = alternate;
				layout.pages[pageIndex].blocks.push_back(block);
				top += block.height + block.gapAfter;
				if (block.type == ReportPrediction ||
					block.type == ReportStatisticsRow)
				{
					alternate = !alternate;
				}

				if (block.type == ReportSection)
				{
					activeSection = block;
					hasActiveSection = true;
					activeSectionIsStatistics = IsStatisticsHeading(block.text);
					hasActiveStatisticsHeader = false;
				}
				else if (block.type == ReportStatisticsHeader)
				{
					activeStatisticsHeader = block;
					hasActiveStatisticsHeader = true;
				}
				else if (block.type == ReportSpacer)
				{
					hasActiveSection = false;
					activeSectionIsStatistics = false;
					hasActiveStatisticsHeader = false;
				}
			}
			layout.pageCount = static_cast<int>(layout.pages.size());
		}

		for (size_t index = 0; index < layout.pages.size(); ++index)
		{
			layout.pages[index].footerY = layout.pageHeight -
				kReportMargin - metrics.detailLine;
			layout.pages[index].footerRuleY =
				layout.pages[index].footerY - metrics.smallGap;
		}
		return layout;
	}

	void DrawReportHeader(
		CDC* pDC,
		const ReportLayout& layout,
		const CString& gameName,
		const CString& metadata,
		CFont* titleFont,
		CFont* subsectionFont,
		CFont* bodyFont,
		CFont* detailFont,
		bool isFinnish)
	{
		pDC->FillSolidRect(layout.pageLeft + 5, -layout.pageHeight - 5,
			layout.pageWidth, layout.pageHeight, RGB(215, 222, 231));
		pDC->FillSolidRect(layout.pageLeft, -layout.pageHeight,
			layout.pageWidth, layout.pageHeight, RGB(255, 255, 255));
		pDC->FillSolidRect(layout.pageLeft, -layout.headerBandHeight,
			layout.pageWidth, layout.headerBandHeight, RGB(25, 44, 74));

		pDC->SelectObject(detailFont);
		pDC->SetTextColor(RGB(176, 202, 225));
		DrawCenteredText(pDC,
			isFinnish ? L"HISTORIATULOKSET  /  ENNUSTENÄKYMÄ"
				: L"HISTORICAL RESULTS  /  PREDICTION INSIGHTS",
			layout.pageLeft, layout.headerKickerTop, layout.pageWidth);

		pDC->SelectObject(titleFont);
		pDC->SetTextColor(RGB(255, 255, 255));
		DrawCenteredText(pDC, L"LOTTOGUI ANALYTICS REPORT",
			layout.pageLeft + kReportMargin, layout.headerTitleTop,
			layout.pageWidth - 2 * kReportMargin);

		pDC->SelectObject(subsectionFont);
		pDC->SetTextColor(RGB(225, 233, 242));
		DrawCenteredText(pDC, gameName, layout.pageLeft + kReportMargin, layout.headerGameTop,
			layout.pageWidth - 2 * kReportMargin);

		pDC->FillSolidRect(layout.pageLeft + kReportMargin, -layout.headerRuleY,
			layout.pageWidth - 2 * kReportMargin, 1, RGB(89, 119, 152));

		const int metadataLeft = layout.contentLeft;
		const int metadataCardHeight =
			layout.metadataCardBottom - layout.metadataTop;
		pDC->FillSolidRect(metadataLeft, -layout.metadataCardBottom,
			layout.contentWidth, metadataCardHeight, RGB(242, 246, 250));
		pDC->FillSolidRect(metadataLeft, -layout.metadataCardBottom,
			4, metadataCardHeight, RGB(63, 133, 177));
		pDC->SelectObject(bodyFont);
		pDC->SetTextColor(RGB(62, 77, 95));
		DrawWrappedText(pDC, metadata, metadataLeft + layout.metadataPadding,
			layout.metadataTextTop,
			layout.contentWidth - 2 * layout.metadataPadding, layout.metadataHeight);
	}

	void DrawReportSection(
		CDC* pDC,
		const ReportBlock& block,
		int left,
		int width,
		CFont* sectionFont)
	{
		pDC->SelectObject(sectionFont);
		pDC->SetTextColor(RGB(31, 61, 94));
		DrawWrappedText(pDC, block.text, left, block.textY,
			width, block.textHeight);
		pDC->FillSolidRect(left, -block.ruleY, width, 1,
			RGB(211, 222, 233));
		pDC->FillSolidRect(left, -block.ruleY, 34, 2,
			RGB(48, 122, 169));
	}

	void DrawReportPrediction(
		CDC* pDC,
		const ReportBlock& block,
		int left,
		int width,
		CFont* subsectionFont,
		CFont* predictionFont,
		CFont* detailFont,
		CFont* resultFont,
		bool alternate,
		int cardPadding)
	{
		const COLORREF cardColor = block.result.IsEmpty()
			? (alternate ? RGB(248, 250, 253) : RGB(243, 247, 251))
			: (block.isWin ? RGB(240, 248, 244) : RGB(250, 247, 247));
		pDC->FillSolidRect(left, -block.bottom, width, block.height, cardColor);
		pDC->FillSolidRect(left, -block.bottom, 4, block.height,
			block.isWin ? RGB(49, 142, 99) : RGB(60, 126, 169));

		if (!block.label.IsEmpty())
		{
			pDC->SelectObject(subsectionFont);
			pDC->SetTextColor(RGB(88, 107, 129));
			DrawWrappedText(pDC, block.label, left + cardPadding,
				block.labelY, width - 2 * cardPadding,
				block.labelHeight);
		}

		pDC->SelectObject(predictionFont);
		pDC->SetTextColor(RGB(29, 49, 70));
		DrawWrappedText(pDC, block.text, left + cardPadding,
			block.textY, width - 2 * cardPadding,
			block.textHeight);

		if (block.detailHeight > 0)
		{
			pDC->SelectObject(detailFont);
			pDC->SetTextColor(RGB(100, 116, 133));
			DrawWrappedText(pDC, block.detail, left + cardPadding,
				block.detailY, width - 2 * cardPadding,
				block.detailHeight);
		}
		if (block.resultHeight > 0)
		{
			pDC->SelectObject(resultFont);
			pDC->SetTextColor(block.isWin ? RGB(27, 112, 78) : RGB(126, 79, 79));
			DrawWrappedText(pDC, block.result, left + cardPadding,
				block.resultY, width - 2 * cardPadding,
				block.resultHeight);
		}
	}

	void DrawStatisticsRow(
		CDC* pDC,
		const ReportBlock& block,
		int left,
		int top,
		int contentWidth,
		int labelWidth,
		int columnWidth,
		CFont* statisticsFont,
		CFont* statisticsHeaderFont,
		bool alternate)
	{
		const bool isHeader = block.type == ReportStatisticsHeader;
		pDC->FillSolidRect(left, -(top + block.height), contentWidth, block.height,
			isHeader ? RGB(230, 238, 247)
				: (alternate ? RGB(249, 251, 253) : RGB(241, 245, 249)));
		pDC->SelectObject(isHeader ? statisticsHeaderFont : statisticsFont);
		pDC->SetTextColor(isHeader ? RGB(44, 71, 101) : RGB(59, 73, 89));

		if (isHeader)
		{
			for (size_t index = 0; index < block.cells.size(); ++index)
			{
				DrawSingleLineCell(pDC, block.cells[index],
					left + labelWidth + static_cast<int>(index) * columnWidth + 4,
					top, columnWidth - 4, block.height, DT_LEFT);
			}
		}
		else if (!block.cells.empty())
		{
			DrawSingleLineCell(pDC, block.cells[0], left + 8, top,
				labelWidth - 12, block.height, DT_LEFT);
			for (size_t index = 1; index < block.cells.size(); ++index)
			{
				DrawSingleLineCell(pDC, block.cells[index],
					left + labelWidth + static_cast<int>(index - 1) * columnWidth + 4,
					top, columnWidth - 4, block.height, DT_LEFT);
			}
		}
		pDC->FillSolidRect(left, -(top + block.height), contentWidth, 1,
			RGB(226, 232, 239));
	}

	ReportLayout CreateReportLayout(
		CDC* pDC,
		CLottoGuiDoc* pDoc,
		const CString& metadata,
		ReportFonts& fonts,
		bool isFinnish,
		bool paginate)
	{
		const int statisticsColumnCount = CountStatisticsColumns(pDoc);
		CreateReportFonts(fonts, pDC, statisticsColumnCount);
		const ReportMetrics metrics = MeasureReportMetrics(pDC,
			&fonts.title, &fonts.section, &fonts.subsection, &fonts.body,
			&fonts.prediction, &fonts.detail, &fonts.result, &fonts.statistics);

		std::vector<ReportBlock> blocks;
		int parsedStatisticsColumnCount = 0;
		const int contentWidth = kA4PortraitWidth - 2 * kReportMargin;
		const int textWidth = contentWidth - 2 * metrics.cardPadding;
		BuildReportBlocks(pDoc, pDC, &fonts.body, &fonts.prediction,
			&fonts.subsection, &fonts.detail, &fonts.result, &fonts.section,
			&fonts.statistics, &fonts.statisticsHeader, metrics, textWidth,
			isFinnish, blocks, parsedStatisticsColumnCount);
		ArrangeReportSections(blocks, pDC, &fonts.section, &fonts.body,
			metrics, isFinnish, textWidth);

		return BuildReportLayout(pDC, metadata, &fonts.body, metrics, blocks, paginate);
	}

	void DrawReportPage(
		CDC* pDC,
		const ReportLayout& layout,
		const ReportPage& page,
		int pageNumber,
		const CString& gameName,
		const CString& metadata,
		ReportFonts& fonts,
		const ReportMetrics& metrics,
		bool isFinnish)
	{
		if (page.drawHeader)
		{
			DrawReportHeader(pDC, layout, gameName, metadata,
				&fonts.title, &fonts.subsection, &fonts.body, &fonts.detail, isFinnish);
		}
		else
		{
			pDC->FillSolidRect(layout.pageLeft, -layout.pageHeight,
				layout.pageWidth, layout.pageHeight, RGB(255, 255, 255));
		}

		for (size_t index = 0; index < page.blocks.size(); ++index)
		{
			const ReportBlock& block = page.blocks[index];
			switch (block.type)
			{
			case ReportSection:
				DrawReportSection(pDC, block, layout.contentLeft,
					layout.contentWidth, &fonts.section);
				break;
			case ReportPrediction:
				DrawReportPrediction(pDC, block, layout.contentLeft,
					layout.contentWidth, &fonts.subsection, &fonts.prediction,
					&fonts.detail, &fonts.result, block.alternate,
					metrics.cardPadding);
				break;
			case ReportStatisticsHeader:
			case ReportStatisticsRow:
				DrawStatisticsRow(pDC, block, layout.contentLeft, block.top,
					layout.contentWidth, layout.statisticsLabelWidth,
					layout.statisticsColumnWidth, &fonts.statistics,
					&fonts.statisticsHeader, block.alternate);
				break;
			case ReportParagraph:
				pDC->FillSolidRect(layout.contentLeft,
					-block.bottom, layout.contentWidth,
					block.height, RGB(247, 249, 252));
				pDC->SelectObject(&fonts.body);
				pDC->SetTextColor(RGB(55, 70, 87));
				DrawWrappedText(pDC, block.text, layout.contentLeft + metrics.smallGap,
					block.textY,
					layout.contentWidth - 2 * metrics.smallGap, block.textHeight);
				break;
			case ReportSpacer:
				break;
			}
		}

		pDC->FillSolidRect(layout.contentLeft, -page.footerRuleY,
			layout.contentWidth, 1, RGB(218, 225, 232));
		CString footer = isFinnish
			? L"LottoGui  |  Tilasto- ja tulosraportti"
			: L"LottoGui  |  Statistics and results report";
		CString pageText;
		pageText.Format(isFinnish ? L"Sivu %d / %d" : L"Page %d / %d",
			pageNumber, layout.pageCount);
		pDC->SelectObject(&fonts.detail);
		pDC->SetTextColor(RGB(127, 139, 151));
		pDC->TextOutW(layout.contentLeft, -page.footerY, footer);
		const CSize pageTextSize = pDC->GetTextExtent(pageText);
		pDC->TextOutW(layout.contentLeft + layout.contentWidth - pageTextSize.cx,
			-page.footerY, pageText);
	}

	int GetPrintScalePercent(CDC* pDC)
	{
		const int dpiX = max(1, pDC->GetDeviceCaps(LOGPIXELSX));
		const int printableWidth = max(1, pDC->GetDeviceCaps(HORZRES));
		const int printableWidthLogical = MulDiv(printableWidth, 100, dpiX);
		return max(1, min(10000,
			MulDiv(printableWidthLogical, 10000, kA4PortraitWidth)));
	}

	int GetPrintPageLogicalHeight(CDC* pDC)
	{
		const int dpiY = max(1, pDC->GetDeviceCaps(LOGPIXELSY));
		const int printableHeight = max(1, pDC->GetDeviceCaps(VERTRES));
		const int viewportHeight = max(1,
			MulDiv(dpiY, GetPrintScalePercent(pDC), 10000));
		return max(1, MulDiv(printableHeight, 100, viewportHeight));
	}

	void ConfigurePrintMapping(CDC* pDC)
	{
		const CPoint viewportOrigin = pDC->GetViewportOrg();
		const int scalePercent = GetPrintScalePercent(pDC);
		const int dpiX = max(1, pDC->GetDeviceCaps(LOGPIXELSX));
		const int dpiY = max(1, pDC->GetDeviceCaps(LOGPIXELSY));
		pDC->SetMapMode(MM_ANISOTROPIC);
		pDC->SetWindowExt(100, 100);
		pDC->SetViewportExt(
			max(1, MulDiv(dpiX, scalePercent, 10000)),
			-max(1, MulDiv(dpiY, scalePercent, 10000)));
		pDC->SetViewportOrg(viewportOrigin);
		pDC->SetWindowOrg(0, 0);
	}
}

// CLottoGuiView

IMPLEMENT_DYNCREATE(CLottoGuiView, CScrollView)

BEGIN_MESSAGE_MAP(CLottoGuiView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CLottoGuiView construction/destruction

CLottoGuiView::CLottoGuiView()
	: m_nReportContentBottom(kA4PortraitHeight)
{
	// TODO: add construction code here

}

CLottoGuiView::~CLottoGuiView()
{
}

BOOL CLottoGuiView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CLottoGuiView drawing

void CLottoGuiView::OnDraw(CDC* pDC)
{
	CLottoGuiDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	const int savedDC = pDC->SaveDC();
	const bool isFinnish = PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH;

	CString gameName = pDoc->m_strCurrentGameType;
	if (gameName.IsEmpty())
	{
		gameName = isFinnish ? L"Lottoanalyysi" : L"Lottery analysis";
	}

	CString metadata = pDoc->m_strSampleNums;
	if (metadata.IsEmpty())
	{
		metadata = isFinnish
			? L"Luo tulokset valitsemalla peli Lotto-valinnat-valikosta."
			: L"Generate results by selecting a game from the Lotto Options menu.";
	}

	ReportFonts fonts;
	ReportLayout layout = CreateReportLayout(
		pDC, pDoc, metadata, fonts, isFinnish, false);
	const ReportMetrics metrics = MeasureReportMetrics(pDC,
		&fonts.title, &fonts.section, &fonts.subsection, &fonts.body,
		&fonts.prediction, &fonts.detail, &fonts.result, &fonts.statistics);

	CRect clientRect;
	GetClientRect(&clientRect);
	pDC->DPtoLP(&clientRect);
	const int canvasWidth = max(kA4PortraitWidth, clientRect.Width());
	layout.pageLeft = max(0, (canvasWidth - kA4PortraitWidth) / 2);
	layout.contentLeft = layout.pageLeft + kReportMargin;
	SetScrollSizes(MM_LOENGLISH, CSize(canvasWidth, layout.pageHeight));
	m_nReportContentBottom = layout.pageHeight;

	pDC->SetBkMode(TRANSPARENT);
	DrawReportPage(pDC, layout, layout.pages[0], 1, gameName, metadata,
		fonts, metrics, isFinnish);
	pDC->RestoreDC(savedDC);
}

void CLottoGuiView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CLottoGuiDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc || !pInfo)
		return;

	const int savedDC = pDC->SaveDC();
	const bool isFinnish = PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH;
	CString gameName = pDoc->m_strCurrentGameType;
	if (gameName.IsEmpty())
	{
		gameName = isFinnish ? L"Lottoanalyysi" : L"Lottery analysis";
	}

	CString metadata = pDoc->m_strSampleNums;
	if (metadata.IsEmpty())
	{
		metadata = isFinnish
			? L"Luo tulokset valitsemalla peli Lotto-valinnat-valikosta."
			: L"Generate results by selecting a game from the Lotto Options menu.";
	}

	ReportFonts fonts;
	ReportLayout layout = CreateReportLayout(
		pDC, pDoc, metadata, fonts, isFinnish, true);
	if (pInfo->m_nCurPage == 0 ||
		pInfo->m_nCurPage > static_cast<UINT>(layout.pages.size()))
	{
		pInfo->m_bContinuePrinting = FALSE;
		pDC->RestoreDC(savedDC);
		return;
	}

	const ReportMetrics metrics = MeasureReportMetrics(pDC,
		&fonts.title, &fonts.section, &fonts.subsection, &fonts.body,
		&fonts.prediction, &fonts.detail, &fonts.result, &fonts.statistics);
	layout.pageLeft = 0;
	layout.contentLeft = kReportMargin;
	pDC->SetBkMode(TRANSPARENT);
	const size_t pageIndex = static_cast<size_t>(pInfo->m_nCurPage - 1);
	DrawReportPage(pDC, layout, layout.pages[pageIndex],
		static_cast<int>(pInfo->m_nCurPage), gameName, metadata,
		fonts, metrics, isFinnish);
	pDC->RestoreDC(savedDC);
}

void CLottoGuiView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;

	// Calculate the total size of this view
	sizeTotal.cx = kA4PortraitWidth;
	sizeTotal.cy = kA4PortraitHeight;
	SetScrollSizes(MM_LOENGLISH, sizeTotal);
}

// CLottoGuiView printing

void CLottoGuiView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	if (!pDC->IsPrinting() || pInfo == NULL)
	{
		CScrollView::OnPrepareDC(pDC, pInfo);
		return;
	}

	CView::OnPrepareDC(pDC, pInfo);
	ConfigurePrintMapping(pDC);
}

BOOL CLottoGuiView::OnPreparePrinting(CPrintInfo* pInfo)
{
	UpdateWindow();
	const int estimatedPageHeight = 800;
	const int pageCount = max(1,
		(m_nReportContentBottom + estimatedPageHeight - 1) / estimatedPageHeight);
	pInfo->SetMaxPage(static_cast<UINT>(pageCount));
	return DoPreparePrinting(pInfo);
}

void CLottoGuiView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CLottoGuiDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	const int savedDC = pDC->SaveDC();
	CView::OnPrepareDC(pDC, pInfo);
	ConfigurePrintMapping(pDC);

	const bool isFinnish = PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH;
	CString metadata = pDoc->m_strSampleNums;
	if (metadata.IsEmpty())
	{
		metadata = isFinnish
			? L"Luo tulokset valitsemalla peli Lotto-valinnat-valikosta."
			: L"Generate results by selecting a game from the Lotto Options menu.";
	}

	ReportFonts fonts;
	const ReportLayout layout = CreateReportLayout(
		pDC, pDoc, metadata, fonts, isFinnish, true);
	pInfo->SetMaxPage(static_cast<UINT>(max(1, layout.pageCount)));
	m_nReportContentBottom = layout.pageCount * layout.pageHeight;
	pDC->RestoreDC(savedDC);
}

void CLottoGuiView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CLottoGuiView diagnostics

#ifdef _DEBUG
void CLottoGuiView::AssertValid() const
{
	CView::AssertValid();
}

void CLottoGuiView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLottoGuiDoc* CLottoGuiView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLottoGuiDoc)));
	return (CLottoGuiDoc*)m_pDocument;
}
#endif //_DEBUG


// CLottoGuiView message handlers
