enum class DSPRevision {
	None,
	DSP1,
	DSP1B,
	DSP2,
	DSP3,
	DSP4
};

class DSP {
public:
	DSP() { }

private:
	DSPRevision revision = DSPRevision::None;
};