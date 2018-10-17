/*******************************************************************\

Module: Validate Goto Programs

Author: Diffblue

Date: Oct 2018

\*******************************************************************/
#include "validate_goto_model.h"

#include <set>

#include <util/invariant.h>
#include <util/namespace.h>

#include "expr_util.h"
#include "goto_model.h"

class validate_goto_modelt
{
public:
  using function_mapt = goto_functionst::function_mapt;

  validate_goto_modelt(
    const goto_functionst &goto_functions,
    const validation_modet vm,
    bool check_function_id,
    bool check_no_fn_ptr_calls,
    bool check_no_returns)
    : vm{vm}, function_map{goto_functions.function_map}
  {
  }

  void do_goto_program_checks(
    bool check_function_id,
    bool check_no_fn_ptr_calls,
    bool check_no_returns);

  /// check the goto_program has an entry point
  void entry_point_exists();

  /// 'function' field of an instruction should contain the id of
  /// the function it is currently in [OPTIONAL]
  void instruction_function_id_is_valid();

  /// check there are no function pointers [OPTIONAL]
  void function_pointer_calls_removed();

  /// no returns in each goto_program.
  /// NB function pointer calls must have been removed already when removing
  /// returns - so this also enables 'function_pointer_calls_removed()'
  void check_returns_removed();

  /// a) check every function call callee is in the function map
  /// b) check every function whose address is taken is in the function map
  void check_called_functions();

  /// check each function has a body and the last instruction is an 'end
  /// function' instruction
  void check_last_instruction();

private:
  const validation_modet vm;
  const function_mapt &function_map;
};

void validate_goto_modelt::do_goto_program_checks(
  bool check_function_id,
  bool check_no_fn_ptr_calls,
  bool check_no_returns)
{
  entry_point_exists();

  if(check_function_id)
    instruction_function_id_is_valid();

  if(check_no_fn_ptr_calls || check_no_returns)
    function_pointer_calls_removed();

  if(check_no_returns)
    check_returns_removed();

  check_called_functions();
  check_last_instruction();
}

void validate_goto_modelt::entry_point_exists()
{
  DATA_CHECK(
    function_map.find(goto_functionst::entry_point()) != function_map.end(),
    "an entry point must exist");
}

void validate_goto_modelt::instruction_function_id_is_valid()
{
  for(const auto &fun : function_map)
  {
    for(auto &instr : fun.second.body.instructions)
      DATA_CHECK(
        instr.function == fun.first, "instruction id must be parent function");
  }
}

void validate_goto_modelt::function_pointer_calls_removed()
{
  for(const auto &fun : function_map)
  {
    for(auto &instr : fun.second.body.instructions)
    {
      if(instr.is_function_call())
      {
        const code_function_callt &function_call =
          to_code_function_call(instr.code);
        DATA_CHECK(
          function_call.function().id() == ID_symbol,
          "no calls via fn pointer should be present");
      }
    }
  }
}

void validate_goto_modelt::check_returns_removed()
{
  for(const auto &fun : function_map)
  {
    const goto_functiont &goto_function = fun.second;
    DATA_CHECK(
      goto_function.type.return_type().id() == ID_empty,
      "functions must have empty return type");

    for(const auto &instr : goto_function.body.instructions)
    {
      DATA_CHECK(
        !instr.is_return(), "no return instructions should be present");

      if(instr.is_function_call())
      {
        const auto &function_call = to_code_function_call(instr.code);
        DATA_CHECK(
          function_call.lhs().is_nil(), "function call return should be nil");

        // consider: if this is redundant?
        // I could not trigger this with unit tests without failing
        // the first test "goto_function.type.return_type().id() == ID_empty"
        // (or a built in invariant if callee was not in symbol table)
        const auto &callee = to_code_type(function_call.function().type());
        DATA_CHECK(callee.return_type().id() == ID_empty, "");
      }
    }
  }
}

void validate_goto_modelt::check_called_functions()
{
  class test_for_function_addresst : public const_expr_visitort
  {
  private:
    const irep_idt match_id{ID_address_of};

  public:
    std::set<irep_idt> identifiers;

    void operator()(const exprt &expr) override
    {
      if(expr.id() == match_id)
      {
        const exprt &pointee = to_address_of_expr(expr).object();
        if(pointee.id() == ID_symbol)
          identifiers.insert(to_symbol_expr(pointee).get_identifier());
      }
    }

    void clear()
    {
      identifiers.clear();
    }
  } test_for_function_address;

  for(const auto &fun : function_map)
  {
    for(auto &instr : fun.second.body.instructions)
    {
      if(instr.is_function_call())
      {
        const auto &function_call = to_code_function_call(instr.code);
        const irep_idt &identifier =
          to_symbol_expr(function_call.function()).get_identifier();

        DATA_CHECK(function_map.find(identifier) != function_map.end(),
                   "every function call callee must be in the function map");
      }

      // to consider: excluding some of the 18 instruction types
      // from this brute force check
      const exprt &src{instr.code};
      src.visit(test_for_function_address);
      if(!test_for_function_address.identifiers.empty())
      {
        for(auto &identifier : test_for_function_address.identifiers)
          DATA_CHECK(function_map.find(identifier) != function_map.end(),
                     "every function whose address is taken must be in the "
                     "function map");
      }
      test_for_function_address.clear();
    }
  }
}

void validate_goto_modelt::check_last_instruction()
{
  for(const auto &fun : function_map)
  {
    DATA_CHECK(
      fun.second.body_available(),
      "all goto_programt instances should have a body");
    DATA_CHECK(
      fun.second.body.instructions.back().is_end_function(),
      "last instruction should be of end function type");
  }
}

void validate_goto_model(
  const goto_functionst &goto_functions,
  const validation_modet vm,
  bool check_function_id,
  bool check_no_fn_ptr_calls,
  bool check_no_returns)
{
  validate_goto_modelt{goto_functions,
                       vm,
                       check_function_id,
                       check_no_fn_ptr_calls,
                       check_no_returns}
    .do_goto_program_checks(
      check_function_id, check_no_fn_ptr_calls, check_no_returns);
}
