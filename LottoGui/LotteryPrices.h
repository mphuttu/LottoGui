#pragma once

#include <afxwin.h>

namespace LotteryPrices
{
	enum
	{
		kSuomenLottoBaseCents = 100,
		kSuomenLottoPlusCents = 50,
		kMilliBaseCents = 100,
		kKtemMinimumStakeCents = 100,
		kKtemMaximumStakeCents = 500,
		kKtemDefaultStakeCents = 100,
		kKenoMinimumStakeCents = 50,
		kKenoMaximumStakeCents = 1000,
		kKenoDefaultStakeCents = 100,
		kStakeStepCents = 50,
		kKenoMinimumLevel = 2,
		kKenoMaximumLevel = 10,
		kEurojackpotBaseCents = 200,
		kVikingLottoBaseCents = 100,
		kVikingLottoPlusCents = 50,
		kJokeriBaseCents = 200,
		kJokeriDoubleCents = 100
	};

	inline CString FormatCents(int cents)
	{
		CString amount;
		amount.Format(L"%d,%02d €", cents / 100, cents % 100);
		return amount;
	}

	inline CString FormatPriceLabel(LPCTSTR label, int cents)
	{
		CString text;
		text.Format(L"%s%s", label, FormatCents(cents).GetString());
		return text;
	}

	inline CString FormatLinePrice(int cents)
	{
		const LPCTSTR label = (PRIMARYLANGID(::GetThreadLocale()) == LANG_FINNISH)
			? L"Rivihinta: " : L"Line price: ";
		return FormatPriceLabel(label, cents);
	}

	inline CString FormatOptionPrice(LPCTSTR option, int cents)
	{
		CString text;
		text.Format(L"%s (+%s)", option, FormatCents(cents).GetString());
		return text;
	}

	inline void AddStakeOptions(CComboBox* combo, int minimumCents, int maximumCents)
	{
		for (int cents = minimumCents; cents <= maximumCents; cents += kStakeStepCents)
		{
			combo->AddString(FormatCents(cents));
		}
	}
}
