#pragma once
#include "cpu.hpp"

// Handler stores pointer to half-cycle instruction

// bool is for handling skipped 

template<typename CpuT>
using HandlerFn = void(*)(CpuT&, bool);

template<typename CpuT>
using PredicateFn = bool(*)(CpuT&);

template<typename CpuT>
struct Handler {
	HandlerFn<CpuT> function;
	PredicateFn<CpuT> predicate;
};

template<typename CpuT>
constexpr Handler<CpuT> MakeHandler(
    void(*fn)(CpuT&, bool),
    bool(*pred)(CpuT&) = nullptr)
{
    return {fn, pred};
}

template<typename CpuT>
using Instruction = std::vector<Handler<CpuT>>;

template<typename CpuT>
using Optable = std::array<Instruction<CpuT>*, 258>;

template<typename CpuT>
struct Opcode {
	HandlerFn<CpuT> function;
	CycleCount idx;
	bool skipped;
};

template<typename CpuT>
Opcode<CpuT> get_opcode(const Optable<CpuT>& optable, Word opcode, CycleCount& idx, CpuT& cpu) {
	Instruction<CpuT>& instruction = *optable[opcode];
	
	Handler<CpuT>* handler = nullptr;
	bool skipped = false;
	bool predicate = true;
	while(predicate && idx < instruction.size()) {
		handler = &instruction[idx++];
		predicate = handler->predicate ? handler->predicate(cpu) : false;
		if (predicate) {
			skipped = true;
		}
	}

	if (idx >= instruction.size() ) { idx = 0; }

	return Opcode{ handler->function, idx, skipped };
}