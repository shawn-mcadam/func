/* 
   Interface for a class that builds and contains FunC transfer
   function pairs g and g^{-1}. These are the backbones of every
   NonUniformLookupTable and are used to map a uniform grid
   in [a,b] to a non-uniform grid in [a,b].
   The new grid will ideally do a better job of distributing error
   when used for grid points.
   g must satisfy the following conditions:
   g(a) = a;
   g(b) = b;
   x < y implies g(x) < g(y) (ie, g must be strictly increasing).

   Another important stipulation is that the encapsulating nonuniform
   table's hash should be baked into g_inv. This will obfustcate the nonuniform
   lookup tables, but this way the the cost of using a transfer
   function just boils down to an indirection.

  Notes:
    - g^{-1} must be quick to evaluate to see any speedup compared to uniform
    lookup tables
    - Want more grid points where g changes the slowest so we
    want g' to grow like 1/f' in order to distribute error.
*/
#pragma once
#include <utility> // std::pair
#include <memory> // std::unique_ptr

/* inheritance macro */
#define IMPLEMENT_TRANSFER_FUNCTION_INTERFACE(IN_TYPE) \
  using TransferFunctionInterface<IN_TYPE>::m_minArg; \
  using TransferFunctionInterface<IN_TYPE>::m_tableMaxArg; \
  using TransferFunctionInterface<IN_TYPE>::m_stepSize;

template <typename IN_TYPE>
class TransferFunctionInterface
{
protected:
  /* This min, max must be the same as the corresponding table's min and
  max resp. (though note that the table max is not necessarily equal to
  the function's max arg) */
  IN_TYPE m_minArg, m_tableMaxArg;
  IN_TYPE m_stepSize;

public:
  // build the function pair
  template <typename OUT_TYPE>
  TransferFunctionInterface(FunctionContainer<IN_TYPE,OUT_TYPE> *fc,
      IN_TYPE minArg, IN_TYPE tableMaxArg, IN_TYPE stepSize) :
    m_minArg(minArg), m_tableMaxArg(tableMaxArg), m_stepSize(stepSize) {}

  virtual ~TransferFunctionInterface(){}

  virtual void print_details(std::ostream& out){};

  // public access to private vars
  std::pair<IN_TYPE,IN_TYPE> arg_bounds_of_interval(){ return std::make_pair(m_minArg, m_tableMaxArg); }

  // getters used to ensure g and g_inv are implemented
  virtual IN_TYPE g(IN_TYPE x) = 0;
  virtual IN_TYPE g_inv(IN_TYPE x) = 0;
};
