// PokerProbability.cpp
// Nathan Damon
// 2023/10/28
// A file that will calculate three card poker probabilities

#include <vector>
using std::vector;
#include <algorithm>
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <sstream>
#include <iomanip>
using std::setw;
using std::left;

// Poker probability class
class PokerProbability
{
public:
	enum class Suit
	{
		Clubs,
		Spades,
		Hearts,
		Diamonds
	};
	enum class Rank
	{
		_Ace,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		_10,
		_Jack,
		_Queen,
		_King
	};
	enum class Hand
	{
		Royal_FLush,		// AKQ (in any suit)
		Straight_Flush,		// 3 suited in sequence
		Three_Aces,			// 3 Aces (any combo of suits)
		Three_of_a_Kind,	// 3 of the same rank
		Straight,			// 3 in sequence (includes AKQ)
		Flush,				// 3 suited
		Pair,				// 2 of the same rank
		High_Card,          // None of the above
		EmptyHand			// No cards held
	};

	struct Card
	{
		Suit _suit;
		Rank _rank;

		bool operator == (const Card& a)
		{
			if (a._suit == _suit && a._rank == _rank)
				return true;
			return false;
		}
	};
	struct Stat
	{
		Hand _hand = Hand::EmptyHand;
		string _description = "";
		int _frequency = 0;
		float _probability = 0.0f;
		int _payout = 0;
		float _expectedPayout = 0.0f;

		static string formatMoney(float dollars)
		{
			std::ostringstream out;
			out << '$' << std::to_string(dollars);

			return out.str();
		}
		string getExpectedPayoutString()
		{
			return formatMoney(_expectedPayout);
		}
		string getFormatted(bool includeProbabilityAndFrequency = true)
		{
			std::ostringstream out;
			out << left << setw(16) << getHandAsString(_hand);
			out << left << setw(30) << _description;
			if (includeProbabilityAndFrequency)
			{
				out << left << setw(7) << _frequency;
				out << std::right << setw(9) << std::setprecision(6) << std::fixed << _probability * 100.0f << "%  ";
			}
			out << " $" << left << setw(7) << _payout;
			out << left << setw(5) << getExpectedPayoutString();
			out << endl;

			return out.str();
		}
	};
	struct DiscardAndReturn
	{
		vector<Card> _discardedCards;
		float _expectedReturn = 0.0f;

		DiscardAndReturn()
		{
			_expectedReturn = 0.0f;
		}
		DiscardAndReturn(vector<Card> discardedCards, float expectedReturn)
		{
			_discardedCards = discardedCards;
			_expectedReturn = expectedReturn;
		}
		void printData()
		{
			printCards(_discardedCards, false, 20);
			cout << " E[x]: " << _expectedReturn << endl;
		}
	};
	struct HandTable
	{
		vector<vector<Stat>> _handStats;

		void setup()
		{
			for (auto& column : _handStats)
				setupStatistics(column);
		}
		void resetTable() {
			for(auto& column : _handStats) // Each column is for a different hold
				for (auto& stat : column) // Each stat is a different hand type
				{
					stat._frequency = 0;
					stat._probability = 0.0f;
					stat._expectedPayout = 0.0f;
				}
		}

		void addData(Hand hand, int column)
		{
			_handStats[column][static_cast<int>(hand)]._frequency++;
		}
		vector<Stat>& getColumn(int index)
		{
			return _handStats[index];
		}
		// Adds up the frequencies and then sets the probability and expected payout for each hand
		void finalizeData()
		{
			int tempCounter = 0;
			for (auto& column : _handStats)
			{
				for (auto& stat : column)
					tempCounter += stat._frequency;
				for (auto& stat : column)
				{
					stat._probability = float(stat._frequency) / float(tempCounter);
					stat._expectedPayout = float(stat._payout) * stat._probability;
				}
				tempCounter = 0;
			}
		}
		// Returns an index to the best column and its expected return
		std::pair<int, float> getBestHoldColumnAndExpectedReturn()
		{
			int bestIndex = 0;
			float bestExpectedReturn = 0.0f;
			for (size_t i = 0; i < _handStats.size(); i++)
			{
				float expectedReturn = 0.0f;
				for (auto& stat : _handStats[i])
					expectedReturn += stat._expectedPayout;
				if (expectedReturn > bestExpectedReturn)
				{
					bestExpectedReturn = expectedReturn;
					bestIndex = i;
				}
			}
			return std::make_pair(bestIndex, bestExpectedReturn);
		}
	};
public:
	PokerProbability()
	{
		_deck = getDeck();
	}
	~PokerProbability()
	{

	}

	// Hand checking (Returns the hand that the given cards make)
	static Hand checkHand(vector<Card>& cards)
	{
		if (cards.empty())
			return Hand::EmptyHand;

		bool flagStraight = isStraight(cards);
		bool flagFlush = isFlush(cards);

		if (flagStraight && flagFlush)
		{
			if (isAKQ(cards))
				return Hand::Royal_FLush;
			else
				return Hand::Straight_Flush;
		}

		if (isThreeAces(cards))
			return Hand::Three_Aces;

		if (flagStraight)
			return Hand::Straight;
		if (flagFlush)
			return Hand::Flush;

		switch (isMultiOfAKind(cards))
		{
		default:
		case 1:
			break;
		case 2:
			return Hand::Pair;
		case 3:
			return Hand::Three_of_a_Kind;
		}

		if(isHighCard(cards))
			return Hand::High_Card;	

		return defaultHand();
	}

