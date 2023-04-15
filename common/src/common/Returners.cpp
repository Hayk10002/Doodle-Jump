#include "Returners.hpp"
#include <level/Level.hpp>
#include <level/LevelGenerator.hpp>

template<ReturnType RT>
	requires (RT == Height)
GeneratedHeightReturner<RT>::ValT GeneratedHeightReturner<RT>::get() const
{
	return Generation::getCurrentLevelForGenerating()->level_generator.getGeneratedHeight();
}

void dummy()
{
	GeneratedHeightReturner<Height> ret;
}
