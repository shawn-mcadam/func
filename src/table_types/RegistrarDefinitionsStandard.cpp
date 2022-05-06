// Sadly, c++17 is the first standard where we can use inline variables.
// So, the registrar needs all these static const definitions to be out of line.
#include "TableIncludes.hpp"
#include "config.hpp" // FUNC_USE_BOOST_AUTODIFF

#include "ConstantTaylorTable.hpp"
#include "LinearInterpolationTable.hpp"
#include "LinearPrecomputedInterpolationTable.hpp"
#include "QuadraticPrecomputedInterpolationTable.hpp"
#include "CubicPrecomputedInterpolationTable.hpp"

FUNC_REGISTER_EACH_ULUT_IMPL(UniformLinearInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformLinearPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformQuadraticPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformCubicPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformConstantTaylorTable);

#ifdef FUNC_USE_BOOST_AUTODIFF
#include "LinearTaylorTable.hpp"
#include "QuadraticTaylorTable.hpp"
#include "CubicTaylorTable.hpp"
#include "CubicHermiteTable.hpp"

FUNC_REGISTER_EACH_ULUT_IMPL(UniformLinearTaylorTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformQuadraticTaylorTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformCubicTaylorTable);
FUNC_REGISTER_EACH_ULUT_IMPL(UniformCubicHermiteTable);

FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformLinearInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformPseudoLinearInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformLinearPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformPseudoLinearPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformQuadraticPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformPseudoQuadraticPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformCubicPrecomputedInterpolationTable);
FUNC_REGISTER_EACH_ULUT_IMPL(NonUniformPseudoCubicPrecomputedInterpolationTable);

#endif