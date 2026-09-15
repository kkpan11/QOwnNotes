# Code contributions

Thank you for contributing code to QOwnNotes.

## Pull requests

### Open an issue first

Please open an issue before opening a pull request.

This helps discuss the problem, proposed solution, scope, and possible side effects before code is reviewed.

The only exception is something obviously trivial, like a typo, a very small documentation fix,
or another tiny change that does not need prior discussion.

If there is already an existing issue for your work, reference it in your pull request.

### Branches and pull requests

Please open pull requests against the `main` branch.

Before you start your work, create a new branch for your change and open the pull request from that branch.

Please do not open pull requests from your own `main` branch, as that can cause problems with keeping your
fork up to date and can lead to accidentally including unrelated changes in the pull request.

### Translation contributions

Please do not make translation contributions by changing the translation files directly.
Translation files are generated automatically, so translation work should go through the
[translation page](translation.md).

## Commit messages

Please follow the existing commit message style used in this repository.

Recent examples:

- `#1765 misc: optionally anonymize personal information in debug output`
- `#1789 misc: add remembered file manager open prompts`
- `#3568 mainwindow: allow dragging text file to import as note`
- `ci: fix step name`
- `release: bump version`

In general, commit messages should:

- be short and descriptive
- use a scope followed by a colon, like `misc:`, `mainwindow:`, `tests:`, or `ci:`
- use an imperative description after the colon, like `fix`, `add`, `allow`, `update`, or `refactor`
- include the issue number at the start when there is one, for example `#3568 mainwindow: allow dragging text file to import as note`

Please keep commits focused, so each commit represents one logical change.
