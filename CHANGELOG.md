# Changelog

All notable changes to Ephemeral Blender are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
Ephemeral Blender's version is separate from the Blender version it is based on.
Release tags are named `ephemeral-vX.Y.Z`.

## [Unreleased]

## [0.3.0] - 2026-11-08

### Added

- Ephemeral posing in Pose Mode on armatures with a Pose Sequence, through
  Blender's own move and rotate tools (snapping and numeric input included):
  - Rotate is FK: the bone turns and its children follow.
  - Move on a bone with a parent aims the parent at the new position, keeping
    bone lengths. Move on a root bone translates it.
  - Edits are written live into the Pose Sequence at the current frame and mark
    it authored. Cancel restores the frame. Each edit is one undo step.
- Aim solver in the ephemeral module.

### Changed

- Auto-keying and Auto IK are skipped for armatures with a Pose Sequence.

## [0.2.0] - 2026-11-08

### Added

- Pose Sequence data-block: stores an explicit pose per frame for an armature's
  bones, marked as empty, authored, or generated. Saved in .blend files, with
  undo through Blender's ID system, and listed in `bpy.data.pose_sequences`.
- `Object.pose_sequence`: assigns a Pose Sequence to an armature object. During
  evaluation its pose replaces the animated transforms of the bones it covers,
  converted to each bone's rotation mode. Empty frames hold the previous stored
  pose.
- `PoseSequence.init()` and `PoseSequence.set_bone_transform()` for creating and
  filling a Pose Sequence from Python.

### Notes

- Files with a Pose Sequence open in stock Blender 5.2; the Pose Sequence is
  dropped there.

## [0.1.0] - 2026-11-08

Based on Blender 5.2 LTS.

### Added

- Forked from Blender 5.2 LTS (`blender-v5.2-release` at `8a5de9e044f`).
- Ephemeral module (`source/blender/ephemeral/`) with per-bone transform
  interpolation and its first unit test.

[Unreleased]: https://github.com/TanMcGi/EphemeralBlender/compare/ephemeral-v0.3.0...HEAD
[0.3.0]: https://github.com/TanMcGi/EphemeralBlender/compare/ephemeral-v0.2.0...ephemeral-v0.3.0
[0.2.0]: https://github.com/TanMcGi/EphemeralBlender/compare/ephemeral-v0.1.0...ephemeral-v0.2.0
[0.1.0]: https://github.com/TanMcGi/EphemeralBlender/releases/tag/ephemeral-v0.1.0
