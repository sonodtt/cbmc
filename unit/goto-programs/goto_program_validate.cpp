/*******************************************************************\

 Module: Unit tests for goto program validation

 Author: Diffblue Ltd.

 Date: Oct 2018

\*******************************************************************/

/// \file
/// Unit tests for goto program validation

#include <testing-utils/catch.hpp>
#include <testing-utils/message.h>

#include <goto-programs/goto_convert_functions.h>
#include <goto-programs/validate_goto_model.h>

#include "util/std_code.h"
#include <util/arith_tools.h>
#include <util/c_types.h>

#include <validate.h>

static code_function_callt make_void_call(const symbol_exprt &function)
{
  code_function_callt ret;
  ret.function() = function;
  return ret;
}

static void
validate(const goto_functionst &goto_functions, const validation_modet vm)
{
  validate_goto_model(goto_functions, vm);
}

static bool test_for_pass(goto_modelt &goto_model)
{
  validate(goto_model.goto_functions, validation_modet::INVARIANT);
  return true;
}

static bool test_for_failure(goto_modelt &goto_model)
{
  bool caught{false};
  try
  {
    validate(goto_model.goto_functions, validation_modet::EXCEPTION);
  }
  catch(incorrect_goto_program_exceptiont &e)
  {
    caught = true;
  }
  return caught;
}

