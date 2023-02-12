/*
   MetaTable handles any _piecewise polynomial based_ interpolation:
   - Reduce code redundancy by factoring out common differences between table types
   (eg method of generating a nonuniform grid type, setup/reading polynomial coefficients)
   which can be pieced together based on template parameters.

   NOTE: if stepSize divides max-min exactly then operator(max) will be out of array bounds!
   - Every table has an extra (unnecessary in most cases) table entry to avoid this
   problem.
   
   N = number of coefficients used in underlying piecewise polynomials
   Provided Horner's method which is the most common table evaluation method in FunC

   UNIFORM: Every subinterval is the same length so the hash is super fast; however,
    many more subintervals might be needed
   NONUNIFORM: Use a transfer function to create a nonuniform grid with an O(1) hash.
   NONUNIFORM_PSEUDO: same as NONUNIFORM but uses a faster, less accurate hash.

   TODO we could make another template which makes FunC save derivative coefs
   on the same cache line as the original coefs (hopefully with implementation
   independent of the LUT in use)
*/
#pragma once
#include "LookupTable.hpp"
#include "TransferFunctionSinh.hpp"
#include "json.hpp"
#include <array>
#include <stdexcept>

#define INHERIT_META(TIN,TOUT,N,GT) \
  using MetaTable<TIN,TOUT,N,GT>::m_table; \
  using MetaTable<TIN,TOUT,N,GT>::m_grid; \
  using MetaTable<TIN,TOUT,N,GT>::m_transferFunction

/* Parallelization macro. Notes:
 * - m_table is aligned to sizeof(TOUT) so that might give some speedup.
 * */
#define FUNC_BUILDPAR _Pragma("omp parallel for schedule(dynamic)")
//#define FUNC_BUILDPAR


namespace func {

static constexpr unsigned int alignments[] = {0,1,2,4,4,8,8,8,8,16,16,16,16,16,16,16,16};

/* Our convention for writing polynomials is:
 *  p(x) = m_table[x0].coefs[0] + m_table[x0].coefs[1]*x + ... + m_table[x0].coefs[N-1]*x^(N-1)
 * operator() does a Horner's style evaluation>
 *
 * Note: LUTs can have other things in their polynomial struct
 * (2D LUTs may have coefs for x & y dimensions of each subsquare,
 * and LUTs could have that polynomial's derivative's coefs, etc). */
template <typename TOUT, unsigned int N>
struct alignas(sizeof(TOUT)*alignments[N]) polynomial {
  static const unsigned int num_coefs = N;
  TOUT coefs[N];
};

enum class GridTypes {UNIFORM, NONUNIFORM, NONUNIFORM_PSEUDO};

template <GridTypes GT>
const std::string grid_type_to_string() {
  switch(GT){
    case GridTypes::UNIFORM:
      return "Uniform";
    case GridTypes::NONUNIFORM:
      return "NonUniform";
    case GridTypes::NONUNIFORM_PSEUDO:
      return "NonUniformPseudo";
    default: { throw std::logic_error("Broken switch case in func::MetaTable"); }
  } 
}

template <typename TIN, typename TOUT, unsigned int N, GridTypes GT=GridTypes::UNIFORM>
class MetaTable : public LookupTable<TIN,TOUT>
{
protected:
  INHERIT_EVALUATION_IMPL(TIN,TOUT);
  INHERIT_LUT(TIN,TOUT);

  // m_grid and m_table MUST have size m_numTableEntries!
  std::unique_ptr<TIN[]> m_grid; // useful for nonuniform tables
  __attribute__((aligned)) std::unique_ptr<polynomial<TOUT,N>[]> m_table; // holds polynomials coefficients
  TransferFunctionSinh<TIN> m_transferFunction; // used to make nonuniform grids (default constructable)

public:
  // std::unique_ptr m_array implicitly deletes the default move ctor so we have to explicitly
  // ask for the default move ctor
  MetaTable(MetaTable&&) = default;
  MetaTable() = default;

  /* Set every generic member variable from a json file */
  MetaTable(FunctionContainer<TIN,TOUT> *func_container, LookupTableParameters<TIN> par) :
    LookupTable<TIN,TOUT>(func_container, par)
  {
    // We need a valid FunctionContainer to generate any LUT
    if(m_func == nullptr)
      throw std::invalid_argument("Error in func::MetaTable. Function not defined in given FunctionContainer");

    // initialize the transfer function to something useful if the grid is nonuniform
    if(GT != GridTypes::UNIFORM)
      m_transferFunction = TransferFunctionSinh<TIN>(func_container,m_minArg,m_tableMaxArg,m_stepSize);
  }

