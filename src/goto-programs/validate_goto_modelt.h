//
// Created by Sonny Martin on 07/10/2018.
//

#ifndef PROJECT_VALIDATE_GOTO_MODELT_H
#define PROJECT_VALIDATE_GOTO_MODELT_H

#include <util/ui_message.h>

class goto_modelt;

// if complex options to be passed, final signature proposed along lines of:
// 'validate_goto_model(goto_model, options, log);'
// similar to process_goto_program(goto_model, options, log)
// (or some hybrid thereof)
bool validate_goto_model(
  const goto_modelt &,
  message_handlert &message_handler,
  ui_message_handlert::uit ui
);

#endif //PROJECT_VALIDATE_GOTO_MODELT_H
