/*******************************************************************\

Module: Goto program validation

Author: Daniel Poetzl

\*******************************************************************/

#ifndef CPROVER_UTIL_VALIDATE_H
#define CPROVER_UTIL_VALIDATE_H

#include <type_traits>

#include "exception_utils.h"
#include "invariant.h"
#include "irep.h"

class namespacet;

enum class validation_modet
{
  INVARIANT,
  EXCEPTION
};

/// This macro takes a condition which denotes a well-formedness criterion on
/// goto programs, expressions, instructions, etc. When invoking the macro, a
/// variable named `vm` of type `const validation_modet` should be in scope. It
/// indicates whether DATA_INVARIANT() is used to check the condition, or if an
/// exception is thrown when the condition does not hold.
#define DATA_CHECK(condition, message)                                         \
  do                                                                           \
  {                                                                            \
    static_assert(                                                             \
      std::is_same<decltype(vm), const validation_modet>::value,               \
      "when invoking the macro DATA_CHECK(), a variable named `vm` of type "   \
      "`const validation_modet` should be in scope which indicates the "       \
      "validation mode (see `validate.h`");                                    \
    switch(vm)                                                                 \
    {                                                                          \
    case validation_modet::INVARIANT:                                          \
      DATA_INVARIANT(condition, message);                                      \
      break;                                                                   \
    case validation_modet::EXCEPTION:                                          \
      if(!(condition))                                                         \
        throw incorrect_goto_program_exceptiont(message);                      \
      break;                                                                   \
    }                                                                          \
  } while(0)

#define DATA_CHECK_WITH_DIAGNOSTICS(condition, message, ...)                   \
  do                                                                           \
  {                                                                            \
    static_assert(                                                             \
      std::is_same<decltype(vm), const validation_modet>::value,               \
      "when invoking the macro DATA_CHECK_WITH_DIAGNOSTICS(), a variable "     \
      "named `vm` of type `const validation_modet` should be in scope which "  \
      "indicates the validation mode (see `validate.h`");                      \
    switch(vm)                                                                 \
    {                                                                          \
    case validation_modet::INVARIANT:                                          \
      DATA_INVARIANT_WITH_DIAGNOSTICS(condition, message, __VA_ARGS__);        \
      break;                                                                   \
    case validation_modet::EXCEPTION:                                          \
      if(!(condition))                                                         \
        throw incorrect_goto_program_exceptiont(message, __VA_ARGS__);         \
      break;                                                                   \
    }                                                                          \
  } while(0)

template <typename T>
struct call_checkt
{
  static_assert(std::is_base_of<irept, T>::value, "");

  void operator()(const irept &irep, const validation_modet vm)
  {
    const T &s = static_cast<const T &>(irep);
    s.check(vm);
  }
};

template <typename T>
struct call_validatet
{
  static_assert(std::is_base_of<irept, T>::value, "");

  void
  operator()(const irept &irep, const namespacet &ns, const validation_modet vm)
  {
    const T &s = static_cast<const T &>(irep);
    s.validate(ns, vm);
  }
};

template <typename T>
struct call_validate_fullt
{
  static_assert(std::is_base_of<irept, T>::value, "");

  void
  operator()(const irept &irep, const namespacet &ns, const validation_modet vm)
  {
    const T &s = static_cast<const T &>(irep);
    s.validate_full(ns, vm);
  }
};

#endif /* CPROVER_UTIL_VALIDATE_H */