  /* build this table from a json file */
  MetaTable(const nlohmann::json& jsonStats, std::string tablename, FunctionContainer<TIN,TOUT> *func_container = nullptr)
  {
    if(jsonStats.empty())
      throw std::invalid_argument("Error in func::MetaTable: The provided nlohmann::json is empty");

    // check that the names match
    m_name = jsonStats["name"].get<std::string>();
    if(m_name != tablename)
      throw std::invalid_argument("Error while building " + tablename + " : provided nlohmann::json "
          " contains data for building a " + m_name + " which is not compatible");

    // user might want to still have info about the original function attached to this LUT
    m_func = (func_container != nullptr) ? func_container->standard_func : nullptr;

    from_json(jsonStats, *this);
  }


  /* Write table data to the provided ostream in the form of json */
  virtual void print_details_json(std::ostream& out) override {
    nlohmann::json jsonStats = *this; // call to_json(jsonStats, this)
    out << jsonStats.dump(2) << std::endl;
  }

  // give the nlohmann json functions access to every member variable (note the silly template names)
  template <typename TIN1, typename TOUT1, unsigned int N1, GridTypes GT1,
         typename std::enable_if<std::is_constructible<nlohmann::json,TIN1 >::value && 
                                 std::is_constructible<nlohmann::json,TOUT1>::value, bool>::type>
  friend void to_json(nlohmann::json& jsonStats, const MetaTable<TIN1,TOUT1,N1,GT1>& lut);

  template <typename TIN1, typename TOUT1, unsigned int N1, GridTypes GT1,
         typename std::enable_if<std::is_constructible<nlohmann::json,TIN1 >::value && 
                                 std::is_constructible<nlohmann::json,TOUT1>::value, bool>::type>
  friend void from_json(const nlohmann::json& jsonStats, MetaTable<TIN1,TOUT1,N1,GT1>& lut);

  /* public access to protected member vars */
  unsigned int num_coefs() const { return N; }
  TOUT table_entry(unsigned int i, unsigned int j) const { return m_table[i].coefs[j]; }
  TIN grid_entry(unsigned int i) const { return m_grid[i]; }
  std::array<TIN,4> transfer_function_coefs() const { return m_transferFunction.get_coefs(); }

  /* Provide the most common hash. The compiler should simplify this method when templates are instantiated 
   * TODO profile to verify that this is actually inlined and doesn't have to be a macro */
  inline std::pair<unsigned int, TOUT> hash(TIN x){
    /* TODO might be able to get some speedup by using c++14's constexpr for this switch?
     * But idk hopefully the compiler optimizes out the unnecessary cases anyways */
    unsigned int x0; TOUT dx;
    switch(GT){
      case GridTypes::UNIFORM:
        {
        // nondimensionalized x position, scaled by step size
        dx  = static_cast<TOUT>(m_stepSize_inv*(x-m_minArg));
        // index of previous table entry
        x0  = static_cast<unsigned>(dx);
        // value of table entries around x position
        dx -= x0;
        break;
        }
      case GridTypes::NONUNIFORM:
        {
        // find the subinterval x lives in
        x0 = static_cast<unsigned>(m_transferFunction.g_inv(x));
        // find where x is within that interval
        TIN h = m_grid[x0+1] - m_grid[x0];
        dx    = (x - m_grid[x0])/h;
        break;
        }
      case GridTypes::NONUNIFORM_PSEUDO:
        {
        // find the subinterval x lives in
        dx  = static_cast<TOUT>(m_transferFunction.g_inv(x));
        // just take the fractional part of dx as x's location in this interval
        x0  = static_cast<unsigned>(dx);
        dx -= x0;
        break;
        }
    }
    return std::make_pair(x0, dx);
  }

  /* TODO this currently cannot be final because the Pade tables must override it; however,
   * there might be a performance benefit to making this final. Profile to check the pros & cons */
  #pragma omp declare simd
  TOUT operator()(TIN x) override
  {
    unsigned int x0; TOUT dx;
    std::tie(x0,dx) = hash(x);
    
    // general degree horners method, evaluated from the inside out.
    TOUT sum = 0;
    for(int k=N-1; k>0; k--)
      sum = dx*(m_table[x0].coefs[k] + sum);
    return m_table[x0].coefs[0]+sum;
  }

  //template <unsigned K>
  //std::array<TOUT,K> transform(std::array<TIN,K>& v) {
  //  std::array<TOUT,K> w;
  //  #pragma omp simd
  //  for(int k=0; k<K; k++){
  //    w[k] = operator()(v[k]);
  //  }
  //}

