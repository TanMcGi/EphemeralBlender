# SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
blender -b --factory-startup --python tests/python/ephemeral_pose_sequence.py
"""

import math
import pathlib
import sys
import tempfile
import unittest

import bpy

QUARTER_TURN_Z = (math.cos(math.pi / 4), 0.0, 0.0, math.sin(math.pi / 4))
IDENTITY = (1.0, 0.0, 0.0, 0.0)


def evaluated_pose_bone(ob, name):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    return ob.evaluated_get(depsgraph).pose.bones[name]


class PoseSequenceTest(unittest.TestCase):

    def setUp(self):
        bpy.ops.wm.read_homefile(use_factory_startup=True)
        armature = bpy.data.armatures.new("Armature")
        self.ob = bpy.data.objects.new("Rig", armature)
        bpy.context.scene.collection.objects.link(self.ob)
        bpy.context.view_layer.objects.active = self.ob
        bpy.ops.object.mode_set(mode='EDIT')
        root = armature.edit_bones.new("root")
        root.tail = (0.0, 0.0, 1.0)
        tip = armature.edit_bones.new("tip")
        tip.head = (0.0, 0.0, 1.0)
        tip.tail = (0.0, 0.0, 2.0)
        tip.parent = root
        bpy.ops.object.mode_set(mode='OBJECT')
        self.ob.pose.bones["tip"].rotation_mode = 'XYZ'

        self.seq = bpy.data.pose_sequences.new("Poses")
        self.seq.init(self.ob, 1, 10)
        self.ob.pose_sequence = self.seq

    def store_test_pose(self):
        self.seq.set_bone_transform(3, "root", (0.0, 1.0, 0.0), IDENTITY, (1.0, 1.0, 1.0))
        self.seq.set_bone_transform(3, "tip", (0.0, 0.0, 0.0), QUARTER_TURN_Z, (1.0, 1.0, 1.0))

    def assert_test_pose_at(self, frame):
        bpy.context.scene.frame_set(frame)
        self.assertAlmostEqual(evaluated_pose_bone(self.ob, "root").location.y, 1.0, places=5)
        # Stored as a quaternion, shown in the bone's own Euler mode.
        self.assertAlmostEqual(
            evaluated_pose_bone(self.ob, "tip").rotation_euler.z, math.pi / 2, places=5)

    def test_init(self):
        self.assertEqual(self.seq.frame_start, 1)
        self.assertEqual(self.seq.frame_count, 10)
        self.assertEqual(self.seq.bone_count, 2)
        self.assertEqual(self.seq.users, 1)

    def test_rest_before_first_stored_pose(self):
        self.store_test_pose()
        bpy.context.scene.frame_set(2)
        self.assertAlmostEqual(evaluated_pose_bone(self.ob, "root").location.y, 0.0, places=5)

    def test_stored_pose_and_hold(self):
        self.store_test_pose()
        self.assert_test_pose_at(3)
        self.assert_test_pose_at(7)

    def test_save_and_reload(self):
        self.store_test_pose()
        with tempfile.TemporaryDirectory() as tmp:
            path = str(pathlib.Path(tmp) / "pose_sequence.blend")
            bpy.ops.wm.save_as_mainfile(filepath=path)
            bpy.ops.wm.open_mainfile(filepath=path)
            self.ob = bpy.data.objects["Rig"]
            self.assertEqual(self.ob.pose_sequence.name, "Poses")
            self.assertEqual(self.ob.pose_sequence.bone_count, 2)
            self.assert_test_pose_at(5)

    def test_undo_and_redo(self):
        # Memfile undo is not initialized by default in background mode.
        bpy.ops.ed.undo_push()
        bpy.ops.ed.undo_push()
        self.store_test_pose()
        bpy.ops.ed.undo_push()
        self.assert_test_pose_at(3)

        bpy.ops.ed.undo()
        self.ob = bpy.data.objects["Rig"]
        bpy.context.scene.frame_set(3)
        self.assertAlmostEqual(evaluated_pose_bone(self.ob, "root").location.y, 0.0, places=5)

        bpy.ops.ed.redo()
        self.ob = bpy.data.objects["Rig"]
        self.assert_test_pose_at(3)

    def test_unknown_bone_reports_error(self):
        with self.assertRaises(RuntimeError):
            self.seq.set_bone_transform(3, "missing", (0.0, 0.0, 0.0), IDENTITY, (1.0, 1.0, 1.0))


if __name__ == "__main__":
    sys.argv = [__file__] + (sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
    unittest.main()
