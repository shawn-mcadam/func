/*
  Quadratic Interpolation LUT with precomputed coefficients

  Usage example:
    QuadraticInterpolationTable look(&function,0,10,0.0001);
    double val = look(0.87354);

  Notes:
  - table precomputes and stores the linear coefficient so it doesn't have to
    perform that operation every lookup (but does have to look it up)
  - static data after constructor has been called
  - evaluate by using parentheses, just like a function
*/
#pragma once
#include "MetaTable.hpp"

namespace func {

template <typename TIN, typename TOUT=TIN, GridTypes GT=GridTypes::UNIFORM>
class QuadraticInterpolationTable final : public MetaTable<TIN,TOUT,3,GT>
{
  INHERIT_EVALUATION_IMPL(TIN,TOUT);
  INHERIT_LUT(TIN,TOUT);
  INHERIT_META(TIN,TOUT,3,GT);

  static const std::string classname;
public:
  // build the LUT from scratch or look in filename for an existing LUT
  QuadraticInterpolationTable(FunctionContainer<TIN,TOUT> *func_container, LookupTableParameters<TIN> par,
      const nlohmann::json& jsonStats=nlohmann::json()) :
    MetaTable<TIN,TOUT,3,GT>(jsonStats.empty() ? // use the default move constructor for MetaTable (probably not elided...)
      std::move(MetaTable<TIN,TOUT,3,GT>(func_container, par)) :
      std::move(MetaTable<TIN,TOUT,3,GT>(jsonStats, classname, func_container)))
  {
    if(!jsonStats.empty())
      return; // all our work is already done

    /* Base class default variables */
    m_name = classname;
    m_order = 3;
    m_numTableEntries = m_numIntervals+1;
    m_dataSize = static_cast<unsigned>(sizeof(m_table[0]) * (m_numTableEntries));

    /* Allocate and set table */
    m_grid.reset(new TIN[m_numTableEntries]);
    m_table.reset(new polynomial<TOUT,3>[m_numTableEntries]);
    for (unsigned int ii=0;ii<m_numTableEntries-1;++ii) {
      TIN x;
      TIN h = m_stepSize;
      // (possibly) transform the uniform grid into a nonuniform grid
      if (GT == GridTypes::UNIFORM)
        x = m_minArg + ii*m_stepSize;
      else{
        x = m_transferFunction.g(m_minArg + ii*m_stepSize);
        h = m_transferFunction.g(m_minArg + (ii+1)*m_stepSize) - x;
      }

      // grid points
      m_grid[ii] = x;
      // polynomial coefficients
      const TOUT y0  = m_func(x);
      const TOUT y1  = m_func(x+h/2);
      const TOUT y2  = m_func(x+h);
      m_table[ii].coefs[0] = y0;
      m_table[ii].coefs[1] = -3*y0+4*y1-y2;
      m_table[ii].coefs[2] = 2*y0+-4*y1+2*y2;
    }
    // special case to make lut(tableMaxArg) work
    m_grid[m_numTableEntries-1] = m_tableMaxArg;
    m_table[m_numTableEntries-1].coefs[0] = m_func(m_tableMaxArg);
    for (unsigned int k=1; k<3; k++)
      m_table[m_numTableEntries-1].coefs[k] = 0;
  }
};

template <typename TIN, typename TOUT, GridTypes GT>
const std::string QuadraticInterpolationTable<TIN,TOUT,GT>::classname = grid_type_to_string<GT>() + "QuadraticInterpolationTable";

template <typename TIN, typename TOUT=TIN>
using UniformQuadraticInterpolationTable = QuadraticInterpolationTable<TIN,TOUT,GridTypes::UNIFORM>;
template <typename TIN, typename TOUT=TIN>
using NonUniformQuadraticInterpolationTable = QuadraticInterpolationTable<TIN,TOUT,GridTypes::NONUNIFORM>;
template <typename TIN, typename TOUT=TIN>
using NonUniformPseudoQuadraticInterpolationTable = QuadraticInterpolationTable<TIN,TOUT,GridTypes::NONUNIFORM_PSEUDO>;
} // namespace func