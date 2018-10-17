/*******************************************************************\

Module: Validate Goto Programs

Author: Diffblue

Date: Oct 2018

\*******************************************************************/
#ifndef CPROVER_GOTO_PROGRAMS_VALIDATE_GOTO_MODELT_H
#define CPROVER_GOTO_PROGRAMS_VALIDATE_GOTO_MODELT_H

#include "validate.h"

class goto_functionst;

void validate_goto_model(
  const goto_functionst &goto_functions,
  const validation_modet vm,
  bool check_function_id = true,
  bool check_no_fn_ptr_calls = true,
  bool check_no_returns = true);

#endif //CPROVER_GOTO_PROGRAMS_VALIDATE_GOTO_MODELT_H