SCENARIO("validate goto program")
{
  goto_modelt goto_model;

  // void f(){int x = 1;}
  symbolt x;
  x.name = "x";
  x.mode = ID_C;
  x.type = signed_int_type();
  goto_model.symbol_table.add(x);

  symbolt f;
  f.name = "f";
  f.mode = ID_C;
  f.type = code_typet({}, empty_typet()); //void rtn, take no params

  code_blockt f_body;
  f_body.copy_to_operands(code_declt(x.symbol_expr()));
  f_body.copy_to_operands(
    code_assignt(x.symbol_expr(), from_integer(1, signed_int_type())));
  f.value = f_body;
  goto_model.symbol_table.add(f);

  // void g(){int y = 2;}
  symbolt y;
  y.name = "y";
  y.mode = ID_C;
  y.type = signed_int_type();
  goto_model.symbol_table.add(y);

  symbolt g;
  g.name = "g";
  g.mode = ID_C;
  g.type = code_typet({}, empty_typet());

  code_blockt g_body;
  g_body.copy_to_operands(code_declt(y.symbol_expr()));
  g_body.copy_to_operands(
    code_assignt(y.symbol_expr(), from_integer(2, signed_int_type())));
  g.value = g_body;
  goto_model.symbol_table.add(g);

  symbolt z;
  z.name = "z";
  z.mode = ID_C;
  z.type = signed_int_type();
  goto_model.symbol_table.add(z);

  symbolt fn_ptr;
  fn_ptr.name = "fn_ptr";
  fn_ptr.mode = ID_C;
  // pointer to fn call
  fn_ptr.type = pointer_typet(code_typet{{}, empty_typet()}, 64);
  goto_model.symbol_table.add(fn_ptr);

  symbolt entry_point;
  entry_point.name = goto_functionst::entry_point();
  entry_point.mode = ID_C;
  entry_point.type = code_typet({}, void_typet());

  code_blockt entry_point_body;
  entry_point_body.copy_to_operands(code_declt(z.symbol_expr()));
  entry_point_body.copy_to_operands(
    code_assignt(z.symbol_expr(), from_integer(3, signed_int_type())));

  entry_point_body.copy_to_operands(
    code_assignt(fn_ptr.symbol_expr(), address_of_exprt{f.symbol_expr()}));
  entry_point_body.copy_to_operands(make_void_call(g.symbol_expr()));

  entry_point.value = entry_point_body;
  goto_model.symbol_table.add(entry_point);

  /// test entry_point_exists()
  WHEN("the goto program has no entry point")
  {
    goto_convert(goto_model, null_message_handler);

    auto &function_map = goto_model.goto_functions.function_map;
    auto it = function_map.find(goto_functionst::entry_point());
    function_map.erase(it);

    THEN("fail!")
    {
      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN("the goto program has an entry point")
  {
    goto_convert(goto_model, null_message_handler);
    THEN("pass!")
    {
      REQUIRE(test_for_pass(goto_model));
    }
  }

  /// test instruction_function_id_is_valid()
  WHEN("instruction function id is not that of host function")
  {
    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);

      goto_programt::instructiont instruction;
      instruction.function = "nil";

      auto &function_map = goto_model.goto_functions.function_map;
      auto it = function_map.find("f");
      auto &instructions = it->second.body.instructions;
      instructions.insert(instructions.begin(), instruction);

      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN("instruction function id is that of host function")
  {
    THEN("pass!")
    {
      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_pass(goto_model));
    }
  }

  /// function_pointer_calls_removed()
  WHEN("not all function calls via fn pointer have been removed")
  {
    THEN("fail!")
    {
      //introduce fn k that has a fn pointer call;
      symbolt k;
      k.name = "k";
      k.mode = ID_C;
      k.type = code_typet({}, empty_typet()); //void rtn, take no params

      code_function_callt function_call;
      function_call.function() = dereference_exprt(
        fn_ptr.symbol_expr(), pointer_typet(code_typet{{}, empty_typet()}, 64));

      code_blockt k_body;
      k_body.copy_to_operands(function_call);
      k.value = k_body;

      goto_model.symbol_table.add(k);

      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN("all function calls via fn pointer have been removed")
  {
    THEN("pass!")
    {
      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_pass(goto_model));
    }
  }

  /// check_returns_removed()
  WHEN(
    "not all returns have been removed - a function has return type "
    "non-empty")
  {
    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);

      auto &function_map = goto_model.goto_functions.function_map;
      auto it = function_map.find("f");
      it->second.type.return_type() = signedbv_typet{32};

      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN(
    "not all returns have been removed - an instruction is of type "
    "'return'")
  {
    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);

      goto_programt::instructiont instruction;
      instruction.make_return();
      instruction.function = "g"; // if this is not set will fail id test

      auto &function_map = goto_model.goto_functions.function_map;
      auto it = function_map.find("g");
      auto &instructions = it->second.body.instructions;
      instructions.insert(instructions.begin(), instruction);

      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN("not all returns have been removed - a function call lhs is not nil")
  {
    symbolt h;
    h.name = "h";
    h.mode = ID_C;
    h.type = code_typet({}, void_typet{});
    h.value = code_blockt{};
    goto_model.symbol_table.add(h);

    symbolt k;
    k.name = "k";
    k.mode = ID_C;
    k.type = code_typet({}, empty_typet());
    code_blockt k_body;

    // the lhs is non-nil
    code_function_callt function_call{from_integer(1, signed_int_type()),
                                      h.symbol_expr(),
                                      code_function_callt::argumentst{}};

    k_body.copy_to_operands(function_call);
    k.value = k_body;
    goto_model.symbol_table.add(k);

    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN("all returns have been removed")
  {
    THEN("true!")
    {
      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_pass(goto_model));
    }
  }

  /// check_called_functions()
  WHEN("not every function call is in the function map")
  {
    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);

      auto &function_map = goto_model.goto_functions.function_map;
      // the entry point has a call to g()
      auto it = function_map.find("g");
      function_map.erase(it);

      test_for_failure(goto_model);
    }
  }

  WHEN("not every function whose address is taken is in the function map")
  {
    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);
      // the address of f is taken in the entry point function
      auto &function_map = goto_model.goto_functions.function_map;
      auto it = function_map.find("f");
      function_map.erase(it); //f is no longer in function map

      test_for_failure(goto_model);
    }
  }

  WHEN(
    "all functions calls and that of every function whose address is "
    "taken are in the function map")
  {
    THEN("true!")
    {
      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_pass(goto_model));
    }
  }

  /// check_last_instruction()
  WHEN("not every function has a body")
  {
    goto_convert(goto_model, null_message_handler);
    auto &function_map = goto_model.goto_functions.function_map;
    auto it = function_map.find("f");
    it->second.body.instructions.clear();
    THEN("fail!")
    {
      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN("the last instruction of a function is not an end function")
  {
    THEN("fail!")
    {
      goto_convert(goto_model, null_message_handler);
      auto &function_map = goto_model.goto_functions.function_map;
      auto it = function_map.find("f");
      it->second.body.instructions.erase(
        std::prev(it->second.body.instructions.end()));
      REQUIRE(test_for_failure(goto_model));
    }
  }

  WHEN(
    "every function has a body and the last instruction is always and end "
    "function")
  {
    THEN("pass!")
    {
      goto_convert(goto_model, null_message_handler);
      REQUIRE(test_for_pass(goto_model));
    }
  }

} //SCENARIO ends