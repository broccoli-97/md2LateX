#include <iostream>
#include <sstream>

#include "md_converter.h"
#include "paper_cition_api.h"

MarkdownConverter::MarkdownConverter()
{
    // Constructor can be used for any initialization if needed
}

std::string MarkdownConverter::convertToLatex(const std::string &markdown)
{
    // First, process all citation references
    processCitationReferences(markdown);

    std::stringstream result;
    std::istringstream stream(markdown);
    std::string line;
    bool inList = false;
    int listDepth = 0;
    bool inQuote = false;
    bool inCodeBlock = false;
    bool inCitationSection = false;
    std::string codeBlockContent;
    std::string codeBlockLanguage;

    // Add LaTeX document preamble
    result << "\\documentclass{article}\n";
    result << "\\usepackage{hyperref}\n";
    result << "\\usepackage{graphicx}\n";
    result << "\\usepackage{listings}\n";
    result << "\\usepackage{xcolor}\n";
    result << "\\usepackage{enumitem}\n";
    result << "\\usepackage{geometry}\n";
    result << "\\usepackage{natbib}  % For citations\n";
    result << "\\geometry{margin=1in}\n";
    result << "\n\\begin{document}\n\n";

    while (std::getline(stream, line))
    {
        // Skip lines that are citation references
        if (line.find("[^") == 0 && line.find("]:") != std::string::npos)
        {
            inCitationSection = true;
            continue;
        }

        // If we've entered the citation section, we should skip all remaining lines
        if (inCitationSection)
        {
            continue;
        }

        // Check for code blocks (```...)
        if (line.find("```") == 0)
        {
            if (!inCodeBlock)
            {
                inCodeBlock = true;
                codeBlockLanguage = line.substr(3);
                continue;
            }
            else
            {
                inCodeBlock = false;
                result << "\\begin{lstlisting}[language="
                       << (codeBlockLanguage.empty() ? "text" : codeBlockLanguage) << "]\n";
                result << codeBlockContent;
                result << "\\end{lstlisting}\n\n";
                codeBlockContent.clear();
                codeBlockLanguage.clear();
                continue;
            }
        }

        if (inCodeBlock)
        {
            codeBlockContent += line + "\n";
            continue;
        }

        // Process regular markdown
        if (!line.empty())
        {
            // First check for headers
            if (line[0] == '#')
            {
                if (inList)
                {
                    result << "\\end{itemize}\n\n";
                    inList = false;
                    listDepth = 0;
                }
                if (inQuote)
                {
                    result << "\\end{quotation}\n\n";
                    inQuote = false;
                }
                result << convertHeaders(line) << "\n\n";
            }
            // Check for lists
            else if (line[0] == '-' || line[0] == '*' || line[0] == '+' ||
                     (line.size() >= 2 && std::isdigit(line[0]) && line[1] == '.'))
            {
                result << convertLists(line, inList, listDepth) << "\n";
            }
            // Check for blockquotes
            else if (line[0] == '>')
            {
                result << convertBlockquotes(line, inQuote) << "\n";
            }
            // Regular text
            else
            {
                if (inList)
                {
                    result << "\\end{itemize}\n\n";
                    inList = false;
                    listDepth = 0;
                }
                if (inQuote)
                {
                    result << "\\end{quotation}\n\n";
                    inQuote = false;
                }

                // Process links, images, emphasis, inline code, and citations
                std::string processedLine = line;
                processedLine = convertLinks(processedLine);
                processedLine = convertImages(processedLine);
                processedLine = convertEmphasis(processedLine);
                processedLine = convertCodeBlocks(processedLine);
                processedLine = convertCitations(processedLine);
                processedLine = escapeLatexChars(processedLine);

                result << processedLine << "\n\n";
            }
        }
        else
        {
            result << "\n";
        }
    }

    // Close any open environments
    if (inList)
    {
        result << "\\end{itemize}\n";
    }
    if (inQuote)
    {
        result << "\\end{quotation}\n";
    }

    // Add bibliography
    if (!citationRefs.empty())
    {
        result << "\\bibliographystyle{plain}\n";
        result << "\\bibliography{references}\n";

        //// Generate BibTeX file
        // std::ofstream bibFile("references.bib");
        // if (bibFile)
        //{
        //     bibFile << generateBibTeX();
        //     bibFile.close();
        // }
        generateBibTeX();
    }

    // Close the document
    result << "\\end{document}\n";

    return result.str();
}

