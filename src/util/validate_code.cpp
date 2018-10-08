/*******************************************************************\

Module: Validate code

Author: Daniel Poetzl

\*******************************************************************/

#include "validate_code.h"

#ifdef REPORT_UNIMPLEMENTED_CODE_CHECKS
#include <iostream>
#endif

#include "namespace.h"
#include "std_code.h"
#include "validate.h"

#define CALL_ON_CODE(code_type)                                                \
  C<code_type>()(code, std::forward<Args>(args)...)

template <template <typename> class C, typename... Args>
void call_on_code(const codet &code, Args &&... args)
{
  if(code.get_statement() == ID_assign)
  {
    CALL_ON_CODE(code_assignt);
  }
  else if(code.get_statement() == ID_decl)
  {
    CALL_ON_CODE(code_declt);
  }
  else
  {
#ifdef REPORT_UNIMPLEMENTED_CODE_CHECKS
    std::cerr << "Unimplemented well-formedness check for code statement with "
                 "id: "
              << code.get_statement().id_string() << std::endl;
#endif

    CALL_ON_CODE(codet);
  }
}

/// Check that the given code statement is well-formed (shallow checks only,
/// i.e., enclosed statements, subexpressions, etc. are not checked)
///
/// The function casts the given codet statement to its concrete subtype (as
/// determined by the id() field) and then calls its check() method.
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void check_code_pick(const codet &code, const validation_modet vm)
{
  call_on_code<call_checkt>(code, vm);
}

/// Check that the given code statement is well-formed, assuming that all its
/// enclosed statements, subexpressions, etc. have already been checked for
/// well-formedness.
///
/// The function casts the given codet statement to its concrete subtype (as
/// determined by the id() field) and then calls its validate() method.
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void validate_code_pick(
  const codet &code,
  const namespacet &ns,
  const validation_modet vm)
{
  call_on_code<call_validatet>(code, ns, vm);
}

/// Check that the given code statement is well-formed (full check, including
/// checks of all subexpressions)
///
/// The function casts the given codet statement to its concrete subtype (as
/// determined by the id() field) and then calls its validate_full() method.
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void validate_code_full_pick(
  const codet &code,
  const namespacet &ns,
  const validation_modet vm)
{
  call_on_code<call_validate_fullt>(code, ns, vm);
}
