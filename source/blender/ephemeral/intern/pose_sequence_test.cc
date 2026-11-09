/* SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include <cmath>

#include "MEM_guardedalloc.h"

#include "BLI_math_constants.h"
#include "BLI_string.h"

#include "DNA_action_types.h"
#include "DNA_armature_types.h"
#include "DNA_pose_sequence_types.h"

#include "EPH_pose_sequence.hh"

#include "testing/testing.h"

namespace blender::ephemeral::tests {

class PoseSequenceTest : public ::testing::Test {
 protected:
  /* Armature "root" with one child "tip". */
  Bone root;
  Bone tip;
  bArmature armature;
  PoseSequence seq;

  void SetUp() override
  {
    STRNCPY(root.name, "root");
    STRNCPY(tip.name, "tip");
    root.childbase.first = root.childbase.last = &tip;
    armature.bonebase.first = armature.bonebase.last = &root;
    pose_sequence_init(seq, armature, 10, 5);
  }

  void TearDown() override
  {
    MEM_SAFE_DELETE(seq.bones);
    MEM_SAFE_DELETE(seq.frame_states);
    MEM_SAFE_DELETE(seq.transforms);
  }

  PoseSequenceTransform &transform(const int frame_index, const int bone)
  {
    return seq.transforms[frame_index * seq.bone_count + bone];
  }
};

TEST_F(PoseSequenceTest, init_covers_all_bones_and_frames)
{
  EXPECT_EQ(seq.frame_start, 10);
  EXPECT_EQ(seq.frame_count, 5);
  ASSERT_EQ(seq.bone_count, 2);
  EXPECT_STREQ(seq.bones[0].name, "root");
  EXPECT_STREQ(seq.bones[1].name, "tip");
  for (int i = 0; i < seq.frame_count; i++) {
    EXPECT_EQ(seq.frame_states[i], POSE_SEQUENCE_FRAME_EMPTY);
  }
  EXPECT_EQ(transform(4, 1).rotation[0], 1.0f);
  EXPECT_EQ(transform(4, 1).scale[2], 1.0f);
}

TEST_F(PoseSequenceTest, frame_to_show_holds_previous_pose)
{
  seq.frame_states[1] = POSE_SEQUENCE_FRAME_AUTHORED;
  seq.frame_states[3] = POSE_SEQUENCE_FRAME_GENERATED;

  EXPECT_EQ(pose_sequence_frame_to_show(seq, 10), -1); /* Before anything is stored. */
  EXPECT_EQ(pose_sequence_frame_to_show(seq, 11), 1);
  EXPECT_EQ(pose_sequence_frame_to_show(seq, 12), 1); /* Held. */
  EXPECT_EQ(pose_sequence_frame_to_show(seq, 13), 3);
  EXPECT_EQ(pose_sequence_frame_to_show(seq, 40), 3);  /* Past the end. */
  EXPECT_EQ(pose_sequence_frame_to_show(seq, -5), -1); /* Before the start. */
}

TEST_F(PoseSequenceTest, store_bone_starts_from_held_pose)
{
  PoseSequenceTransform moved;
  moved.location[1] = 2.0f;
  ASSERT_TRUE(pose_sequence_store_bone(seq, 11, "root", moved));

  PoseSequenceTransform scaled;
  scaled.scale[0] = 3.0f;
  ASSERT_TRUE(pose_sequence_store_bone(seq, 13, "tip", scaled));

  EXPECT_EQ(seq.frame_states[1], POSE_SEQUENCE_FRAME_AUTHORED);
  EXPECT_EQ(seq.frame_states[2], POSE_SEQUENCE_FRAME_EMPTY);
  EXPECT_EQ(seq.frame_states[3], POSE_SEQUENCE_FRAME_AUTHORED);
  EXPECT_EQ(transform(3, 0).location[1], 2.0f); /* Root copied from the held frame 11. */
  EXPECT_EQ(transform(3, 1).scale[0], 3.0f);
  EXPECT_EQ(transform(1, 1).scale[0], 1.0f); /* Frame 11 untouched. */

  EXPECT_FALSE(pose_sequence_store_bone(seq, 13, "missing", scaled));
  EXPECT_FALSE(pose_sequence_store_bone(seq, 15, "tip", scaled)); /* Past the end. */
  EXPECT_FALSE(pose_sequence_store_bone(seq, 9, "tip", scaled));  /* Before the start. */
}

TEST_F(PoseSequenceTest, apply_converts_to_channel_rotation_mode)
{
  /* Frame 11: root moved, tip turned 90 degrees around Z. */
  seq.frame_states[1] = POSE_SEQUENCE_FRAME_AUTHORED;
  transform(1, 0).location[1] = 2.0f;
  transform(1, 1).rotation[0] = std::cos(float(M_PI_4));
  transform(1, 1).rotation[3] = std::sin(float(M_PI_4));

  bPoseChannel root_chan;
  bPoseChannel tip_chan;
  bPoseChannel other_chan;
  STRNCPY(root_chan.name, "root");
  STRNCPY(tip_chan.name, "tip");
  STRNCPY(other_chan.name, "other");
  root_chan.rotmode = ROT_MODE_QUAT;
  tip_chan.rotmode = ROT_MODE_XYZ;
  other_chan.loc[0] = 7.0f;
  bPose pose;
  pose.chanbase.first = &root_chan;
  root_chan.next = &tip_chan;
  tip_chan.prev = &root_chan;
  tip_chan.next = &other_chan;
  other_chan.prev = &tip_chan;
  pose.chanbase.last = &other_chan;

  pose_sequence_apply(seq, 12, pose);

  EXPECT_EQ(root_chan.loc[1], 2.0f);
  EXPECT_EQ(root_chan.quat[0], 1.0f);
  EXPECT_NEAR(tip_chan.eul[0], 0.0f, 1e-6f);
  EXPECT_NEAR(tip_chan.eul[1], 0.0f, 1e-6f);
  EXPECT_NEAR(tip_chan.eul[2], float(M_PI_2), 1e-6f);
  EXPECT_EQ(other_chan.loc[0], 7.0f); /* Not in the sequence: untouched. */
}

}  // namespace blender::ephemeral::tests
