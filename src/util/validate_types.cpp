/*******************************************************************\

Module: Validate types

Author: Daniel Poetzl

\*******************************************************************/

#include "validate_types.h"

#ifdef REPORT_UNIMPLEMENTED_TYPE_CHECKS
#include <iostream>
#endif

#include "namespace.h"
#include "std_types.h"
#include "type.h"
#include "validate.h"

#define CALL_ON_TYPE(type_type)                                                \
  C<type_type>()(type, std::forward<Args>(args)...)

template <template <typename> class C, typename... Args>
void call_on_type(const typet &type, Args &&... args)
{
  if(type.id() == ID_signedbv)
  {
    CALL_ON_TYPE(signedbv_typet);
  }
  else if(type.id() == ID_unsignedbv)
  {
    CALL_ON_TYPE(unsignedbv_typet);
  }
  else
  {
#ifdef REPORT_UNIMPLEMENTED_TYPE_CHECKS
    std::cerr << "Unimplemented well-formedness check for type with id: "
              << type.id_string() << std::endl;
#endif

    CALL_ON_TYPE(typet);
  }
}

/// Check that the given type is well-formed (shallow checks only, i.e.,
/// subtypes are not checked)
///
/// The function casts the given type to its concrete subtype (as determined by
/// the id() field) and then calls its check() method.
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void check_type_pick(const typet &type, const validation_modet vm)
{
  call_on_type<call_checkt>(type, vm);
}

/// Check that the given type is well-formed, assuming that its subtypes have
/// already been checked for well-formedness.
///
/// The function casts the given type to its concrete subtype (as determined by
/// the id() field) and then calls its validate() method.
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void validate_type_pick(
  const typet &type,
  const namespacet &ns,
  const validation_modet vm)
{
  call_on_type<call_validatet>(type, ns, vm);
}

/// Check that the given type is well-formed (full check, including checks of
/// subtypes)
///
/// The function casts the given type to its concrete subtype (as determined by
/// the id() field) and then calls its validate_full() method.
///
/// The validation mode indicates whether well-formedness check failures are
/// reported via DATA_INVARIANT violations or exceptions.
void validate_type_full_pick(
  const typet &type,
  const namespacet &ns,
  const validation_modet vm)
{
  call_on_type<call_validate_fullt>(type, ns, vm);
}
