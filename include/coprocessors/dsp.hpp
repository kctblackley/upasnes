enum class DSPRevision {
	None,
	DSP1, // DSP1A is equivalent to DSP1
	DSP1B,
	DSP2,
	DSP3,
	DSP4,
	ST0010,
	ST0011,
	ST0018 // Uses ARM
};

class DSP {
public:
	DSP() { }

private:
	DSPRevision revision = DSPRevision::None;
};