std::string MarkdownConverter::convertHeaders(const std::string &line)
{
    // Count the number of # symbols
    size_t level = 0;
    while (level < line.size() && line[level] == '#')
    {
        level++;
    }

    // Extract the header text
    std::string headerText = line.substr(level);
    // Trim leading spaces
    headerText.erase(0, headerText.find_first_not_of(" \t"));

    // Map the header level to LaTeX section commands
    std::string latexCommand;
    switch (level)
    {
    case 1:
        latexCommand = "\\section{";
        break;
    case 2:
        latexCommand = "\\subsection{";
        break;
    case 3:
        latexCommand = "\\subsubsection{";
        break;
    case 4:
        latexCommand = "\\paragraph{";
        break;
    case 5:
    case 6:
        latexCommand = "\\subparagraph{";
        break;
    default:
        latexCommand = "\\section{";
    }

    return latexCommand + headerText + "}";
}

std::string MarkdownConverter::convertEmphasis(const std::string &line)
{
    std::string result = line;

    // Bold: **text** or __text__ -> \textbf{text}
    std::regex boldPattern1("\\*\\*(.*?)\\*\\*");
    result = std::regex_replace(result, boldPattern1, "\\\\textbf{$1}");

    std::regex boldPattern2("__(.*?)__");
    result = std::regex_replace(result, boldPattern2, "\\\\textbf{$1}");

    // Italic: *text* or _text_ -> \textit{text}
    std::regex italicPattern1("\\*(.*?)\\*");
    result = std::regex_replace(result, italicPattern1, "\\\\textit{$1}");

    std::regex italicPattern2("_(.*?)_");
    result = std::regex_replace(result, italicPattern2, "\\\\textit{$1}");

    return result;
}

std::string MarkdownConverter::convertCodeBlocks(const std::string &line)
{
    std::string result = line;

    // Inline code: `code` -> \texttt{code}
    std::regex inlineCodePattern("`(.*?)`");
    result = std::regex_replace(result, inlineCodePattern, "\\\\texttt{$1}");

    return result;
}

std::string MarkdownConverter::convertLists(const std::string &line, bool &inList, int &listDepth)
{
    // If not already in a list, start one
    std::string result;
    if (!inList)
    {
        result = "\\begin{itemize}\n";
        inList = true;
        listDepth = 1;
    }

    // Calculate indentation level
    int currentDepth = 0;
    size_t i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
    {
        currentDepth += (line[i] == ' ') ? 1 : 4; // Count spaces and tabs
        i++;
    }
    currentDepth = (currentDepth / 2) + 1; // Normalize depth

    // Adjust list depth if needed
    while (currentDepth > listDepth)
    {
        result += "\\begin{itemize}\n";
        listDepth++;
    }
    while (currentDepth < listDepth)
    {
        result += "\\end{itemize}\n";
        listDepth--;
    }

    // Extract list item text (skip the bullet/number and space)
    std::string itemText;
    std::regex listPattern("^\\s*[\\*\\-\\+]\\s+(.*)$");     // For bullet lists
    std::regex numberedListPattern("^\\s*\\d+\\.\\s+(.*)$"); // For numbered lists

    std::smatch matches;
    if (std::regex_match(line, matches, listPattern) ||
        std::regex_match(line, matches, numberedListPattern))
    {
        itemText = matches[1].str();
    }
    else
    {
        // Fallback if regex doesn't match
        size_t textStart = line.find_first_of("-*+1234567890");
        if (textStart != std::string::npos)
        {
            textStart = line.find_first_not_of("-*+1234567890. ", textStart);
            if (textStart != std::string::npos)
            {
                itemText = line.substr(textStart);
            }
            else
            {
                itemText = "";
            }
        }
    }

    // Process the item text for other markdown elements
    itemText = convertLinks(itemText);
    itemText = convertEmphasis(itemText);
    itemText = convertCodeBlocks(itemText);
    itemText = convertCitations(itemText);
    itemText = escapeLatexChars(itemText);

    result += "\\item " + itemText + "\n";

    return result;
}

std::string MarkdownConverter::convertLinks(const std::string &line)
{
    std::string result = line;

    // [text](url) -> \href{url}{text}
    std::regex linkPattern("\\[(.*?)\\]\\((.*?)\\)");
    result = std::regex_replace(result, linkPattern, "\\\\href{$2}{$1}");

    return result;
}

std::string MarkdownConverter::convertImages(const std::string &line)
{
    std::string result = line;

    // ![alt](url) -> \includegraphics{url}
    std::regex imagePattern("!\\[(.*?)\\]\\((.*?)\\)");
    result = std::regex_replace(
        result, imagePattern,
        "\\\\begin{figure}\n\\\\centering\n\\\\includegraphics{$2}\n\\\\caption{$"
        "1}\n\\\\end{figure}");

    return result;
}

