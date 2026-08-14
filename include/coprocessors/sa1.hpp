enum class SA1Revision {
	None,
	SA1
};

class SA1 {
public:
	SA1() { }

private:
	SA1Revision revision = SA1Revision::None;
};