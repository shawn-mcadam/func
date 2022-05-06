/*
  Cubic Taylor LUT

  Usage example:
    CubicTaylorTable look(&function,0,10,0.0001);
    double val = look(0.87354);

  Notes:
  - static data after constructor has been called
  - evaluate by using parentheses, just like a function
*/
#pragma once
#include "MetaTable.hpp"
#include "config.hpp"

#ifndef FUNC_USE_BOOST_AUTODIFF
#error "CubicTaylorTable needs boost version >= 1.71"
#endif

template <typename TIN, typename TOUT=TIN, GridTypes GT=UNIFORM>
class CubicTaylorTable final : public MetaTable<TIN,TOUT,4,TAYLOR,GT>
{
  INHERIT_EVALUATION_IMPL(TIN,TOUT);
  INHERIT_LUT(TIN,TOUT);
  INHERIT_META(TIN,TOUT,4,TAYLOR,GT);
  FUNC_REGISTER_LUT(CubicTaylorTable);

  std::function<adVar<TOUT,3>(adVar<TOUT,3>)> mp_boost_func;

public:
  CubicTaylorTable(FunctionContainer<TIN,TOUT> *func_container, LookupTableParameters<TIN> par) :
    MetaTable<TIN,TOUT,4,TAYLOR,GT>(func_container, par)
  {
    using boost::math::differentiation::make_fvar;

    /* Base class default variables */
    m_name = grid_type_to_string<GT>() + "CubicTaylorTable";
    m_order = 4;
    m_numTableEntries = m_numIntervals;
    m_dataSize = (unsigned) sizeof(m_table[0]) * (m_numTableEntries);

    __IS_NULL(func_container->autodiff3_func);
    mp_boost_func = func_container->autodiff3_func;

    /* Allocate and set table */
    m_table.reset(new polynomial<TOUT,4>[m_numTableEntries]);
    for (int ii=0;ii<m_numIntervals;++ii) {
      TIN x;
      // (possibly) transform the uniform grid into a nonuniform grid
      if (GT == UNIFORM)
        x = m_minArg + ii*m_stepSize;
      else
        x = m_transferFunction.g(m_minArg + ii*m_stepSize);

      m_grid[ii] = x;
      auto const derivs = (mp_boost_func)(make_fvar<TIN,3>(x));

      m_table[ii].coefs[0] = derivs.derivative(0);
      m_table[ii].coefs[1] = derivs.derivative(1);
      m_table[ii].coefs[2] = derivs.derivative(2)/2;
      m_table[ii].coefs[3] = derivs.derivative(3)/6;
    }
  }

  /* build this table from a file. Everything other than m_table is built by MetaTable */
  CubicTaylorTable(FunctionContainer<TIN,TOUT> *func_container, std::string filename) :
    MetaTable<TIN,TOUT,4,TAYLOR,GT>(func_container, filename,
        grid_type_to_string<GT>() + "CubicTaylorTable") {}
  // operator() comes from MetaTable

  std::function<adVar<TOUT,3>(adVar<TOUT,3>)> boost_function(){ return mp_boost_func; }
};

// define friendlier names
template <typename TIN, typename TOUT=TIN>
using UniformCubicTaylorTable = CubicTaylorTable<TIN,TOUT,UNIFORM>;