std::string MarkdownConverter::convertBlockquotes(const std::string &line, bool &inQuote)
{
    // Extract the quote text
    std::string quoteText = line.substr(1);
    // Trim leading spaces
    quoteText.erase(0, quoteText.find_first_not_of(" \t"));

    // Process the quote text for other markdown elements
    quoteText = convertLinks(quoteText);
    quoteText = convertEmphasis(quoteText);
    quoteText = convertCodeBlocks(quoteText);
    quoteText = convertCitations(quoteText);
    quoteText = escapeLatexChars(quoteText);

    std::string result;
    if (!inQuote)
    {
        result = "\\begin{quotation}\n";
        inQuote = true;
    }

    result += quoteText + "\n";

    return result;
}

std::string MarkdownConverter::convertCitations(const std::string &line)
{
    std::string result = line;

    // Replace [^1] with \cite{ref1}
    std::regex citationPattern("\\[\\^(\\d+)\\]");
    std::smatch matches;
    std::string::const_iterator searchStart(result.cbegin());

    while (std::regex_search(searchStart, result.cend(), matches, citationPattern))
    {
        std::string refNum = matches[1].str();
        std::string citation = "\\cite{ref" + refNum + "}";

        // Replace this occurrence
        result.replace(matches.position() + (searchStart - result.cbegin()), matches.length(),
                       citation);

        // Move to the next position
        searchStart = result.cbegin() + matches.position() + citation.length();
    }

    return result;
}

void MarkdownConverter::processCitationReferences(const std::string &markdown)
{
    std::istringstream stream(markdown);
    std::string line;

    // Regular expression to match citation references like [^1]: reference text
    std::regex refPattern("\\[\\^(\\d+)\\]:\\s*(.+)");

    while (std::getline(stream, line))
    {
        std::smatch matches;
        if (std::regex_match(line, matches, refPattern))
        {
            std::string refNum = matches[1].str();
            std::string refText = matches[2].str();

            // Store the reference
            citationRefs["ref" + refNum] = refText;
        }
    }
}

std::string MarkdownConverter::generateBibTeX()
{
    std::stringstream bibtex;

    std::vector<citation::PaperInfo> res;
    for (const auto &ref : citationRefs)
    {
        // Parse the reference text to extract author, title, year, etc.
        // This is a simplified approach; in reality, you might want to parse the
        // reference text more carefully to extract structured information

        std::string refText = ref.second;

        citation::PaperCitationAPI api;

        auto papers = api.search(refText);

        // std::cout << "Found " << papers.size() << " papers." << "\n";

        if (!papers.empty())
        {
            for (size_t i = 0; i < papers.size(); ++i)
            {
                const auto &paper = papers[i];
                std::cout << i + 1 << ". " << paper.title << " (" << paper.year << ")" << "\n";
                for (auto author : paper.authors)
                {
                    std::cout << author << ", ";
                }
                std::cout << "\n";
            }

            std::cout << "Please select a paper, input the number" << "\n";

            std::string ref_str;
            std::getline(std::cin, ref_str);
            int ref_num = std::stoi(ref_str);

            if (ref_num > 0 && ref_num <= papers.size())
            {
                res.emplace_back(papers[ref_num - 1]);
                res.back().citation_key = ref.first;
            }

            // std::cout << "Saving all results to references.bib" << "\n";
        }

        api.saveBibFile(res, "references.bib");
        //// Simple BibTeX entry format
        // bibtex << "@misc{" << ref.first << ",\n";
        // bibtex << "  author = {Author},\n";
        // bibtex << "  title = {" << refText << "},\n";
        // bibtex << "  year = {2023}\n";
        // bibtex << "}\n\n";
    }

    return bibtex.str();
}

std::string MarkdownConverter::escapeLatexChars(const std::string &text)
{
    std::string result = text;

    // LaTeX special characters that need to be escaped: #, $, %, &, _, {, }, ~, ^, \
    // But we need to be careful not to escape characters that are already part of a LaTeX command

    // This is a simplified approach - in a real implementation, this would need
    // to be more sophisticated
    std::regex escapePattern("([\\#\\$\\%\\&\\_\\~\\^\\\\])");
    result = std::regex_replace(result, escapePattern, "\\$1");

    // Fix double escaping that might have occurred from other conversions
    result = std::regex_replace(
        result,
        std::regex("\\\\\\\\(textbf|textit|texttt|href|includegraphics|begin|end|item|"
                   "section|subsection|subsubsection|paragraph|subparagraph|cite)"),
        "\\$1");

    return result;
}