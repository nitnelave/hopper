find_program( MEMORYCHECK_COMMAND valgrind )
set( MEMORYCHECK_COMMAND_OPTIONS
  "--trace-children=yes --leak-check=full --error-exitcode=1" )
set(MEMCHECK_COMMAND "${MEMORYCHECK_COMMAND} ${MEMORYCHECK_COMMAND_OPTIONS}")
separate_arguments(MEMCHECK_COMMAND)

function(add_memcheck_test name binary)
  add_test(NAME ${name} COMMAND ${binary} ${ARGN})
  if (DEFINED MEMORYCHECK_COMMAND)
    add_test(NAME memcheck_${name} COMMAND ${MEMCHECK_COMMAND} $<TARGET_FILE:${binary}> ${ARGN})
  endif(DEFINED MEMORYCHECK_COMMAND)
endfunction(add_memcheck_test)

function(set_memcheck_test_properties name)
  set_tests_properties(${name} ${ARGN})
  if (DEFINED MEMORYCHECK_COMMAND)
    set_tests_properties(memcheck_${name} ${ARGN})
  endif(DEFINED MEMORYCHECK_COMMAND)
endfunction(set_memcheck_test_properties)
