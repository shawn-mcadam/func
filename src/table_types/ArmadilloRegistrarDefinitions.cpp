#include "UniformArmadilloPrecomputedInterpolationTable.hpp"
#include "config.hpp" // FUNC_USE_BOOST_AUTODIFF, FUNC_USE_ARMADILLO

#ifndef FUNC_USE_ARMADILLO
#error "ArmadilloRegistrarDefinitions needs Armadillo"
#endif

FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformArmadilloPrecomputedInterpolationTable,4);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformArmadilloPrecomputedInterpolationTable,5);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformArmadilloPrecomputedInterpolationTable,6);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformArmadilloPrecomputedInterpolationTable,7);

#ifdef FUNC_USE_BOOST_AUTODIFF
#include "UniformPadeTable.hpp"
//#include "NonUniformLookupTable.hpp"
//#include "NonUniformLinearInterpolationTable.hpp"
//#include "NonUniformPseudoLinearInterpolationTable.hpp"
//#include "NonUniformCubicPrecomputedInterpolationTable.hpp"
//#include "NonUniformPseudoCubicPrecomputedInterpolationTable.hpp"

FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,1,1);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,2,1);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,3,1);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,4,1);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,5,1);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,6,1);

FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,2,2);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,3,2);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,4,2);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,5,2);

FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,3,3);
FUNC_REGISTER_EACH_TEMPLATED_ULUT_IMPL(UniformPadeTable,4,3);

// Register a whole load of different options for the nonuniform LUTs
//FUNC_REGISTER_EACH_NONUNIFORM_IMPL_TYPE(NonUniformCubicPrecomputedInterpolationTable);
//FUNC_REGISTER_EACH_NONUNIFORM_IMPL_TYPE(NonUniformLinearInterpolationTable);
//FUNC_REGISTER_EACH_NONUNIFORM_IMPL_TYPE(NonUniformPseudoCubicPrecomputedInterpolationTable);
//FUNC_REGISTER_EACH_NONUNIFORM_IMPL_TYPE(NonUniformPseudoLinearInterpolationTable);
#endif
