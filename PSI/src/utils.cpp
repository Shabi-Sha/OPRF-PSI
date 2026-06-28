#pragma once

#include "utils.h"

#include <fstream>
#include <iostream>
#include <utility>

namespace PSI {

	std::vector<u8> fromBlock(const block &block) {
		u8* start = (u8*) &block;
		return std::vector<u8>(start, start + sizeof(block));
	}

	std::vector<u8> fromU64(const u64 &u) {
		u8* start = (u8*) &u;
		return std::vector<u8>(start, start + sizeof(u64));
	}

	u64 toU64(const u8 *data) {
		return *((u64*)data);
	}

	void paddingToBlock(std::vector<u8> &data) {
		u64 more = (sizeof(block) - data.size() % sizeof(block)) % sizeof(block);
		for (auto i = 0; i < more; ++i) {
			data.push_back(0);
		}
	}


	// --- Benchmark-Instrumentierung ---------------------------------------------

	Instrument::Instrument(Channel &ch, std::string party, std::string outPrefix)
		: mCh(ch),
		  mParty(std::move(party)),
		  mOutPrefix(std::move(outPrefix)),
		  mLast(std::chrono::steady_clock::now()),
		  mPrevSent((long long) ch.getTotalDataSent()),
		  mPrevRecv((long long) ch.getTotalDataRecv()) {}

	void Instrument::mark(const std::string &round) {
		auto now = std::chrono::steady_clock::now();
		double ms = std::chrono::duration<double, std::milli>(now - mLast).count();
		long long sent = (long long) mCh.getTotalDataSent();
		long long recv = (long long) mCh.getTotalDataRecv();
		mRecords.push_back(PhaseRecord{round, ms, sent - mPrevSent, recv - mPrevRecv});
		mLast = now;
		mPrevSent = sent;
		mPrevRecv = recv;
	}

	void Instrument::dump(const u64 &senderSize, const u64 &receiverSize, const u64 &width,
	                      const u64 &logHeight, const u64 &hashLengthInBytes) {
		std::string path = mOutPrefix + "_" + mParty + ".csv";
		bool exists = std::ifstream(path).good();
		std::ofstream f(path, std::ios::app);
		if (!f) {
			std::cerr << "[instrument] konnte CSV nicht oeffnen: " << path << "\n";
			return;
		}
		if (!exists) {
			f << "party,round,wall_ms,bytes_sent,bytes_recv,"
			     "sender_size,receiver_size,width,log_height,hash_bytes\n";
		}
		double totMs = 0;
		long long totSent = 0, totRecv = 0;
		for (const auto &r : mRecords) {
			f << mParty << "," << r.name << "," << r.wall_ms << ","
			  << r.bytes_sent << "," << r.bytes_recv << ","
			  << senderSize << "," << receiverSize << "," << width << ","
			  << logHeight << "," << hashLengthInBytes << "\n";
			totMs += r.wall_ms;
			totSent += r.bytes_sent;
			totRecv += r.bytes_recv;
		}
		f << mParty << ",TOTAL," << totMs << "," << totSent << "," << totRecv << ","
		  << senderSize << "," << receiverSize << "," << width << ","
		  << logHeight << "," << hashLengthInBytes << "\n";
		std::cout << "[instrument] geschrieben: " << path << "\n";
	}

}
