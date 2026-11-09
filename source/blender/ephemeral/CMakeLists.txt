# SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Solver and pose logic for ephemeral rigging. May use blenlib and DNA,
# never RNA, editors, or UI.

set(INC
  PUBLIC .
)

set(INC_SYS
)

set(SRC
  intern/pose.cc
  intern/pose_sequence.cc
  intern/solver.cc

  EPH_pose.hh
  EPH_pose_sequence.hh
  EPH_solver.hh
)

set(LIB
  PUBLIC bf::blenlib
  PUBLIC bf::dna
  PRIVATE bf::intern::guardedalloc
)

blender_add_lib(bf_ephemeral "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
add_library(bf::ephemeral ALIAS bf_ephemeral)

if(WITH_GTESTS)
  set(TEST_INC
  )
  set(TEST_SRC
    intern/pose_sequence_test.cc
    intern/pose_test.cc
    intern/solver_test.cc
  )
  set(TEST_LIB
    PRIVATE bf::ephemeral
  )
  blender_add_test_suite_lib(ephemeral "${TEST_SRC}" "${INC};${TEST_INC}" "${INC_SYS}" "${LIB};${TEST_LIB}")
endif()
