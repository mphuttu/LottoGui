#include "LottoGuiCore.h"

#include <map>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
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
	bool IsValidResultCheckContext(const ResultCheckContext& context)
	{
		if (context.mainCount <= 0)
			return false;
		if (context.game == L"Custom")
			return true;
		if (context.game == L"Suomen Lotto")
			return context.mainCount == 7;
		if (context.game == L"Milli" || context.game == L"Viking Lotto")
			return context.mainCount == 6;
		if (context.game == L"Eurojackpot")
			return context.mainCount == 5;
		if (context.game == L"Jokeri")
			return context.mainCount == 7;
		if (context.game == L"KTEM")
			return context.mainCount == 12 && context.ktemStakeCents >= 100 &&
				context.ktemStakeCents <= 500 && context.ktemStakeCents % 50 == 0;
		if (context.game == L"Keno")
			return context.kenoLevel >= 2 && context.kenoLevel <= 10 &&
				context.mainCount == context.kenoLevel && context.kenoStakeCents >= 50 &&
				context.kenoStakeCents <= 1000 && context.kenoStakeCents % 50 == 0;
		return false;
	}

	std::wstring EncodeResultCheckContext(const ResultCheckContext& context)
	{
		if (!IsValidResultCheckContext(context))
			return std::wstring();
		std::wostringstream stream;
		stream << context.game << L'\n' << context.mainCount << L' '
			<< (context.jokeriDouble ? 1 : 0) << L' ' << context.kenoLevel << L' '
			<< context.kenoStakeCents << L' ' << (context.kenoKunkku ? 1 : 0) << L' '
			<< context.ktemStakeCents;
		return stream.str();
	}

	bool DecodeResultCheckContext(const std::wstring& text, ResultCheckContext& context)
	{
		context = ResultCheckContext();
		ResultCheckContext parsed;
		int jokeriDouble = 0;
		int kenoKunkku = 0;
		std::wistringstream stream(text);
		std::wstring extra;
		if (!std::getline(stream, parsed.game) ||
			!(stream >> parsed.mainCount >> jokeriDouble >> parsed.kenoLevel >>
				parsed.kenoStakeCents >> kenoKunkku >> parsed.ktemStakeCents) ||
			(stream >> extra) || (jokeriDouble != 0 && jokeriDouble != 1) ||
			(kenoKunkku != 0 && kenoKunkku != 1))
			return false;
		parsed.jokeriDouble = jokeriDouble != 0;
		parsed.kenoKunkku = kenoKunkku != 0;
		if (!IsValidResultCheckContext(parsed))
			return false;
		context = parsed;
		return true;
	}

	bool IsPredictionCompatibleWithGame(const ResultCheckContext& context,
		const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers)
	{
		if (!IsValidResultCheckContext(context) ||
			mainNumbers.size() != static_cast<std::size_t>(context.mainCount))
			return false;
		if (context.game == L"Custom")
			return true;
		if (context.game == L"Jokeri")
			return IsValidJokeriDraw(mainNumbers, bonusNumbers);
		int maximum = 40;
		if (context.game == L"KTEM") maximum = 24;
		else if (context.game == L"Keno") maximum = 70;
		else if (context.game == L"Eurojackpot") maximum = 50;
		else if (context.game == L"Viking Lotto") maximum = 48;
		std::vector<bool> seen(maximum + 1, false);
		for (int n : mainNumbers)
		{
			if (n < 1 || n > maximum || seen[n]) return false;
			seen[n] = true;
		}
		if (context.game == L"Eurojackpot" || context.game == L"Viking Lotto")
		{
			const int bonusMaximum = context.game == L"Eurojackpot" ? 12 : 5;
			const std::size_t bonusCount = context.game == L"Eurojackpot" ? 2 : 1;
			if (bonusNumbers.size() != bonusCount) return false;
			std::vector<bool> seenBonus(bonusMaximum + 1, false);
			for (int n : bonusNumbers)
			{
				if (n < 1 || n > bonusMaximum || seenBonus[n]) return false;
				seenBonus[n] = true;
			}
			return true;
		}
		return bonusNumbers.empty();
	}

	bool IsValidActualDraw(const ResultCheckContext& context,
		const std::vector<int>& mainNumbers, const std::vector<int>& bonusNumbers)
	{
		if (!IsValidResultCheckContext(context)) return false;
		if (context.game == L"Custom") return !mainNumbers.empty();
		if (context.game == L"Jokeri") return IsValidJokeriDraw(mainNumbers, bonusNumbers);
		int maximum = 40;
		std::size_t drawCount = static_cast<std::size_t>(context.mainCount);
		if (context.game == L"KTEM") maximum = 24;
		else if (context.game == L"Keno") { maximum = 70; drawCount = 20; }
		else if (context.game == L"Eurojackpot") maximum = 50;
		else if (context.game == L"Viking Lotto") maximum = 48;
		if (mainNumbers.size() != drawCount) return false;
		std::vector<bool> seen(maximum + 1, false);
		for (int n : mainNumbers)
		{
			if (n < 1 || n > maximum || seen[n]) return false;
			seen[n] = true;
		}
		if (bonusNumbers.empty())
			return context.game != L"Suomen Lotto" &&
				context.game != L"Milli" && context.game != L"Eurojackpot" &&
				context.game != L"Viking Lotto";
		if (context.game == L"Suomen Lotto" || context.game == L"Milli")
		{
			if (bonusNumbers.size() != 1) return false;
			const int n = bonusNumbers[0];
			return n >= 1 && n <= maximum && !seen[n];
		}
		if (context.game != L"Eurojackpot" && context.game != L"Viking Lotto")
			return false;
		const int bonusMaximum = context.game == L"Eurojackpot" ? 12 : 5;
		const std::size_t bonusCount = context.game == L"Eurojackpot" ? 2 : 1;
		if (bonusNumbers.size() != bonusCount) return false;
		std::vector<bool> seenBonus(bonusMaximum + 1, false);
		for (int n : bonusNumbers)
		{
			if (n < 1 || n > bonusMaximum || seenBonus[n]) return false;
			seenBonus[n] = true;
		}
		return true;
	}

	StructuralResult GenerateStructuralLine(int maximumNumber, int picks,
		const StructuralOptions& options, std::mt19937& generator, std::vector<int>& line)
	{
		line.clear();
		if (maximumNumber < 1 || maximumNumber > 70 || picks < 1 ||
			picks > maximumNumber || options.minimumGap < 1 || options.minimumGap > maximumNumber)
			return StructuralResult::InvalidOptions;
		if ((picks - 1) * options.minimumGap > maximumNumber - 1)
			return StructuralResult::NoSolution;

		const auto isPrime = [](int n)
		{
			if (n < 2) return false;
			for (int d = 2; d * d <= n; ++d)
				if (n % d == 0) return false;
			return true;
		};
		int primePopulation = 0;
		std::vector<int> candidates(maximumNumber);
		std::iota(candidates.begin(), candidates.end(), 1);
		for (int n : candidates)
			if (isPrime(n)) ++primePopulation;
		std::shuffle(candidates.begin(), candidates.end(), generator);
		const int primeTarget = static_cast<int>(std::round(
			static_cast<double>(picks) * primePopulation / maximumNumber));
		const int minimumPrimes = (std::max)(0, primeTarget - 1);
		const int maximumPrimes = (std::min)(picks, primeTarget + 1);
		const int requiredGroups = (std::min)(picks, (maximumNumber + 9) / 10);
		const int halfLow = picks / 2;
		const int halfHigh = (picks + 1) / 2;
		std::size_t visited = 0;
		const std::size_t nodeLimit = 200000;
		bool exhausted = false;

		std::function<bool(const std::vector<int>&, int, int, int, int)> search;
		search = [&](const std::vector<int>& pool, int groupMask,
			int primeCount, int evenCount, int smallCount)
		{
			if (++visited > nodeLimit)
			{
				exhausted = true;
				return false;
			}
			const int needed = picks - static_cast<int>(line.size());
			if (static_cast<int>(pool.size()) < needed)
				return false;
			if (options.primes && (primeCount > maximumPrimes || primeCount + needed < minimumPrimes))
				return false;
			if (options.balanced && (evenCount > halfHigh || evenCount + needed < halfLow ||
				smallCount > halfHigh || smallCount + needed < halfLow))
				return false;
			int groupCount = 0;
			for (int mask = groupMask; mask != 0; mask >>= 1)
				groupCount += mask & 1;
			if (options.decades && groupCount + needed < requiredGroups)
				return false;
			if (needed == 0)
				return !options.decades || groupCount >= requiredGroups;

			for (std::size_t i = 0; i < pool.size() && !exhausted; ++i)
			{
				const int n = pool[i];
				int endingCount = 0;
				for (int selected : line)
					if (selected % 10 == n % 10) ++endingCount;
				if (options.endings && endingCount >= 2)
					continue;
				line.push_back(n);
				if (options.avoidRuns)
				{
					std::vector<int> sorted(line);
					std::sort(sorted.begin(), sorted.end());
					int run = 1;
					bool tooLong = false;
					for (std::size_t j = 1; j < sorted.size(); ++j)
					{
						run = sorted[j] == sorted[j - 1] + 1 ? run + 1 : 1;
						if (run > 3) { tooLong = true; break; }
					}
					if (tooLong) { line.pop_back(); continue; }
				}
				std::vector<int> next;
				for (std::size_t j = i + 1; j < pool.size(); ++j)
					if (std::abs(pool[j] - n) >= options.minimumGap)
						next.push_back(pool[j]);
				if (search(next, groupMask | (1 << ((n - 1) / 10)),
					primeCount + (isPrime(n) ? 1 : 0), evenCount + (n % 2 == 0 ? 1 : 0),
					smallCount + (n <= (maximumNumber + 1) / 2 ? 1 : 0)))
					return true;
				line.pop_back();
			}
			return false;
		};
		if (search(candidates, 0, 0, 0, 0))
		{
			std::sort(line.begin(), line.end());
			return StructuralResult::Success;
		}
		line.clear();
		return exhausted ? StructuralResult::SearchLimit : StructuralResult::NoSolution;
	}

	namespace
	{
		bool Prime(int n)
		{
			if (n < 2) return false;
			for (int d = 2; d * d <= n; ++d)
				if (n % d == 0) return false;
			return true;
		}

		int BitCount(std::uint64_t value)
		{
			int count = 0;
			while (value) { value &= value - 1; ++count; }
			return count;
		}
	}

	bool BuildHistoricalProfile(const std::vector<std::vector<int>>& rows,
		int maximumNumber, int drawSize, int picks, bool positional, HistoricalProfile& profile)
	{
		profile = HistoricalProfile();
		if (maximumNumber < 1 || maximumNumber > 70 || drawSize < 1 ||
			picks < 1 || picks > drawSize || (!positional && drawSize > maximumNumber) ||
			(positional && picks != drawSize))
			return false;
		profile.maximumNumber = maximumNumber;
		profile.picks = picks;
		profile.drawSize = drawSize;
		profile.positional = positional;
		profile.decades.assign(positional ? 1 : maximumNumber / 10 + 1, 0.0);
		std::vector<double> sums;
		double withinVariance = 0;
		int primes = 0;
		for (const auto& source : rows)
		{
			if (source.size() < static_cast<std::size_t>(drawSize)) continue;
			std::vector<int> row(source.begin(), source.begin() + drawSize);
			std::uint64_t low = 0, high = 0;
			bool valid = true;
			int sum = 0, rowPrimes = 0;
			std::vector<int> groups(profile.decades.size(), 0);
			for (int n : row)
			{
				if (n < (positional ? 0 : 1) || n > maximumNumber) { valid = false; break; }
				if (!positional)
				{
					std::uint64_t& mask = n <= 64 ? low : high;
					const std::uint64_t bit = std::uint64_t(1) << ((n - 1) % 64);
					if (mask & bit) { valid = false; break; }
					mask |= bit;
				}
				sum += n;
				rowPrimes += Prime(n) ? 1 : 0;
				++groups[positional ? 0 : n / 10];
			}
			if (!valid) continue;
			profile.rows.push_back(row);
			profile.lowMasks.push_back(low);
			profile.highMasks.push_back(high);
			const double scaledSum = static_cast<double>(sum) * picks / drawSize;
			sums.push_back(scaledSum);
			primes += rowPrimes;
			for (std::size_t i = 0; i < groups.size(); ++i)
				profile.decades[i] += static_cast<double>(groups[i]) * picks / drawSize;
			++profile.decadePatterns[groups];
			if (!positional && picks < drawSize && drawSize > 1)
			{
				double squared = 0;
				for (int n : row) squared += (n - static_cast<double>(sum) / drawSize) *
					(n - static_cast<double>(sum) / drawSize);
				withinVariance += static_cast<double>(picks) * (drawSize - picks) /
					(drawSize * (drawSize - 1)) * squared;
			}
		}
		if (sums.empty()) return false;
		const double count = static_cast<double>(sums.size());
		profile.mean = std::accumulate(sums.begin(), sums.end(), 0.0) / count;
		std::sort(sums.begin(), sums.end());
		profile.median = (sums[(sums.size() - 1) / 2] + sums[sums.size() / 2]) / 2;
		double variance = withinVariance / count;
		for (double sum : sums) variance += (sum - profile.mean) * (sum - profile.mean) / count;
		profile.deviation = std::sqrt(variance);
		profile.primeRatio = primes / (count * drawSize);
		for (double& group : profile.decades) group /= count;
		return true;
	}

	bool EvaluateHistoricalLine(const HistoricalProfile& profile, const std::vector<int>& line,
		const StructuralOptions& options, double& score)
	{
		score = 0;
		if (profile.rows.empty() || line.size() != static_cast<std::size_t>(profile.picks))
			return false;
		std::uint64_t low = 0, high = 0;
		int sum = 0, even = 0, small = 0, primes = 0;
		std::vector<double> groups(profile.decades.size(), 0.0);
		for (int n : line)
		{
			if (n < (profile.positional ? 0 : 1) || n > profile.maximumNumber) return false;
			if (!profile.positional)
			{
				std::uint64_t& mask = n <= 64 ? low : high;
				const std::uint64_t bit = std::uint64_t(1) << ((n - 1) % 64);
				if (mask & bit) return false;
				mask |= bit;
			}
			sum += n;
			even += n % 2 == 0;
			small += n <= profile.maximumNumber / 2;
			primes += Prime(n);
			++groups[profile.positional ? 0 : n / 10];
		}
		const bool compareHistory = options.excludeHistory || options.avoidSimilarHistory ||
			options.historicalNovelty;
		int mostShared = 0;
		if (compareHistory)
			for (std::size_t i = 0; i < profile.rows.size(); ++i)
			{
				int shared = 0;
				if (profile.positional)
					for (int j = 0; j < profile.picks; ++j)
						shared += line[j] == profile.rows[i][j];
				else
					shared = BitCount(low & profile.lowMasks[i]) + BitCount(high & profile.highMasks[i]);
				mostShared = (std::max)(mostShared, shared);
				if ((options.excludeHistory || options.historicalNovelty) && shared == profile.picks)
					return false;
				if (options.avoidSimilarHistory && shared > (std::max)(0, profile.picks - 2))
					return false;
			}
		const double difference = std::abs(sum - profile.mean);
		if (options.historicalSum && difference > profile.deviation + 1e-9) return false;
		double decadeDistance = 0;
		if (options.historicalDecades || options.historicalNovelty)
			for (const auto& pattern : profile.decadePatterns)
			{
				double distance = 0;
				for (std::size_t i = 0; i < groups.size(); ++i)
					distance += std::abs(groups[i] -
						static_cast<double>(pattern.first[i]) * profile.picks / profile.drawSize);
				decadeDistance += distance * pattern.second / profile.rows.size();
			}
		const double decadeScore = 1.0 - (std::min)(1.0, decadeDistance / (2 * profile.picks));
		if (options.historicalDecades) score += decadeScore;
		if (options.historicalNovelty)
		{
			const double balance = 1.0 - (std::abs(2.0 * even - profile.picks) +
				std::abs(2.0 * small - profile.picks)) / (2 * profile.picks);
			const double sumScore = profile.deviation > 0
				? 1.0 / (1.0 + difference / profile.deviation)
				: (difference < 1e-9 ? 1.0 : 0.0);
			const double primeScore = 1.0 - std::abs(static_cast<double>(primes) / profile.picks - profile.primeRatio);
			const double novelty = 1.0 - static_cast<double>(mostShared) / profile.picks;
			score += 0.40 * balance + 0.20 * sumScore + 0.15 * decadeScore +
				0.15 * primeScore + 0.10 * novelty;
		}
		if (options.antiCrowd)
		{
			std::vector<int> sorted(line);
			if (!profile.positional) std::sort(sorted.begin(), sorted.end());
			if (!profile.positional && profile.maximumNumber > 31 && sorted.back() <= 31) score -= 1;
			if (sorted.size() >= 3)
			{
				bool progression = true;
				const int step = sorted[1] - sorted[0];
				for (std::size_t i = 2; i < sorted.size(); ++i)
					if (sorted[i] - sorted[i - 1] != step) progression = false;
				if (progression) score -= 1;
			}
			if (!profile.positional && sorted.size() > 1 && sorted.front() / 10 == sorted.back() / 10)
				score -= 1;
		}
		return true;
	}

	StructuralResult GenerateHistoricalLine(const HistoricalProfile& profile,
		const StructuralOptions& options, std::mt19937& generator, std::vector<int>& line)
	{
		line.clear();
		if (profile.rows.empty()) return StructuralResult::InvalidOptions;
		double bestScore = -std::numeric_limits<double>::infinity();
		int accepted = 0;
		const bool rank = options.historicalDecades || options.historicalNovelty || options.antiCrowd;
		for (int attempt = 0; attempt < 1000 && accepted < (rank ? 64 : 1); ++attempt)
		{
			std::vector<int> candidate;
			if (profile.positional)
			{
				std::uniform_int_distribution<int> digit(0, profile.maximumNumber);
				for (int i = 0; i < profile.picks; ++i) candidate.push_back(digit(generator));
			}
			else
			{
				const StructuralResult result = GenerateStructuralLine(profile.maximumNumber,
					profile.picks, options, generator, candidate);
				if (result != StructuralResult::Success) return result;
			}
			double score = 0;
			if (!EvaluateHistoricalLine(profile, candidate, options, score)) continue;
			++accepted;
			if (score > bestScore) { bestScore = score; line = candidate; }
		}
		return line.empty() ? StructuralResult::SearchLimit : StructuralResult::Success;
	}

	bool BacktestFrequency(
		const std::vector<std::vector<int>>& rows, int maximumNumber,
		int drawSize, int picks, std::size_t window, std::size_t testDraws,
		std::uint32_t seed, BacktestResult& result)
	{
		result = BacktestResult();
		if (maximumNumber < 1 || drawSize < 1 || drawSize > maximumNumber ||
			picks < 1 || picks > maximumNumber || window == 0 || testDraws == 0 ||
			rows.size() <= window)
			return false;

		const std::size_t firstTest = (std::max)(window,
			rows.size() > testDraws ? rows.size() - testDraws : window);
		for (std::size_t i = firstTest - window; i < rows.size(); ++i)
		{
			if (rows[i].size() < static_cast<std::size_t>(drawSize))
			{
				result.invalidRow = i + 1;
				return false;
			}
			std::vector<bool> seen(maximumNumber + 1, false);
			for (int j = 0; j < drawSize; ++j)
			{
				const int n = rows[i][j];
				if (n < 1 || n > maximumNumber || seen[n])
				{
					result.invalidRow = i + 1;
					return false;
				}
				seen[n] = true;
			}
		}

		std::vector<int> counts(maximumNumber + 1, 0);
		for (std::size_t i = firstTest - window; i < firstTest; ++i)
			for (int j = 0; j < drawSize; ++j)
				++counts[rows[i][j]];

		std::mt19937 frequencyRng(seed);
		std::mt19937 randomRng(seed ^ 0x9e3779b9U);
		for (std::size_t i = firstTest; i < rows.size(); ++i)
		{
			std::vector<int> available(maximumNumber);
			std::iota(available.begin(), available.end(), 1);
			std::vector<double> weights;
			for (int n : available)
				weights.push_back(static_cast<double>(counts[n]));
			std::vector<int> prediction;
			for (int j = 0; j < picks; ++j)
			{
				if (std::accumulate(weights.begin(), weights.end(), 0.0) == 0.0)
					return false;
				std::discrete_distribution<std::size_t> distribution(weights.begin(), weights.end());
				const std::size_t selected = distribution(frequencyRng);
				prediction.push_back(available[selected]);
				weights[selected] = 0.0;
			}

			std::shuffle(available.begin(), available.end(), randomRng);
			for (int j = 0; j < drawSize; ++j)
			{
				const int actual = rows[i][j];
				if (std::find(prediction.begin(), prediction.end(), actual) != prediction.end())
					++result.frequencyMatches;
				if (std::find(available.begin(), available.begin() + picks, actual) != available.begin() + picks)
					++result.randomMatches;
			}
			++result.draws;
			for (int j = 0; j < drawSize; ++j)
			{
				--counts[rows[i - window][j]];
				++counts[rows[i][j]];
			}
		}
		return true;
	}

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

	void FindResultMatches(
		const std::wstring& game,
		const std::vector<int>& predictedMain,
		const std::vector<int>& predictedBonus,
		const std::vector<int>& actualMain,
		const std::vector<int>& actualBonus,
		std::vector<int>& mainMatches,
		std::vector<int>& bonusMatches)
	{
		mainMatches.clear();
		bonusMatches.clear();
		if (game == L"Jokeri")
			FindPositionalMatches(predictedMain, actualMain, mainMatches);
		else
			FindMatches(predictedMain, actualMain, mainMatches);
		FindMatches(game == L"Suomen Lotto" || game == L"Milli" ? predictedMain : predictedBonus,
			actualBonus, bonusMatches);
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

	static std::wstring FormatSeparatedMatches(
		const std::vector<int>& mainMatches,
		const std::vector<int>& bonusMatches,
		bool finnish,
		bool stars)
	{
		std::wostringstream output;
		output << mainMatches.size() << L" + " << bonusMatches.size()
			<< (finnish ? L" oikein (osumat: " : L" correct (matches: ");
		for (size_t index = 0; index < mainMatches.size(); ++index)
		{
			if (index > 0) output << L", ";
			output << mainMatches[index];
		}
		if (mainMatches.empty()) output << (finnish ? L"ei osumia" : L"none");
		output << (finnish ? L" (päänumerot); " : L" (main numbers); ");
		for (size_t index = 0; index < bonusMatches.size(); ++index)
		{
			if (index > 0) output << L", ";
			output << bonusMatches[index];
		}
		if (bonusMatches.empty()) output << (finnish ? L"ei osumia" : L"none");
		output << (stars
			? (finnish ? L" (tähtinumerot))" : L" (star numbers))")
			: (finnish ? L" (lisänumero))" : L" (bonus number))"));
		return output.str();
	}

	std::wstring FormatEurojackpotMatches(
		const std::vector<int>& mainMatches, const std::vector<int>& starMatches, bool finnish)
	{
		return FormatSeparatedMatches(mainMatches, starMatches, finnish, true);
	}

	std::wstring FormatFinnishLottoMatches(
		const std::vector<int>& mainMatches, const std::vector<int>& bonusMatches, bool finnish)
	{
		return FormatSeparatedMatches(mainMatches, bonusMatches, finnish, false);
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
			return (mainMatches >= 4 || (mainMatches == 3 && bonusMatches == 1))
				? LotteryResult::Prize : LotteryResult::NoPrize;
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
