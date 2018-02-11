/*******************************************************************\

Module: ANSI-C Frontend Interface

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_ANSI_C_ANSI_C_FRONTEND_H
#define CPROVER_ANSI_C_ANSI_C_FRONTEND_H

#include <goto-programs/goto_frontend.h>

/*! \defgroup gr_ansi_c ANSI-C front-end */

class ansi_c_frontendt:public goto_frontendt
{
public:
  goto_functiont &&operator()(const irep_idt &) = 0;
};

#endif // CPROVER_ANSI_C_ANSI_C_FRONTEND_H