	// Returns true when given cards are Ace, King, and Queen (order does not matter)
	static bool isRoyalFlush(vector<Card> cards, bool preFlushChecked = false)
	{
		if (cards.empty())
			return false;

		sortCards(cards, true);
		return (preFlushChecked || isFlush(cards)) && isAKQ(cards);
	}
	static bool isAKQ(vector<Card>& cards)
	{
		int rankSum = 0;
		bool acePresent = false;
		for (auto& card : cards)
		{
			if (card._rank == Rank::_Ace)
				acePresent = true;
			rankSum += static_cast<int>(card._rank);
		}
		
		return acePresent && rankSum == 23;
	}
	// Returns true when given cards consist of 3 aces
	static bool isThreeAces(vector<Card>& cards)
	{
		if (cards.empty())
			return false;

		for (size_t i = 0; i < cards.size(); i++)
		{
			if (cards[i]._rank != Rank::_Ace)
				return false;
		}
		return true;
	}
	// Multiple cards of the same rank (0/1 = no, 2 = pair, 3 = three of a kind)
	// Returns the number of cards with the same rank
	static int isMultiOfAKind(vector<Card>& cards)
	{
		if (cards[0]._rank == cards[1]._rank)
		{
			if (cards[0]._rank == cards[2]._rank)
				return 3;
			else
				return 2;
		}
		if (cards[1]._rank == cards[2]._rank)
			return 2;
		if (cards[0]._rank == cards[2]._rank)
			return 2;
		return 1;
	}
	// Returns true when given cards make a sequence
	static bool isStraightDeprecated(vector<Card> cards)
	{
		if (cards.empty())
			return false;

		sortCards(cards, true);
		
		// Wrap check
		bool wraps = false;
		if (cards[0]._rank == Rank::_Ace && cards.back()._rank == Rank::_King)
			wraps = true;

		auto previousVal = static_cast<int>(cards[0]._rank);
		for (size_t i = 1; i < cards.size(); i++)
		{
			auto currentVal = static_cast<int>(cards[i]._rank);

			// Don't include KA2
			if (wraps && currentVal == static_cast<int>(Rank::_2))
				return false;

			// Cards are in order ascending 1 rank at a time
			if (previousVal != currentVal - 1)
			{
				// Wrap arround (check again, but only once)
				if (wraps)
				{
					previousVal = currentVal;
					wraps = false;
					continue;
				}
				
				return false;
			}
			previousVal = currentVal;
		}

		return true;
	}
	static bool isStraight(vector<Card>& cards)
	{
		if (cards.empty())
			return false;

		int card1 = static_cast<int>(cards[0]._rank);
		int card2 = static_cast<int>(cards[1]._rank);
		int card3 = static_cast<int>(cards[2]._rank);

		// Check if AKQ
		if (isAKQ(cards))
			return true;

		switch (2 + card1 - card2)
		{
		case 0:
			return card3 == card1 + 1;
		case 1:
			return card3 == card1 + 2 || card3 == card1 - 1;
		case 3:
			return card3 == card1 + 1 || card3 == card1 - 2;
		case 4:
			return card3 == card1 - 1;

		default:
			return false;
		}

		return true;
	}
	// Returns true when given cards are of the same suit
	static bool isFlush(vector<Card>& cards)
	{
		if (cards.empty())
			return false;

		auto flushSuit = cards[0]._suit;
		for (size_t i = 1; i < cards.size(); i++)
		{
			if (cards[i]._suit != flushSuit)
				return false;
		}
		return true;
	}
	// Returns true when called but can be changed later if need be
	static bool isHighCard(vector<Card>& cards)
	{
		return true;
	}
	// Returns the default hand if no other hand is made
	static Hand defaultHand()
	{
		return Hand::High_Card;
	}

	void generateCardCombinations(int handSize = 3)
	{
		if (handSize < 1 || handSize > 52)
		{
			cout << "Invalid input for generateCardCombinations(). Input must be between 1 and 52 inclusive. Given input was " << handSize << endl;
			return;
		}

		_allCardCombinations = runDC_Combinations(handSize);
	}
	void generateStatisticsNoDraws()
	{
		setupStatistics(_statistics);

		// Check all hands and get frequencies
		for (auto& hand : _allCardCombinations)
			_statistics[static_cast<int>(checkHand(hand))]._frequency++;


		// #######################################
		// Finalize statistics
		// #######################################
		
		auto computeStat = [&](Hand hand, string description) {
			Stat& statRef = _statistics[static_cast<int>(hand)];
			statRef._description = description;

			statRef._probability = float(statRef._frequency) / float(_allCardCombinations.size());			
			statRef._expectedPayout = float(statRef._payout) * statRef._probability;
			};

		computeStat(Hand::Royal_FLush, "AKQ (in any suit)");
		computeStat(Hand::Straight_Flush, "3 suited in sequence");
		computeStat(Hand::Three_Aces, "3 Aces (any combo of suits)");
		computeStat(Hand::Three_of_a_Kind, "3 of the same rank");
		computeStat(Hand::Straight, "3 in sequence (includes AKQ)");
		computeStat(Hand::Flush, "3 suited");
		computeStat(Hand::Pair, "2 of the same rank");
		computeStat(Hand::High_Card, "None of the above");
	}
	void generateStatisticsWithDraws(bool storeLast4InterestingHands = false)
	{		
		setupStatistics(_statistics);

		cout << "Adjust generateStatisticsWithDraws code" << endl;
		//cout << "Generating statistics...";
		//int incrementOfInform = 1; // Percent increment dispayed while running
		//int countPerIncrementOfInform = _allCardCombinations.size() / (100 / incrementOfInform);
		//int currentPercent = 0;
		//cout << "Count per percent: " << countPerIncrementOfInform << endl;

		//// Check all hands and add expected values
		//for (size_t i = 0; i < _allCardCombinations.size(); i++)
		//{
		//	// Display percent progress
		//	if (i > countPerIncrementOfInform * currentPercent)
		//	{
		//		cout << incrementOfInform * currentPercent << "% ";
		//		currentPercent++;
		//	}
		//	//cout << i << " ";

		//	auto tempValue = getOptimalExpectedValueOfDraws(_allCardCombinations[i], storeLast4InterestingHands);
		//	auto tempHand = checkHand(_allCardCombinations[i]);

		//	_statistics[static_cast<int>(tempHand)]._expectedPayout += tempValue;
		//	_statistics[static_cast<int>(tempHand)]._frequency++;
		//}
		//cout << "100% \nComplete" << endl;

		//
		//// #######################################
		//// Finalize statistics
		//// #######################################

		//// Adds the description and payout
		//auto setupStat = [&](Hand hand, string description) {
		//	Stat& statRef = _statistics[static_cast<int>(hand)];
		//	statRef._description = description;

		//	if(statRef._frequency != 0)
		//		statRef._expectedPayout /= float(_allCardCombinations.size());
		//	};

		//setupStat(Hand::Royal_FLush, "AKQ (in any suit)");
		//setupStat(Hand::Straight_Flush, "3 suited in sequence");
		//setupStat(Hand::Three_Aces, "3 Aces (any combo of suits)");
		//setupStat(Hand::Three_of_a_Kind, "3 of the same rank");
		//setupStat(Hand::Straight, "3 in sequence (includes AKQ)");
		//setupStat(Hand::Flush, "3 suited");
		//setupStat(Hand::Pair, "2 of the same rank");
		//setupStat(Hand::High_Card, "None of the above");
	}

