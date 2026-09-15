---
title: Layouts and layout presets
description: Workspaces were renamed to layouts and the preset page was renamed to layout presets.
date: 2026-04-14
order: 20260414000000
---

# Layouts and layout presets

<BlogDate v-bind:fm="$frontmatter" />

QOwnNotes now uses clearer names for interface arrangements.

The old **Workspaces** feature is now called **Layouts**.

The old **Layout** settings page is now called **Layout presets**.

This keeps the distinction simpler:

- **Layouts** are your saved interface arrangements
- **Layout presets** are the built-in starting points you can apply to create a layout

Existing saved setups are migrated automatically, including the current layout, the previous layout, layout names, and layout selector shortcuts.

For scripting, the new hook name is `layoutSwitchedHook(oldUuid, newUuid)`.
Older scripts using `workspaceSwitchedHook(oldUuid, newUuid)` will still work for now, but they are deprecated and should be updated.

If you want to follow the discussion behind the rename, please take a look at [GitHub issue #3564](https://github.com/pbek/QOwnNotes/issues/3564).
