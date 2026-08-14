enum class SDD1Revision {
	None,
	SDD1
};

class SDD1 {
public:
	SDD1() { }

private:
	SDD1Revision revision = SDD1Revision::None;
};