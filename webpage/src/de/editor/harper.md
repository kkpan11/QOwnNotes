# Harper

QOwnNotes can use [Harper](https://writewithharper.com/) for optional
**offline grammar and style checking** in the note editor.

It complements the existing [spellchecking](spellchecking.md) support instead of replacing it.

![harper-demo](/img/editor/harper.webp)

## Features

- Colored underlines for detected grammar, style, punctuation, and wording issues
- Context-menu suggestions for replacements
- `Ignore this rule` and `Ignore word` actions in the context menu
- A quick toggle in the **Edit** menu with `Check grammar with Harper`
- Local checking with the `harper-ls` language server over `Stdio` or `TCP`

## Setup

Open `Settings` and select `Editor -> Harper`.

- Turn on `Enable offline grammar and style checking with Harper`
- Choose the `Transport`
  - `Stdio (recommended)` starts a local `harper-ls` process directly
  - `TCP` connects to an already running `harper-ls` server
- For `Stdio`, set the `Command / binary path`
  - The default command is `harper-ls`
  - `Auto-detect` searches for it in your `PATH`
- For `TCP`, set the `Server address` and `Port`
  - The defaults are `127.0.0.1` and `4000`
- Choose the `Dialect`
  - `American`
  - `British`
  - `Australian`
  - `Canadian`
  - `Indian`
- Adjust the `Check delay` to control how long QOwnNotes waits after typing before sending a request
- Select which linters should be enabled
  - `Spell Check`
  - `Sentence Capitalization`
  - `Repeated Words`
  - `Long Sentences`
  - `An vs A`
  - `Unclosed Quotes`
  - `Correct Number Suffix`
  - `Spaces`
  - `Quote Spacing`
  - `No French Spaces`
  - `Wrong Apostrophe`
  - `Spelled Numbers`

Use `Test Connection` to verify that QOwnNotes can either start `harper-ls` in `Stdio` mode or
reach the configured server in `TCP` mode.

## How It Works

- QOwnNotes checks visible editor blocks instead of the whole document at once
- Empty lines, headings, and code blocks are skipped
- Requests are debounced so Harper is not queried on every keystroke
- Results are shown inline in the Markdown editor
- Harper and LanguageTool can be enabled at the same time

## Notes

- Harper support is optional and depends on the feature being enabled in your build
- `Stdio` mode is the simplest setup for local offline checking
- If Harper cannot be reached, QOwnNotes shows a warning and stops checking until it is available again

## Related

- [Spellchecking](spellchecking.md)
- [LanguageTool](languagetool.md)
- [Concept](../getting-started/concept.md)
