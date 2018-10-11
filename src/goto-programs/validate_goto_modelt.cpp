//
// Created by Sonny Martin on 07/10/2018.
//
#include "validate_goto_modelt.h"

#include <queue>

#include "goto_functions.h"
#include "goto_model.h"
#include "util/invariant.h"
#include <util/namespace.h>

class validate_goto_modelt
{
public:
  typedef std::map<irep_idt, goto_functiont> function_mapt;

  validate_goto_modelt(
    const goto_modelt &,
    message_handlert &message_handler,
    ui_message_handlert::uit ui);

  /// call all the checks - true if they all pass (temp implementation only)
  /// (see the method impl for some remarks on this)
  bool do_goto_program_checks();

  /// check the goto_program has an entry point
  bool entry_point_exists();

  ///'function' field of an instruction should contain the id of the function
  /// it is currently in [optional]
  bool instruction_function_id_isvalid();

  /// check there are no function pointers
  bool function_pointer_calls_removed();

  /// no returns in each goto_program -
  /// "function pointer calls must have been removed already when removing
  // returns" - so this should also enable 'function_pointer_calls_removed'
  // NB (atm it does not - see my other remarks)
  bool check_returns_removed();

  /// a) check every function call callee is in the function map
  /// b) check every function whose address is taken is in the function map
  bool check_called_functions();

  /// check each function has a body and the last instruction is an 'end
  /// function' instruction
  bool check_last_instruction();

  //TODO change to private (atm if private compiler complains e.g. 'ui'
  // not used)
public:
  const namespacet ns;
  const goto_functionst &goto_functions;
  const function_mapt &function_map;

  message_handlert &message_handler;
  ui_message_handlert::uit ui;
  messaget msg;
};

validate_goto_modelt::validate_goto_modelt(
  const goto_modelt &goto_model,
  message_handlert &message_handler,
  ui_message_handlert::uit ui)
  : ns{goto_model.symbol_table},
    goto_functions{goto_model.goto_functions},
    function_map{goto_functions.function_map},
    message_handler{message_handler},
    ui{ui},
    msg{message_handler}
{
  // this skeleton is here so you, dear reader, are aware I'm aware...
  messaget msg(message_handler);

  switch(ui)
  {
  case ui_message_handlert::uit::XML_UI:
  {
  }
  break;

  case ui_message_handlert::uit::JSON_UI:
  {
  }
  break;

  case ui_message_handlert::uit::PLAIN:
  {
  }
  break;
  }
}

bool validate_goto_modelt::do_goto_program_checks()
{
  bool result{true};
  result = entry_point_exists();
  result = instruction_function_id_isvalid() && result;
  result = function_pointer_calls_removed() && result;
  result = check_returns_removed() && result;
  result = check_called_functions() && result;
  result = check_last_instruction() && result;

  // Am aware of the true/false rtn debate. Currently use true for success/pass.
  // I would like to prompt some feedback on method names above in case
  // of returning false - e.g.
  // a. preserve names but return opposite of implied
  // b. or remove 'state' from name e.g. just use 'check_entry_point'
  // c. or 'double negative' style e.g. 'entry_point_not_exists' - and return
  // false when it does.
  return result;
}

bool validate_goto_modelt::entry_point_exists()
{
  for(const auto &fun : function_map)
  {
    if(goto_functions.entry_point() == fun.first)
      return true;
  }
  return false;
}

bool validate_goto_modelt::instruction_function_id_isvalid()
{
  for(const auto &fun : function_map)
  {
    for(auto &instr : fun.second.body.instructions)
    {
      if(instr.function != fun.first)
        return false;
    }
  }
  return true;
}

bool validate_goto_modelt::function_pointer_calls_removed()
{
  for(const auto &fun : function_map)
  {
    for(auto &instr : fun.second.body.instructions)
    {
      if(instr.is_function_call())
      {
        const code_function_callt &function_call =
          to_code_function_call(instr.code);

        if(function_call.function().id() == ID_dereference)
          return false;
      }
    }
  }
  return true;
}

bool validate_goto_modelt::check_returns_removed()
{
  for(const auto &fun : function_map)
  {
    const goto_functiont &goto_function = fun.second;
    for(const auto &instr : goto_function.body.instructions)
    {
      if(goto_function.type.return_type().id() != ID_empty)
        return false;

      if(instr.is_return())
        return false;

      if(instr.is_function_call())
      {
        const auto &function_call = to_code_function_call(instr.code);
        if(!function_call.lhs().is_nil())
          return false;

        const auto &callee = to_code_type(function_call.function().type());
        if(callee.return_type().id() != ID_empty)
          return false;
      }
    }
  }
  return true;
}

bool validate_goto_modelt::check_called_functions()
{
  class test_for_function_addresst : public const_expr_visitort
  {
  private:
    const exprt expr_address_of;

  public:
    bool found{false};
    std::queue<irep_idt> identifiers; //preserve order

    explicit test_for_function_addresst(exprt expr_address_of)
      : expr_address_of{expr_address_of}
    {
    }

    void operator()(const exprt &expr) override
    {
      if(expr.id() == ID_address_of)
      {
        auto &pointee = to_address_of_expr(expr).object();
        if(pointee.type().id() == ID_code) // a function
        {
          found = true;
          irep_idt identifier = to_symbol_expr(pointee).get_identifier();
          identifiers.push(identifier);
        }
      }
    }

    void clear()
    {
      found = false;
      std::queue<irep_idt>().swap(identifiers); // this should be redundant
    };
  };

  test_for_function_addresst test_for_function_address(exprt{ID_address_of});

  for(const auto &fun : function_map)
  {
    for(const auto &instr : fun.second.body.instructions)
    {
      // check every function call callee is in the function map
      if(instr.is_function_call())
      {
        const auto &function_call = to_code_function_call(instr.code);

        const irep_idt &identifier =
          to_symbol_expr(function_call.function()).get_identifier();

        if(function_map.find(identifier) == function_map.end())
          return false;
      }

      // verify every function whose address is taken is in the function map
      // NB the below is a _very_ brute force: iterating over _every_
      // instruction and _every_ expression.
      // To consider: more optimal approaches, no time to give it thought yet
      const exprt &src = instr.code;
      src.visit(test_for_function_address);

      if(test_for_function_address.found)
      {
        while(!test_for_function_address.identifiers.empty())
        {
          auto &identifier = test_for_function_address.identifiers.front();
          if(function_map.find(identifier) == function_map.end())
            return false;

          test_for_function_address.identifiers.pop();
        }
      }

      test_for_function_address.clear();
    } // end of iterating over all instructions
  }
  return true;
}

bool validate_goto_modelt::check_last_instruction()
{
  for(const auto &fun : function_map)
  {
    if(fun.second.body_available() != true)
      return false;

    if(!fun.second.body.instructions.back().is_end_function())
      return false;
  }
  return true;
}

bool validate_goto_model(
  const goto_modelt &goto_model,
  message_handlert &message_handler,
  ui_message_handlert::uit ui)
{
  return validate_goto_modelt{goto_model, message_handler, ui}
    .do_goto_program_checks();
}