  //virtual TOUT diff(unsigned int N, TIN x) = 0;
};

/* Reading & writing functions for any LUT derived from MetaTable.
 * Enables the convenient "get" syntax from nlohmann::json for specific implementations.
   eg:
  ```c++
  nlohmann::json jsonStats;
  std::ifstream(filename) >> jsonStats;
  auto lut = jsonStats.get<func::UniformLinearInterpolationTable<TIN,TOUT>>();
  ```
 * Uses SFINAE to automatically disable these functions if TIN or TOUT do not support nlohmann's json */
template <typename TIN, typename TOUT, unsigned int N, GridTypes GT,
         typename std::enable_if<std::is_constructible<nlohmann::json,TIN >::value && 
                                 std::is_constructible<nlohmann::json,TOUT>::value, bool>::type = true>
void to_json(nlohmann::json& jsonStats, const MetaTable<TIN,TOUT,N,GT>& lut)
{
  jsonStats["_comment"] = "FunC lookup table data";
  jsonStats["name"] = lut.m_name;
  jsonStats["minArg"] = lut.m_minArg;
  jsonStats["maxArg"] = lut.m_maxArg;
  jsonStats["order"] = lut.m_order;
  jsonStats["dataSize"] = lut.m_dataSize;
  jsonStats["stepSize"] = lut.m_stepSize;
  jsonStats["numTableEntries"] = lut.m_numTableEntries;
  jsonStats["numIntervals"] = lut.m_numIntervals;
  jsonStats["tableMaxArg"] = lut.m_tableMaxArg;

  // things that are important for nonuniform tables:
  jsonStats["transfer_function_coefs"] = lut.transfer_function_coefs();
  for(unsigned int i=0; i<lut.m_numTableEntries; i++)
    jsonStats["grid"][std::to_string(i)] = lut.m_grid[i];

  // save the polynomial coefs of each lookup table
  // Note: m_order is used as the number of polynomial coefs
  for(unsigned int i=0; i<lut.m_numTableEntries; i++)
    for(unsigned int j=0; j<lut.num_coefs(); j++)
      jsonStats["table"][std::to_string(i)]["coefs"][std::to_string(j)] = lut.table_entry(i,j);
}

template <typename TIN, typename TOUT, unsigned int N, GridTypes GT,
         typename std::enable_if<!(std::is_constructible<nlohmann::json,TIN >::value && 
                                   std::is_constructible<nlohmann::json,TOUT>::value), bool>::type = true>
void to_json(nlohmann::json& jsonStats, const MetaTable<TIN,TOUT,N,GT>& lut)
{
  (void) jsonStats;
  (void) lut;
  throw std::invalid_argument(std::string(typeid(TIN).name()) + " or " + std::string(typeid(TOUT).name()) + " does not implement nlohmann's to_json");
}

/* this variant of from_json will be called for any specific implementation of a LUT
 * inhereting from MetaTable */
template <typename TIN, typename TOUT, unsigned int N, GridTypes GT,
         typename std::enable_if<std::is_constructible<nlohmann::json,TIN >::value && 
                                 std::is_constructible<nlohmann::json,TOUT>::value, bool>::type = true>
void from_json(const nlohmann::json& jsonStats, MetaTable<TIN,TOUT,N,GT>& lut)
{
  // name checking happens in MetaTable's constructor
  jsonStats.at("name").get_to(lut.m_name);
  jsonStats.at("minArg").get_to(lut.m_minArg);
  jsonStats.at("maxArg").get_to(lut.m_maxArg);
  jsonStats.at("stepSize").get_to(lut.m_stepSize);
  lut.m_stepSize_inv = 1.0/lut.m_stepSize;

  jsonStats.at("stepSize").get_to(lut.m_stepSize);
  jsonStats.at("order").get_to(lut.m_order);
  jsonStats.at("dataSize").get_to(lut.m_dataSize);
  jsonStats.at("numIntervals").get_to(lut.m_numIntervals);
  jsonStats.at("numTableEntries").get_to(lut.m_numTableEntries);
  jsonStats.at("tableMaxArg").get_to(lut.m_tableMaxArg);

  // read array of points used
  lut.m_grid.reset(new TIN[lut.m_numTableEntries]);
  for(unsigned int i=0; i<lut.m_numTableEntries; i++)
    jsonStats.at("grid").at(std::to_string(i)).get_to(lut.m_grid[i]);

  // Recompute m_table (the array of polynomials) and the transfer function
  lut.m_table.reset(new polynomial<TOUT,N>[lut.m_numTableEntries]);
  for(unsigned int i=0; i<lut.m_numTableEntries; i++)
    for(unsigned int j=0; j<lut.num_coefs(); j++)
      jsonStats.at("table").at(std::to_string(i)).at("coefs").at(std::to_string(j)).get_to(lut.m_table[i].coefs[j]);

  // rebuild the transfer function
  lut.m_transferFunction = TransferFunctionSinh<TIN>(lut.m_minArg,lut.m_tableMaxArg,lut.m_stepSize,
      jsonStats["transfer_function_coefs"].get<std::array<TIN,4>>());
}

template <typename TIN, typename TOUT, unsigned int N, GridTypes GT,
         typename std::enable_if<!(std::is_constructible<nlohmann::json,TIN >::value && 
                                   std::is_constructible<nlohmann::json,TOUT>::value), bool>::type = true>
void from_json(const nlohmann::json& jsonStats, MetaTable<TIN,TOUT,N,GT>& lut)
{
  (void) jsonStats;
  (void) lut;
  throw std::invalid_argument(std::string(typeid(TIN).name()) + " or " + std::string(typeid(TOUT).name()) + " does not implement nlohmann's to_json");
}

} // namespace func
