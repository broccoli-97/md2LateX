// md_converter.h
#ifndef MD_CONVERTER_H
#define MD_CONVERTER_H

#include <string>
#include <vector>
#include <regex>
#include <map>

class MarkdownConverter {
public:
    MarkdownConverter();
    std::string convertToLatex(const std::string& markdown);

private:
    // Convert headers (# Header -> \section{Header}, ## Header -> \subsection{Header}, etc.)
    std::string convertHeaders(const std::string& line);

    // Convert bold and italic text
    std::string convertEmphasis(const std::string& line);

    // Convert code blocks and inline code
    std::string convertCodeBlocks(const std::string& line);

    // Convert lists
    std::string convertLists(const std::string& line, bool& inList, int& listDepth);

    // Convert links [text](url) -> \href{url}{text}
    std::string convertLinks(const std::string& line);

    // Convert images ![alt](url) -> \includegraphics{url}
    std::string convertImages(const std::string& line);

    // Convert blockquotes
    std::string convertBlockquotes(const std::string& line, bool& inQuote);

    // Convert citations [^1] -> \cite{ref1}
    std::string convertCitations(const std::string& line);

    // Process citation references at the end of the document [^1]: reference text
    void processCitationReferences(const std::string& markdown);

    // Generate BibTeX entries from collected references
    std::string generateBibTeX();

    // Escape LaTeX special characters
    std::string escapeLatexChars(const std::string& text);

    // Map to store citation references
    std::map<std::string, std::string> citationRefs;
};

#endif // MD_CONVERTER_H