	void printTable(bool includeProbabilityAndFrequency = true)
	{
		std::ostringstream out;

		// Categories
		out << left << setw(16) << "Hand";
		out << left << setw(30) << "Description";
		if (includeProbabilityAndFrequency)
		{
			out << left << setw(7) << "Freq";
			out << left << setw(13) << "Probability";
		}
		out << left << setw(8) << "Payout";
		out << left << setw(9) << "Return";
		out << endl;

		int tableBorderSize = 83;
		if (includeProbabilityAndFrequency == false)
			tableBorderSize = 83 - 20;

		// Border
		out << setw(tableBorderSize) << std::setfill('-') << "-" << endl;

		float totalReturnInDollars = 0.0f;
		for (auto& stat : _statistics)
		{
			out << stat.getFormatted(includeProbabilityAndFrequency);
			totalReturnInDollars += stat._expectedPayout;
		}

		// Border
		out << setw(tableBorderSize) << std::setfill('-') << "-" << endl;

		// Return
		out << std::setfill(' ');
		out << std::right << setw(tableBorderSize - 9) << "Total Return: ";
		out << left << setw(7) << Stat::formatMoney(totalReturnInDollars);

		// Print to console
		cout << out.str();
	}

	static void setupStatistics(vector<Stat>& stats)
	{
		// Clear and set
		stats = vector<Stat>(static_cast<int>(Hand::EmptyHand));

		for (size_t i = 0; i < stats.size(); i++)
		{
			stats[i]._hand = static_cast<Hand>(i);
			stats[i]._frequency = 0;
			stats[i]._payout = getHandPayout(static_cast<Hand>(i));
		}
	}
	vector<DiscardAndReturn> getExpectedValuesOfDrawsNoTable(vector<Card>& heldCards)
	{
		//vector<DiscardAndReturn> expectedReturns;
		//// Draw 0
		//expectedReturns.push_back(DiscardAndReturn({}, getHandPayout(checkHand(heldCards))));

		//// Get cards in deck minus the ones that are held
		//auto remainingCards = _deck;
		//for (auto& heldCard : heldCards)
		//	remainingCards.erase(std::find(remainingCards.begin(), remainingCards.end(), heldCard));

		//// Draw 1
		//{
		//	Card droppedCard;
		//	for (size_t i = 0; i < heldCards.size(); i++)
		//	{
		//		droppedCard = heldCards[i];
		//		auto changedHand = heldCards;
		//		changedHand.erase(std::find(changedHand.begin(), changedHand.end(), droppedCard));

		//		int sumOfPayouts = 0;

		//		for (size_t j = 0; j < remainingCards.size(); j++)
		//		{
		//			// Check for removed cards
		//			if (remainingCards[j] == droppedCard)
		//				continue;

		//			changedHand.push_back(remainingCards[j]);
		//			sumOfPayouts += getHandPayout(checkHand(changedHand));
		//			changedHand.pop_back();
		//		}

		//		expectedReturns.push_back(DiscardAndReturn({ droppedCard }, float(sumOfPayouts) / float(remainingCards.size())));
		//	}
		//}

		//// Draw 2
		//{
		//	Card droppedCard1;
		//	Card droppedCard2;
		//	for (size_t i = 0; i < heldCards.size(); i++)
		//	{
		//		droppedCard1 = heldCards[i];
		//		droppedCard2 = heldCards[(i + 1) % heldCards.size()];

		//		vector<Card> changedHand = { heldCards[(i + 2) % heldCards.size()]};
		//		changedHand.resize(3);

		//		int sumOfPayouts = 0;
		//		int handCount = 0;
		//		DC_CombinationsPayoutSum(remainingCards, changedHand, 2, sumOfPayouts, handCount);

		//		float expectedValue = float(sumOfPayouts) / float(handCount);
		//		expectedReturns.push_back(DiscardAndReturn({ droppedCard1, droppedCard2 }, expectedValue));
		//	}
		//}

		//// Draw 3
		//{
		//	vector<Card> emptyHand(3);
		//	int sumOfPayouts = 0;
		//	int handCount = 0;
		//	DC_CombinationsPayoutSum(remainingCards, emptyHand, 3, sumOfPayouts, handCount);

		//	float expectedValue = float(sumOfPayouts) / float(handCount);
		//	expectedReturns.push_back(DiscardAndReturn(heldCards, expectedValue));
		//}		

		//return expectedReturns;
	}
	vector<vector<Card>> getHoldsAndTableOfDraws(vector<Card>& heldCards)
	{
		_handStatsTable.resetTable();
		vector<vector<Card>> discardedCards;
		// Draw 0
		auto hand = checkHand(heldCards);
		discardedCards.push_back(vector<Card>());
		_handStatsTable.addData(hand, 0);

		// Get cards in deck minus the ones that are held
		auto remainingCards = _deck;
		for (auto& heldCard : heldCards)
			remainingCards.erase(std::find(remainingCards.begin(), remainingCards.end(), heldCard));

		// Draw 1
		{
			Card droppedCard;
			for (size_t i = 0; i < heldCards.size(); i++)
			{
				droppedCard = heldCards[i];
				auto changedHand = heldCards;
				changedHand.erase(std::find(changedHand.begin(), changedHand.end(), droppedCard));

				int sumOfPayouts = 0;

				for (size_t j = 0; j < remainingCards.size(); j++)
				{
					// Check for removed cards
					if (remainingCards[j] == droppedCard)
						continue;

					changedHand.push_back(remainingCards[j]);
					auto hand = checkHand(heldCards);
					sumOfPayouts += getHandPayout(hand);
					_handStatsTable.addData(hand, 1 + i);
					changedHand.pop_back();
				}

				discardedCards.push_back(vector<Card>({ droppedCard }));
			}
		}

		// Draw 2
		{
			Card droppedCard1;
			Card droppedCard2;
			for (size_t i = 0; i < heldCards.size(); i++)
			{
				droppedCard1 = heldCards[i];
				droppedCard2 = heldCards[(i + 1) % heldCards.size()];

				vector<Card> changedHand = { heldCards[(i + 2) % heldCards.size()] };
				changedHand.resize(3);

				int sumOfPayouts = 0;
				int handCount = 0;
				DC_CombinationsPayoutSum(remainingCards, changedHand, 2, sumOfPayouts, handCount, _handStatsTable.getColumn(4 + i));

				float expectedValue = float(sumOfPayouts) / float(handCount);
				discardedCards.push_back(vector<Card>({ droppedCard1, droppedCard2 }));
			}
		}

		// Draw 3
		{
			vector<Card> emptyHand(3);
			int sumOfPayouts = 0;
			int handCount = 0;
			DC_CombinationsPayoutSum(remainingCards, emptyHand, 3, sumOfPayouts, handCount, _handStatsTable.getColumn(7));

			float expectedValue = float(sumOfPayouts) / float(handCount);
			discardedCards.push_back(vector<Card>(heldCards));
		}

		_handStatsTable.finalizeData();
		return discardedCards;
	}
	float getOptimalExpectedValueOfDraws(vector<Card>& heldCards, bool storeLast4InterestingHands = false)
	{
		auto results = getExpectedValuesOfDrawsNoTable(heldCards);
		float expectedReturn = 0.0f;

		int bestIndex = 0;
		for (size_t i = 0; i < results.size(); i++)
		{
			if (results[i]._expectedReturn > expectedReturn)
			{
				expectedReturn = results[i]._expectedReturn;
				bestIndex = i;
			}
		}

		// Finds and stores 4 interesting hands
		if (storeLast4InterestingHands)
			pickBestAndWorsts(results, bestIndex, heldCards);

		return expectedReturn;
	}

