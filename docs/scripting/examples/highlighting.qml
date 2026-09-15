import QtQml 2.0
import QOwnNotesTypes 1.0

Script {
    function init() {
        // Highlight a text line like "BLOCK: some text" as blockquote (state 18)
        script.addHighlightingRule("^BLOCK: (.+)", "BLOCK:", 18);

        // Mask out (state 24) all characters after 32 characters in a line
        // capturingGroup 1 means the expression from the first bracketed part of the pattern will be highlighted
        // maskedGroup -1 means that no masking should be done
        script.addHighlightingRule("^.{32}(.+)", "", 24, 1, -1);

        // Highlight "IMPORTANT" with custom colors: red bold text on a yellow background
        // The 6th parameter is a map with custom format properties:
        //   foregroundColor, backgroundColor, bold, italic, underline, fontSize
        // Use state -1 (NoState) to use only custom formatting without a predefined state
        script.addHighlightingRule("IMPORTANT", "IMPORTANT", -1, 0, 0, {
            "foregroundColor": "#ff0000",
            "backgroundColor": "#ffff00",
            "bold": true
        });

        // Highlight "NOTE:" with custom italic green text
        script.addHighlightingRule("NOTE:", "NOTE:", -1, 0, 0, {
            "foregroundColor": "#00aa00",
            "italic": true
        });
    }

    /**
     * This hook is called for each text block during syntax highlighting.
     * It allows context-aware, dynamic highlighting that goes beyond
     * static regex rules.
     *
     * @param text - the text of the current block being highlighted
     * @param previousBlockState - the highlighter state of the previous block
     *     (-1 if this is the first block)
     * @return an array of highlight range objects, each with:
     *     start {int} - start position in the text
     *     length {int} - number of characters to highlight
     *     state {int} - the HighlighterState to use (optional, -1 for custom only)
     *     foregroundColor {string} - foreground color (optional)
     *     backgroundColor {string} - background color (optional)
     *     bold {bool} - bold (optional)
     *     italic {bool} - italic (optional)
     *     underline {bool} - underline (optional)
     *     fontSize {int} - font point size (optional)
     */
    function highlightingHook(text, previousBlockState) {
        var highlights = [];

        // Example: highlight all occurrences of "FIXME" with red underline
        var re = /FIXME/g;
        var match;
        while ((match = re.exec(text)) !== null) {
            highlights.push({
                "start": match.index,
                "length": match[0].length,
                "foregroundColor": "#ff0000",
                "underline": true,
                "bold": true
            });
        }

        return highlights;
    }
}
