#pragma once

#include "Defines.h"

#include <vector>
#include <string>
#include <chrono>

namespace PSI {

	std::vector<u8> fromBlock(const block &block);

	std::vector<u8> fromU64(const u64 &u);

	u64 toU64(const u8 *data);

	void paddingToBlock(std::vector<u8> &data);

	void aesDecBlocks(AESDec &aesDec, const block* cyphertext, const u64 &blockLength, block* plaintext);


	// --- Benchmark-Instrumentierung ---------------------------------------------
	// Misst pro Runde (Phase) die Wall-Clock-Zeit und die ueber den Channel
	// gesendeten/empfangenen Bytes (Delta seit der letzten Marke). Am Ende wird
	// eine CSV pro Party geschrieben: <outPrefix>_<party>.csv
	//
	// Hinweis: wall_ms ist die Wall-Clock-Zeit der Runde; bei Runden, die auf den
	// Channel warten (z.B. ch.recv), steckt darin auch die Netzwerk-Wartezeit.

	struct PhaseRecord {
		std::string name;
		double wall_ms;        // Wall-Clock seit der vorigen Marke
		long long bytes_sent;  // gesendete Bytes in dieser Runde
		long long bytes_recv;  // empfangene Bytes in dieser Runde
	};

	class Instrument {
	public:
		Instrument(Channel &ch, std::string party, std::string outPrefix);

		// Schliesst die aktuelle Runde ab und merkt sich ihre Kennzahlen.
		void mark(const std::string &round);

		// Schreibt alle Runden + eine TOTAL-Zeile nach <outPrefix>_<party>.csv.
		void dump(const u64 &senderSize, const u64 &receiverSize, const u64 &width,
		          const u64 &logHeight, const u64 &hashLengthInBytes);

	private:
		Channel &mCh;
		std::string mParty;
		std::string mOutPrefix;
		std::chrono::steady_clock::time_point mLast;
		long long mPrevSent;
		long long mPrevRecv;
		std::vector<PhaseRecord> mRecords;
	};

}