	// The following functions are for finding interesting hands
	void pickBestAndWorsts(vector<DiscardAndReturn>& results, int bestIndex, vector<Card>& heldCards)
	{
		// Best single card drop
		if (bestIndex > 0 && bestIndex < 4)
		{
			if (_amongTheBestSingleCardDropHands._expectedReturn < results[bestIndex]._expectedReturn)
			{
				cout << "New best single card drop!\n";
				_amongTheBestSingleCardDropHands = DiscardAndReturn(heldCards, results[bestIndex]._expectedReturn);
			}
		}
		// Best double card drop
		if (bestIndex > 3 && bestIndex < 7)
		{
			if (_amongTheBestDoubleCardDropHands._expectedReturn < results[bestIndex]._expectedReturn)
			{
				cout << "New best double card drop!\n";
				_amongTheBestDoubleCardDropHands = DiscardAndReturn(heldCards, results[bestIndex]._expectedReturn);
			}
		}
		// Best and worst high card hands
		if (_amongTheWorstHands._expectedReturn > results[bestIndex]._expectedReturn)
		{
			cout << "New worst hand!\n";
			_amongTheWorstHands = DiscardAndReturn(heldCards, results[bestIndex]._expectedReturn);
		}
		if (checkHand(heldCards) == Hand::High_Card)
		{
			if (_amongTheBestHighCardHands._expectedReturn < results[bestIndex]._expectedReturn)
			{
				cout << "New best high card hand!\n";
				_amongTheBestHighCardHands = DiscardAndReturn(heldCards, results[bestIndex]._expectedReturn);
			}
		}
	}
	void findAndPrintTheLastForInterestingHandsInCopyableCode()
	{
		cout << "Finding last 4 interesting 3 card poker hands...\n";

		// Looking for the worst, so expectedReturn should start at the highest value
		_amongTheWorstHands._expectedReturn = getHandPayout(static_cast<Hand>(0));

		generateCardCombinations();
		generateStatisticsWithDraws(true);

		auto codePrintout = [](DiscardAndReturn& interestingHand) {
			cout << "auto cards = vector<Card>();\n";
			for (auto& card : interestingHand._discardedCards)
			{
				cout << "cards.push_back({ Suit::" << getSuitAsString(card._suit, false) << ", Rank::_" << getRankAsString(card._rank) << " });\n";
			}
			cout << "hands.push_back(cards);\n";
			cout << "cout << \"Expected Return of: $" << interestingHand._expectedReturn << "\" << endl;\n" << endl;
			};

		codePrintout(_amongTheBestSingleCardDropHands);
		codePrintout(_amongTheBestDoubleCardDropHands);
		codePrintout(_amongTheBestHighCardHands);
		codePrintout(_amongTheWorstHands);
	}
	
