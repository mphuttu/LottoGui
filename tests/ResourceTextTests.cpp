#include <gtest/gtest.h>

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

TEST(ResourceText, FinnishDialogLabelsPreserveAccentedCharacters)
{
	std::ifstream resource(LOTTOGUI_RESOURCE_FILE, std::ios::binary);
	ASSERT_TRUE(resource.is_open());
	const std::string text((std::istreambuf_iterator<char>(resource)),
		std::istreambuf_iterator<char>());

	const std::vector<std::string> expected{
		"Yhdistelt" "\xc3\xa4" "v" "\xc3\xa4" "t rakennestrategiat",
		"Yksi lis" "\xc3\xa4" "riviryhm" "\xc3\xa4" "; kaikki valitut ehdot samoille riveille.",
		"Samaa p" "\xc3\xa4" "\xc3\xa4" "tenumeroa enint" "\xc3\xa4" "\xc3\xa4" "n kaksi",
		"Alkulukujen suhteellinen m" "\xc3\xa4" "\xc3\xa4" "r" "\xc3\xa4" " (+/- 1)",
		"Ei yli kolmen per" "\xc3\xa4" "kk" "\xc3\xa4" "isi" "\xc3\xa4" " sarjoja",
		"Minimiv" "\xc3\xa4" "li:"
	};
	for (const std::string& label : expected)
		EXPECT_NE(std::string::npos, text.find(label)) << label;

	const std::string backtest = "J" "\xc3\xa4" "lkitestaus: frekvenssi ja satunnainen (100)";
	std::size_t count = 0;
	for (std::size_t pos = text.find(backtest); pos != std::string::npos;
		pos = text.find(backtest, pos + backtest.size()))
		++count;
	EXPECT_EQ(6u, count);
}
