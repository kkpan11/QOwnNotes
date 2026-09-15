import QtQml 2.0
import QOwnNotesTypes 1.0

/**
 * This script demonstrates the custom highlighting features:
 * 1. Static rules with custom colors/styles via addHighlightingRule()
 * 2. Dynamic per-block highlighting via highlightingHook()
 */
Script {
    function init() {
        // Highlight "TODO" with bold white text on a red background
        script.addHighlightingRule("TODO", "TODO", -1, 0, 0, {
            "foregroundColor": "#ffffff",
            "backgroundColor": "#cc0000",
            "bold": true
        });

        // Highlight "DONE" with green strikethrough-like italic text
        script.addHighlightingRule("DONE", "DONE", -1, 0, 0, {
            "foregroundColor": "#22aa22",
            "italic": true
        });

        // Highlight lines starting with ">" and containing "IMPORTANT"
        // using blockquote state (18) with a custom red foreground color
        script.addHighlightingRule("^>.*IMPORTANT.*", "IMPORTANT", 18, 0, 0, {
            "foregroundColor": "#ff4444"
        });

        // Highlight "@username" mentions with a custom color and underline
        script.addHighlightingRule("@\\w+", "@", -1, 0, 0, {
            "foregroundColor": "#3366cc",
            "underline": true
        });
    }

    /**
     * Dynamic per-block highlighting hook.
     *
     * This function is called for every text block in the editor during
     * syntax highlighting. It receives the block text and the previous
     * block's highlighter state, allowing context-aware highlighting.
     *
     * @param text the text of the current block
     * @param previousBlockState the state of the previous block (-1 for the first block)
     * @return an array of highlight range objects
     */
    function highlightingHook(text, previousBlockState) {
        var highlights = [];

        // Highlight "#tag" style tags with a custom teal color
        var tagRegex = /#[a-zA-Z]\w*/g;
        var match;
        while ((match = tagRegex.exec(text)) !== null) {
            highlights.push({
                "start": match.index,
                "length": match[0].length,
                "foregroundColor": "#008080",
                "bold": true
            });
        }

        // Highlight "FIXME" and "HACK" keywords with red underlined text
        var keywordRegex = /\b(FIXME|HACK)\b/g;
        while ((match = keywordRegex.exec(text)) !== null) {
            highlights.push({
                "start": match.index,
                "length": match[0].length,
                "foregroundColor": "#ff0000",
                "underline": true,
                "bold": true
            });
        }

        // Highlight dates in YYYY-MM-DD format with a purple color
        var dateRegex = /\b\d{4}-\d{2}-\d{2}\b/g;
        while ((match = dateRegex.exec(text)) !== null) {
            highlights.push({
                "start": match.index,
                "length": match[0].length,
                "foregroundColor": "#8855cc"
            });
        }

        return highlights;
    }
}