	// Utility functions ####################

	static int getHandPayout(Hand hand)
	{
		switch (hand)
		{
		case PokerProbability::Hand::Royal_FLush:
			return 250;
		case PokerProbability::Hand::Straight_Flush:
			return 100;
		case PokerProbability::Hand::Three_Aces:
			return 100;
		case PokerProbability::Hand::Three_of_a_Kind:
			return 30;
		case PokerProbability::Hand::Straight:
			return 15;
		case PokerProbability::Hand::Flush:
			return 5;
		case PokerProbability::Hand::Pair:
			return 1;
		case PokerProbability::Hand::High_Card:
		case PokerProbability::Hand::EmptyHand:
		default:
			return 0;
		}
	}
	static vector<Card> getDeck()
	{
		auto deck = vector<Card>(52);
		for (int i = 0; i < 52; i++)
		{
			deck[i]._rank = static_cast<Rank>(i % 13);
			deck[i]._suit = static_cast<Suit>(i / 13);
		}
		return deck;
	}
	static void sortCards(vector<Card>& cards, bool sortByRankOnly)
	{
		std::sort(cards.begin(), cards.end(), [&](Card& a, Card& b) {
			if(sortByRankOnly)
				return a._rank < b._rank;
			else
				return a._rank <= b._rank && a._suit <= b._suit;
			});
	}
	static string getHandAsString(Hand hand)
	{
		switch (hand)
		{
		case PokerProbability::Hand::Royal_FLush:
			return "Royal_FLush";
		case PokerProbability::Hand::Straight_Flush:
			return "Straight_Flush";
		case PokerProbability::Hand::Three_Aces:
			return "Three_Aces";
		case PokerProbability::Hand::Three_of_a_Kind:
			return "Three_of_a_Kind";
		case PokerProbability::Hand::Straight:
			return "Straight";
		case PokerProbability::Hand::Flush:
			return "Flush";
		case PokerProbability::Hand::Pair:
			return "Pair";
		case PokerProbability::Hand::High_Card:
			return "High_Card";
		case PokerProbability::Hand::EmptyHand:
			return "EmptyHand";
		default:
			return "Error in getHandAsString";
		}
	}
	static string getSuitAsString(Suit suit, bool shortened = true)
	{
		if (shortened)
		{
			switch (suit)
			{
			case PokerProbability::Suit::Clubs:
				return "C_";
			case PokerProbability::Suit::Spades:
				return "S_";
			case PokerProbability::Suit::Hearts:
				return "H_";
			case PokerProbability::Suit::Diamonds:
				return "D_";
			default:
				return "Error in getCardString";
			}
		}
		else
		{
			switch (suit)
			{
			case PokerProbability::Suit::Clubs:
				return "Clubs";
			case PokerProbability::Suit::Spades:
				return "Spades";
			case PokerProbability::Suit::Hearts:
				return "Hearts";
			case PokerProbability::Suit::Diamonds:
				return "Diamonds";
			default:
				return "Error in getCardString";
			}
		}
	}
	static string getRankAsString(Rank rank)
	{
		switch (rank)
		{
		case PokerProbability::Rank::_Ace:
			return "A";
		case PokerProbability::Rank::_2:
		case PokerProbability::Rank::_3:
		case PokerProbability::Rank::_4:
		case PokerProbability::Rank::_5:
		case PokerProbability::Rank::_6:
		case PokerProbability::Rank::_7:
		case PokerProbability::Rank::_8:
		case PokerProbability::Rank::_9:
		case PokerProbability::Rank::_10:
			return std::to_string(static_cast<int>(rank) + 1);
		case PokerProbability::Rank::_Jack:
			return "J";
		case PokerProbability::Rank::_Queen:
			return "Q";
		case PokerProbability::Rank::_King:
			return "K";
		default:
			return "Error in getCardString";
		}
	}
	static string getCardString(const Card& card)
	{
		string cardStr = "";
		cardStr += getSuitAsString(card._suit);
		cardStr += getRankAsString(card._rank);
		
		return cardStr;
	}
	static void printCards(vector<Card>& cards, bool newLineAtEnd = true, int setCardAreaSize = 0, std::ostream& os = cout)
	{
		std::ostringstream out;
		out << "{ ";
		for (auto& card : cards)
			out << getCardString(card) << " ";
		out << "}";

		if (newLineAtEnd)
			out << endl;

		if (setCardAreaSize > 0)
			os << left << setw(setCardAreaSize) << out.str();
		else
			os << out.str();
	}

