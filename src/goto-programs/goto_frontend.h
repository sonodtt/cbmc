/*******************************************************************\

Module: A Front-end that sources Goto Programs

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_GOTO_PROGRAMS_GOTO_FRONTEND_H
#define CPROVER_GOTO_PROGRAMS_GOTO_FRONTEND_H

#include "goto_functions.h"

class goto_frontendt
{
public:
  virtual ~goto_frontendt()
  {
  }

  using goto_functiont=goto_functionst::goto_functiont;

  // get the goto-function with the given identifier
  virtual goto_functiont &&operator()(const irep_idt &) = 0;
};

#endif // CPROVER_GOTO_PROGRAMS_GOTO_FRONTEND_H
