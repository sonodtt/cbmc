/*******************************************************************\

Module: Validate expressions

Author: Daniel Poetzl

\*******************************************************************/

#ifndef CPROVER_UTIL_VALIDATE_EXPRESSIONS_H
#define CPROVER_UTIL_VALIDATE_EXPRESSIONS_H

class exprt;
class namespacet;
enum class validation_modet;

void check_expr_pick(const exprt &expr, const validation_modet vm);

void validate_expr_pick(
  const exprt &expr,
  const namespacet &ns,
  const validation_modet vm);

void validate_expr_full_pick(
  const exprt &expr,
  const namespacet &ns,
  const validation_modet vm);

#endif /* CPROVER_UTIL_VALIDATE_EXPRESSIONS_H */