	void tenInterestingHands()
	{
		auto hands = std::vector<std::vector<Card>>();

		// Hand 1: AKQ all diamonds { D_A D_K D_Q } One of 4 best hands. Hold it
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Diamonds, Rank::_Ace });
			cards.push_back({ Suit::Diamonds, Rank::_King });
			cards.push_back({ Suit::Diamonds, Rank::_Queen });
			hands.push_back(cards);
		}
		// Hand 2: { D_A S_2 C_4 } Going for straight by dropping either Ace or 4
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Diamonds, Rank::_Ace });
			cards.push_back({ Suit::Spades, Rank::_2 });
			cards.push_back({ Suit::Clubs, Rank::_4});
			hands.push_back(cards);
		}
		// Hand 3: { D_A S_3 C_4 } Going for straight
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Diamonds, Rank::_Ace });
			cards.push_back({ Suit::Spades, Rank::_3 });
			cards.push_back({ Suit::Clubs, Rank::_4 });
			hands.push_back(cards);
		}
		// Hand 4: { D_K S_3 D_A } Going for straight or flush while hoping for royal flush
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Diamonds, Rank::_King });
			cards.push_back({ Suit::Spades, Rank::_3 });
			cards.push_back({ Suit::Diamonds, Rank::_Ace });
			hands.push_back(cards);
		}
		// Hand 5: { D_2 S_8 C_J } Bad hand, going for anything else
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Diamonds, Rank::_2 });
			cards.push_back({ Suit::Spades, Rank::_8 });
			cards.push_back({ Suit::Clubs, Rank::_Jack });
			hands.push_back(cards);
		}
		// Hand 6: { H_7 H_2 H_4 } Hold for $5, or drop D_7 for $4 exactly
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Hearts, Rank::_7 });
			cards.push_back({ Suit::Hearts, Rank::_2 });
			cards.push_back({ Suit::Hearts, Rank::_4 });
			hands.push_back(cards);
		}
		// Hand 7: The best single card drop hand and is also among the best high card hands
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Clubs, Rank::_2 });
			cards.push_back({ Suit::Spades, Rank::_Queen });
			cards.push_back({ Suit::Spades, Rank::_King });
			hands.push_back(cards);
			//cout << "Expected Return of: $10.0204" << endl;
		}
		// Hand 8: One of the best double card drop hand
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Clubs, Rank::_2 });
			cards.push_back({ Suit::Clubs, Rank::_5 });
			cards.push_back({ Suit::Spades, Rank::_Queen });
			hands.push_back(cards);
			//cout << "Expected Return of: $1.47449" << endl;
		}
		// Hand 9: 
		{

		}
		// Hand 10: Among the worst hands
		{
			auto cards = vector<Card>();
			cards.push_back({ Suit::Clubs, Rank::_2 });
			cards.push_back({ Suit::Clubs, Rank::_5 });
			cards.push_back({ Suit::Spades, Rank::_King });
			hands.push_back(cards);
			cout << "Expected Return of: $1.30017" << endl;
		}

		// Print out the hands with expected returns in order of greatest to least
		for (size_t i = 5; i < hands.size(); i++)
		{
			cout << "Hand " << i + 1 << ": ";
			printCards(hands[i]);

			auto results = getExpectedValuesOfDrawsNoTable(hands[i]);
			std::sort(results.begin(), results.end(),
				[](const DiscardAndReturn& a, const DiscardAndReturn& b) {
					return a._expectedReturn > b._expectedReturn;
				});

			cout << endl;
			for (auto& result : results)
				result.printData();
			std::cout << endl << endl;
		}
	}

	// Testing
	void runTests()
	{
		// Deprecation testing
		/*generateCardCombinations();
		for (auto& hand : _allCardCombinations)
		{
			if (isStraight(hand) != isStraightDeprecated(hand))
				printCards(hand);
		}*/



		int testsFailed = 0;
		testsFailed += testRoyalFlush();
		testsFailed += testThreeAces();
		testsFailed += testMultiOfAKind();
		testsFailed += testStraight();
		testsFailed += testFlush();
		testsFailed += testCheckHand();

		if (testsFailed > 0)
			cout << "Tests failed: " << testsFailed << endl;
		else
			cout << "All tests passed" << endl;

		//debugPrintExpectedValuesOfDraws();
	}

