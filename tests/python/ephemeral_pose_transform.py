# SPDX-FileCopyrightText: 2026 Ephemeral Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
blender -b --factory-startup --python tests/python/ephemeral_pose_transform.py
"""

import math
import sys
import unittest

import bpy
from mathutils import Vector


def view3d_context():
    for area in bpy.context.window.screen.areas:
        if area.type == 'VIEW_3D':
            region = next(r for r in area.regions if r.type == 'WINDOW')
            return {"area": area, "region": region, "space_data": area.spaces.active}
    raise RuntimeError("No 3D Viewport")


def evaluated_pose_bone(ob, name):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    return ob.evaluated_get(depsgraph).pose.bones[name]


class PoseTransformTest(unittest.TestCase):
    """A two-bone chain: "root" from (0, 0, 0) to (0, 0, 1), then connected "tip" up to z = 2."""

    def setUp(self):
        bpy.ops.wm.read_homefile(use_factory_startup=True)
        bpy.context.scene.frame_set(1)
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
        tip.use_connect = True
        bpy.ops.object.mode_set(mode='POSE')
        self.ob.pose.bones["root"].rotation_mode = 'XYZ'

        seq = bpy.data.pose_sequences.new("Poses")
        seq.init(self.ob, 1, 10)
        self.ob.pose_sequence = seq

    def select_only(self, name):
        for pose_bone in self.ob.pose.bones:
            pose_bone.select = pose_bone.name == name

    def assert_vector(self, actual, expected):
        for a, e in zip(actual, expected):
            self.assertAlmostEqual(a, e, places=4)

    def test_grab_child_aims_parent(self):
        self.select_only("tip")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.translate(value=(1.0, 0.0, -1.0))

        # The tip's head (the root's tail) went to (1, 0, 0): the root now points along +X.
        self.assert_vector(evaluated_pose_bone(self.ob, "root").tail, (1.0, 0.0, 0.0))
        self.assert_vector(evaluated_pose_bone(self.ob, "tip").head, (1.0, 0.0, 0.0))
        self.assert_vector(evaluated_pose_bone(self.ob, "tip").tail, (2.0, 0.0, 0.0))

    def test_grab_keeps_bone_length(self):
        self.select_only("tip")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.translate(value=(5.0, 0.0, -1.0))
        self.assert_vector(evaluated_pose_bone(self.ob, "root").tail, (1.0, 0.0, 0.0))

    def test_grab_root_translates(self):
        self.select_only("root")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.translate(value=(0.0, 2.0, 0.0))
        self.assert_vector(evaluated_pose_bone(self.ob, "root").head, (0.0, 2.0, 0.0))
        self.assert_vector(evaluated_pose_bone(self.ob, "tip").tail, (0.0, 2.0, 2.0))

    def test_rotate_is_fk(self):
        self.select_only("root")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.rotate(value=math.pi / 2, orient_axis='X')
        root_tail = evaluated_pose_bone(self.ob, "root").tail
        tip_tail = evaluated_pose_bone(self.ob, "tip").tail
        # Quarter turn around X: the chain now lies along Y, the child following.
        self.assertAlmostEqual(abs(root_tail.y), 1.0, places=4)
        self.assertAlmostEqual(root_tail.z, 0.0, places=4)
        self.assert_vector(tip_tail, root_tail * 2.0)

    def test_edit_is_stored_and_held(self):
        self.select_only("tip")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.translate(value=(1.0, 0.0, -1.0))
        bpy.context.scene.frame_set(6)
        self.assert_vector(evaluated_pose_bone(self.ob, "root").tail, (1.0, 0.0, 0.0))

    def test_no_keys_inserted(self):
        bpy.context.scene.tool_settings.use_keyframe_insert_auto = True
        self.select_only("tip")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.translate(value=(1.0, 0.0, -1.0))
        self.assertIsNone(self.ob.animation_data and self.ob.animation_data.action)

    def test_undo(self):
        # Memfile undo is not initialized by default in background mode.
        bpy.ops.ed.undo_push()
        bpy.ops.ed.undo_push()
        self.select_only("tip")
        with bpy.context.temp_override(**view3d_context()):
            bpy.ops.transform.translate(value=(1.0, 0.0, -1.0))
        bpy.ops.ed.undo_push()
        bpy.ops.ed.undo()
        self.ob = bpy.data.objects["Rig"]
        self.assert_vector(evaluated_pose_bone(self.ob, "root").tail, (0.0, 0.0, 1.0))


if __name__ == "__main__":
    sys.argv = [__file__] + (sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
    unittest.main()
