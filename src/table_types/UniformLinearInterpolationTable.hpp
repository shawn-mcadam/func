/*
  Linear Interpolation LUT with uniform sampling

  Usage example:
    UniformLinearInterpolationTable look(&function,0,10,0.0001);
    double val = look(0.87354);

  Notes:
  - static data after constructor has been called
  - evaluate by using parentheses, just like a function
*/
#pragma once
#include "UniformLookupTable.hpp"

template <typename IN_TYPE, typename OUT_TYPE>
class UniformLinearInterpolationTable final : public UniformLookupTable<IN_TYPE,OUT_TYPE>
{
  INHERIT_EVALUATION_IMPL(IN_TYPE,OUT_TYPE);
  INHERIT_UNIFORM_LUT(IN_TYPE,OUT_TYPE);
  REGISTER_LUT(UniformLinearInterpolationTable);

  __attribute__((aligned)) std::unique_ptr<polynomial<OUT_TYPE,1>[]> m_table;
public:
  UniformLinearInterpolationTable(FunctionContainer<IN_TYPE,OUT_TYPE> *func_container, UniformLookupTableParameters<IN_TYPE> par) :
    UniformLookupTable<IN_TYPE,OUT_TYPE>(func_container, par)
  {
    /* Base class variables */
    m_name = STR(UniformLinearInterpolationTable);
    m_order = 2;
    m_numTableEntries = m_numIntervals;
    m_dataSize = (unsigned) sizeof(m_table[0]) * m_numTableEntries;

    /* Allocate and set table */
    m_table.reset(new polynomial<OUT_TYPE,1>[m_numTableEntries]);
    for (int ii=0; ii<m_numTableEntries; ++ii) {
      const IN_TYPE x = m_minArg + ii*m_stepSize;
      m_grid[ii]  = x;
      m_table[ii].coefs[0] = (mp_func)(x);
    }
  }

  OUT_TYPE operator()(IN_TYPE x) override
  {
    // nondimensionalized x position, scaled by step size
    OUT_TYPE dx = (x-m_minArg)/m_stepSize;
    // index of previous table entry
    unsigned x1 = (unsigned) dx;
    // value of table entries around x position
    dx -= x1;
    OUT_TYPE y1  = m_table[x1].coefs[0];
    OUT_TYPE y2  = m_table[x1+1].coefs[0];
    // linear interpolation
    return y1+dx*(y2-y1);
  }
};

REGISTER_DOUBLE_AND_FLOAT_LUT_IMPLS(UniformLinearInterpolationTable);