private:
	int testRoyalFlush()
	{
		int testsFailed = 0;
		int testNum = 0;
		auto cardChecker = [&](bool result, bool check) {
			if (result != check)
			{
				cout << "TestRoyalFlush : SubTest #" << testNum << " [FAILED]" << endl;
				testsFailed++;
			}
			testNum++;
			};

		// Test case 0
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_2 });
			testCards.push_back({ Suit::Spades, Rank::_3 });

			auto retVal = isRoyalFlush(testCards);
			cardChecker(retVal, false);
		}
		// Test case 1
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_King });
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Diamonds, Rank::_Queen });

			auto retVal = isRoyalFlush(testCards);
			cardChecker(retVal, true);
		}
		// Test case 2
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_King });
			testCards.push_back({ Suit::Hearts, Rank::_Queen });
			testCards.push_back({ Suit::Hearts, Rank::_Queen });

			auto retVal = isRoyalFlush(testCards);
			cardChecker(retVal, false);
		}
		// Test case 3
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_Ace });
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_Ace });

			auto retVal = isRoyalFlush(testCards);
			cardChecker(retVal, false);
		}
		// Test case 4
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_King });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });
			testCards.push_back({ Suit::Clubs, Rank::_Ace });

			auto retVal = isRoyalFlush(testCards);
			cardChecker(retVal, true);
		}


		return testsFailed;
	}
	int testThreeAces()
	{
		int testsFailed = 0;
		int testNum = 0;
		auto cardChecker = [&](bool result, bool check) {
			if (result != check)
			{
				cout << "TestThreeAces : SubTest #" << testNum << " [FAILED]" << endl;
				testsFailed++;
			}
			testNum++;
			};

		// Test case 0
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_Ace });

			auto retVal = isThreeAces(testCards);
			cardChecker(retVal, true);
		}
		// Test case 1
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_King });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });

			auto retVal = isThreeAces(testCards);
			cardChecker(retVal, false);
		}
		// Test case 2
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_King });
			testCards.push_back({ Suit::Hearts, Rank::_9 });
			testCards.push_back({ Suit::Hearts, Rank::_2 });

			auto retVal = isThreeAces(testCards);
			cardChecker(retVal, false);
		}
		// Test case 3
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_Ace });
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_Ace });

			auto retVal = isThreeAces(testCards);
			cardChecker(retVal, true);
		}
		// Test case 4
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_7 });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });
			testCards.push_back({ Suit::Clubs, Rank::_Ace });

			auto retVal = isThreeAces(testCards);
			cardChecker(retVal, false);
		}


		return testsFailed;
	}
	int testMultiOfAKind()
	{
		int testsFailed = 0;
		int testNum = 0;
		auto cardChecker = [&](int result, int countCheck) {
			if (result != countCheck)
			{
				cout << "TestMultiOfAKind : SubTest #" << testNum << " [FAILED]" << endl;
				testsFailed++;
			}
			testNum++;
			};

		// Test case 0
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_2 });
			testCards.push_back({ Suit::Hearts, Rank::_3 });

			auto retVal = isMultiOfAKind(testCards);
			cardChecker(retVal, 1);
		}
		// Test case 1
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_2 });
			testCards.push_back({ Suit::Spades, Rank::_2 });
			testCards.push_back({ Suit::Hearts, Rank::_3 });

			auto retVal = isMultiOfAKind(testCards);
			cardChecker(retVal, 2);
		}
		// Test case 2
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_10 });
			testCards.push_back({ Suit::Spades, Rank::_10 });
			testCards.push_back({ Suit::Hearts, Rank::_10 });

			auto retVal = isMultiOfAKind(testCards);
			cardChecker(retVal, 3);
		}
		// Test case 3
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_7 });
			testCards.push_back({ Suit::Clubs, Rank::_3 });
			testCards.push_back({ Suit::Clubs, Rank::_9 });

			auto retVal = isMultiOfAKind(testCards);
			cardChecker(retVal, 1);
		}
		// Test case 3
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Spades, Rank::_9 });
			testCards.push_back({ Suit::Clubs, Rank::_3 });
			testCards.push_back({ Suit::Clubs, Rank::_9 });

			auto retVal = isMultiOfAKind(testCards);
			cardChecker(retVal, 2);
		}

		return testsFailed;
	}
	int testStraight()
	{
		int testsFailed = 0;
		int testNum = 0;
		auto cardChecker = [&](bool result, bool check) {
			if (result != check)
			{
				cout << "TestStraight : SubTest #" << testNum << " [FAILED]" << endl;
				testsFailed++;
			}
			testNum++;
			};

		// Test case 0
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_2 });
			testCards.push_back({ Suit::Clubs, Rank::_3 });

			auto retVal = isStraight(testCards);
			cardChecker(retVal, true);
		}
		// Test case 1
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_King });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });

			auto retVal = isStraight(testCards);
			cardChecker(retVal, true);
		}
		// Test case 2
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_7 });
			testCards.push_back({ Suit::Diamonds, Rank::_9 });
			testCards.push_back({ Suit::Spades, Rank::_8 });

			auto retVal = isStraight(testCards);
			cardChecker(retVal, true);
		}
		// Test case 3
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_Jack });
			testCards.push_back({ Suit::Diamonds, Rank::_9 });
			testCards.push_back({ Suit::Spades, Rank::_8 });

			auto retVal = isStraight(testCards);
			cardChecker(retVal, false);
		}
		// Test case 4
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_10 });
			testCards.push_back({ Suit::Clubs, Rank::_2 });
			testCards.push_back({ Suit::Clubs, Rank::_5 });

			auto retVal = isStraight(testCards);
			cardChecker(retVal, false);
		}
		// Test case 5
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_Jack });
			testCards.push_back({ Suit::Spades, Rank::_Queen });
			testCards.push_back({ Suit::Diamonds, Rank::_King });

			auto retVal = isStraight(testCards);
			cardChecker(retVal, true);
		}


		return testsFailed;
	}
	int testFlush()
	{
		int testsFailed = 0;
		int testNum = 0;
		auto cardChecker = [&](bool result, bool check) {
			if (result != check)
			{
				cout << "TestFlush : SubTest #" << testNum << " [FAILED]" << endl;
				testsFailed++;
			}
			testNum++;
			};

		// Test case 0
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_2 });
			testCards.push_back({ Suit::Spades, Rank::_3 });

			auto retVal = isFlush(testCards);
			cardChecker(retVal, true);
		}
		// Test case 1
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_King });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });

			auto retVal = isFlush(testCards);
			cardChecker(retVal, false);
		}
		// Test case 2
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_7 });
			testCards.push_back({ Suit::Hearts, Rank::_9 });
			testCards.push_back({ Suit::Hearts, Rank::_8 });

			auto retVal = isFlush(testCards);
			cardChecker(retVal, true);
		}
		// Test case 3
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_Jack });
			testCards.push_back({ Suit::Diamonds, Rank::_9 });
			testCards.push_back({ Suit::Spades, Rank::_8 });

			auto retVal = isFlush(testCards);
			cardChecker(retVal, false);
		}
		// Test case 4
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_10 });
			testCards.push_back({ Suit::Clubs, Rank::_2 });
			testCards.push_back({ Suit::Clubs, Rank::_5 });

			auto retVal = isFlush(testCards);
			cardChecker(retVal, true);
		}


		return testsFailed;
	}
	int testCheckHand()
	{
		int testsFailed = 0;
		int testNum = 0;
		auto cardChecker = [&](Hand result, Hand check, vector<Card>& cards) {
			if (result != check)
			{
				cout << "TestCheckHand : SubTest #" << testNum << " [FAILED]" << endl;
				printCards(cards);
				cout << "Categorized as: " << getHandAsString(result) << endl;
				cout << "Should have been: " << getHandAsString(check) << endl << endl;
				testsFailed++;
			}
			testNum++;
			};

		// Test case 0 Empty Hand
		{
			auto testCards = vector<Card>();

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::EmptyHand, testCards);
		}
		// Test case 1 Straight
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_King });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Straight, testCards);
		}
		// Test case 2 Straight
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_7 });
			testCards.push_back({ Suit::Clubs, Rank::_9 });
			testCards.push_back({ Suit::Hearts, Rank::_8 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Straight, testCards);
		}
		// Test case 3 Flush
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_Jack });
			testCards.push_back({ Suit::Hearts, Rank::_9 });
			testCards.push_back({ Suit::Hearts, Rank::_8 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Flush, testCards);
		}
		// Test case 4 Flush
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_10 });
			testCards.push_back({ Suit::Clubs, Rank::_2 });
			testCards.push_back({ Suit::Clubs, Rank::_5 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Flush, testCards);
		}
		// Test case 5 Straight_Flush
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_2 });
			testCards.push_back({ Suit::Clubs, Rank::_3 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Straight_Flush, testCards);
		}
		// Test case 6 Straight_Flush
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_10 });
			testCards.push_back({ Suit::Diamonds, Rank::_Jack });
			testCards.push_back({ Suit::Diamonds, Rank::_Queen });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Straight_Flush, testCards);
		}
		// Test case 7 Three_of_a_Kind
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_10 });
			testCards.push_back({ Suit::Hearts, Rank::_10 });
			testCards.push_back({ Suit::Diamonds, Rank::_10 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Three_of_a_Kind, testCards);
		}
		// Test case 8 Three_of_a_Kind
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Spades, Rank::_Queen });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Three_of_a_Kind, testCards);
		}
		// Test case 9 Pair
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Spades, Rank::_7 });
			testCards.push_back({ Suit::Diamonds, Rank::_7 });
			testCards.push_back({ Suit::Hearts, Rank::_9 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Pair, testCards);
		}
		// Test case 10 Pair
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_4 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Pair, testCards);
		}
		// Test case 11 High_Card
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_9 });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_4 });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::High_Card, testCards);
		}
		// Test case 12 High_Card
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_2 });
			testCards.push_back({ Suit::Spades, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::High_Card, testCards);
		}
		// Test case 13 Royal Flush
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_King });
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Diamonds, Rank::_Queen });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Royal_FLush, testCards);
		}
		// Test case 14 Royal Flush
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Clubs, Rank::_King });
			testCards.push_back({ Suit::Clubs, Rank::_Queen });
			testCards.push_back({ Suit::Clubs, Rank::_Ace });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Royal_FLush, testCards);
		}
		// Test case 15 Three Aces
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Hearts, Rank::_Ace });
			testCards.push_back({ Suit::Clubs, Rank::_Ace });
			testCards.push_back({ Suit::Spades, Rank::_Ace });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Three_Aces, testCards);
		}
		// Test case 16 Three Aces
		{
			auto testCards = vector<Card>();
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });
			testCards.push_back({ Suit::Diamonds, Rank::_Ace });

			auto retVal = checkHand(testCards);
			cardChecker(retVal, Hand::Three_Aces, testCards);
		}

		return testsFailed;
	}

	void debugPrintExpectedValuesOfDraws()
	{
		auto testCards = vector<Card>();
		testCards.push_back({ Suit::Diamonds, Rank::_King });
		testCards.push_back({ Suit::Spades, Rank::_Ace });
		testCards.push_back({ Suit::Clubs, Rank::_Queen });
				
		auto results = getExpectedValuesOfDraws(testCards);
		for (auto& result : results)
			result.printData();
	}

	// Uses DC_Combinations to generate all combinations of cards as hands of size handSize
	vector<vector<Card>> runDC_Combinations(int handSize)
	{
		auto possibleHands = vector<vector<Card>>();

		auto currentHand = vector<Card>(handSize);
		DC_Combinations(possibleHands, _deck, currentHand, handSize);

		return possibleHands;
	}
	// Decrease and conquer combination algorithm
	static void DC_Combinations(vector<vector<Card>>& possibleHands, vector<Card>& deck, vector<Card>& currentHand, int handSize, int x = 0)
	{
		for (size_t i = x; i < deck.size() - handSize + 1; i++)
		{
			currentHand[currentHand.size() - handSize] = deck[i];
			if (handSize > 1)
				DC_Combinations(possibleHands, deck, currentHand, handSize - 1, i + 1);
			else
				possibleHands.push_back(vector<Card>(currentHand));
		}
	}
	static void DC_CombinationsPayoutSum(vector<Card>& deck, vector<Card>& currentHand, int handSize, int& payoutSum, int& handCount, vector<Stat>& stats, int x = 0)
	{
		for (size_t i = x; i < deck.size() - handSize + 1; i++)
		{
			currentHand[currentHand.size() - handSize] = deck[i];
			if (handSize > 1)
				DC_CombinationsPayoutSum(deck, currentHand, handSize - 1, payoutSum, handCount, stats, i + 1);
			else
			{
				auto hand = checkHand(currentHand);
				payoutSum += getHandPayout(hand);
				stats[static_cast<int>(hand)]._frequency++;
				handCount++;
			}
		}
	}

	// The following variables are for finding interesting hands
	DiscardAndReturn _amongTheBestSingleCardDropHands;
	DiscardAndReturn _amongTheBestDoubleCardDropHands;
	DiscardAndReturn _amongTheBestHighCardHands;
	DiscardAndReturn _amongTheWorstHands;

	vector<Card> _deck;
	vector<vector<Card>> _allCardCombinations;
	vector<DiscardAndReturn> _allCardCombinationsExpectedDrawValues;
	vector<Stat> _statistics;
	HandTable _handStatsTable;
};


int main()
{
	PokerProbability pokerP;

	bool runTests = true;
	bool perfectGame = false;

	if (runTests)
	{
		//pokerP.runTests();
		pokerP.tenInterestingHands();
		//pokerP.findAndPrintTheLastForInterestingHandsInCopyableCode();
	}
	else
	{
		pokerP.generateCardCombinations();

		if (perfectGame)
			pokerP.generateStatisticsWithDraws();
		else
			pokerP.generateStatisticsNoDraws();

		pokerP.printTable(!perfectGame);
	}

	return 0;
}