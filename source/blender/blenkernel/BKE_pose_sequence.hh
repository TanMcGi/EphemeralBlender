/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup bke
 */

namespace blender {

struct Main;
struct PoseSequence;

PoseSequence *BKE_pose_sequence_add(Main *bmain, const char *name);

}  // namespace blender
