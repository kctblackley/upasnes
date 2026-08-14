enum class SuperFXRevision {
	None,
	MARIO,
	GSU1,
	GSU2,
	GSU2SP1 // functionally-identical to GSU2, I'm just including detection of this revision for the sake of it
};

class SuperFX {
public:
	SuperFX() { }

private:
	SuperFXRevision revision = SuperFXRevision::None;
};