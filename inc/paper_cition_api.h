#ifndef PAPER_CITATION_API_H
#define PAPER_CITATION_API_H

#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace citation
{

// Paper information structure
struct PaperInfo
{
    std::string title;
    std::vector<std::string> authors;
    std::string journal;
    std::string volume;
    std::string issue;
    std::string pages;
    std::string year;
    std::string doi;
    std::string url;
    std::string publisher;
    std::string abstract;
    std::string citation_key;
    // Additional fields for books
    std::string book_title;
    std::string edition;
    std::string isbn;
    std::string type{"article"}; // Can be "article" or "book"
};

// API query result structure
struct QueryResult
{
    std::vector<PaperInfo> papers;
    std::string raw_response;
    bool success{false};
    std::string error_message;
};

// Network request callback function
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char *)contents, newLength);
        return newLength;
    }
    catch (std::bad_alloc &e)
    {
        return 0;
    }
}

// Abstract base class
class CitationSource
{
  public:
    virtual ~CitationSource() = default;
    virtual QueryResult query(const std::string &query_string) = 0;
    virtual std::string name() const = 0;
};

// CrossRef API implementation
class CrossRefAPI : public CitationSource
{
  public:
    QueryResult query(const std::string &query_string) override
    {
        QueryResult result;

        // Initialize CURL
        CURL *curl = curl_easy_init();
        if (!curl)
        {
            result.error_message = "Failed to initialize CURL";
            return result;
        }

        // Construct URL and encode the query string
        char *encoded_query = curl_easy_escape(curl, query_string.c_str(), query_string.length());
        if (!encoded_query)
        {
            curl_easy_cleanup(curl);
            result.error_message = "Failed to encode query string";
            return result;
        }

        std::string url = "https://api.crossref.org/works?query=" + std::string(encoded_query) +
                          "&rows=5&sort=relevance";
        curl_free(encoded_query);

        // Set request parameters
        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        // Add user agent and email as recommended by the CrossRef API
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
                         "PaperCitationTool/1.0 (mailto:user@example.com)");

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            result.error_message = "CURL request failed: " + std::string(curl_easy_strerror(res));
            return result;
        }

        result.raw_response = response_string;

        try
        {
            // Parse JSON response
            auto json = nlohmann::json::parse(response_string);
            auto items = json["message"]["items"];

            for (const auto &item : items)
            {
                PaperInfo paper;

                // Extract title
                if (item.contains("title") && !item["title"].empty())
                {
                    paper.title = item["title"][0];
                }

                // Extract authors
                if (item.contains("author"))
                {
                    for (const auto &author : item["author"])
                    {
                        std::string author_name;
                        if (author.contains("family"))
                        {
                            author_name = author["family"].get<std::string>();
                        }
                        if (author.contains("given"))
                        {
                            if (!author_name.empty())
                                author_name += ", ";
                            author_name += author["given"].get<std::string>();
                        }
                        if (!author_name.empty())
                        {
                            paper.authors.push_back(author_name);
                        }
                    }
                }

                // Extract journal information
                if (item.contains("container-title") && !item["container-title"].empty())
                {
                    paper.journal = item["container-title"][0];
                }

                // Extract volume number
                if (item.contains("volume"))
                {
                    paper.volume = item["volume"];
                }

                // Extract issue number
                if (item.contains("issue"))
                {
                    paper.issue = item["issue"];
                }

                // Extract page numbers
                if (item.contains("page"))
                {
                    paper.pages = item["page"];
                }

                // Extract year
                if (item.contains("published") && item["published"].contains("date-parts") &&
                    !item["published"]["date-parts"].empty() &&
                    !item["published"]["date-parts"][0].empty())
                {
                    paper.year = std::to_string(item["published"]["date-parts"][0][0].get<int>());
                }

                // Extract DOI
                if (item.contains("DOI"))
                {
                    paper.doi = item["DOI"];
                }

                // Extract URL
                if (item.contains("URL"))
                {
                    paper.url = item["URL"];
                }

                // Extract publisher information
                if (item.contains("publisher"))
                {
                    paper.publisher = item["publisher"];
                }

                // Generate citation key
                if (!paper.authors.empty() && !paper.year.empty())
                {
                    // Extract the last name of the first author
                    size_t comma_pos = paper.authors[0].find(",");
                    std::string first_author = (comma_pos != std::string::npos)
                                                   ? paper.authors[0].substr(0, comma_pos)
                                                   : paper.authors[0];

                    // Remove non-alphanumeric characters
                    first_author.erase(std::remove_if(first_author.begin(), first_author.end(),
                                                      [](unsigned char c)
                                                      { return !std::isalnum(c); }),
                                       first_author.end());

                    paper.citation_key = first_author + paper.year;
                }
                else
                {
                    // If there is no author or year, use DOI or a part of the title
                    if (!paper.doi.empty())
                    {
                        paper.citation_key =
                            "doi" + paper.doi.substr(paper.doi.find_last_of("/") + 1);
                    }
                    else if (!paper.title.empty())
                    {
                        std::string short_title = paper.title.substr(0, 20);
                        short_title.erase(std::remove_if(short_title.begin(), short_title.end(),
                                                         [](unsigned char c)
                                                         { return !std::isalnum(c); }),
                                          short_title.end());
                        paper.citation_key = "title" + short_title;
                    }
                    else
                    {
                        paper.citation_key = "unknown" + std::to_string(result.papers.size() + 1);
                    }
                }

                result.papers.push_back(paper);
            }

            result.success = true;
        }
        catch (const std::exception &e)
        {
            result.error_message = "Failed to parse response: " + std::string(e.what());
        }

        return result;
    }

    std::string name() const override { return "CrossRef"; }
};

// Google Scholar API implementation
class GoogleScholarAPI : public CitationSource
{
  public:
    QueryResult query(const std::string &query_string) override
    {
        QueryResult result;

        // Initialize CURL
        CURL *curl = curl_easy_init();
        if (!curl)
        {
            result.error_message = "Failed to initialize CURL";
            return result;
        }

        // Construct URL and encode the query string
        char *encoded_query = curl_easy_escape(curl, query_string.c_str(), query_string.length());
        if (!encoded_query)
        {
            curl_easy_cleanup(curl);
            result.error_message = "Failed to encode query string";
            return result;
        }

        std::string url = "https://scholar.google.com/scholar?q=" + std::string(encoded_query) +
                          "&hl=en&as_sdt=0,5";
        curl_free(encoded_query);

        // Set request parameters
        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        // Add user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
                         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");

        // Set cookie file
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            result.error_message = "CURL request failed: " + std::string(curl_easy_strerror(res));
            return result;
        }

        result.raw_response = response_string;

        try
        {
            // Parse HTML response
            // Note: A HTML parsing library such as libxml2 or pugixml should be included here.
            // The following is a simplified parsing logic.
            std::istringstream stream(response_string);
            std::string line;
            PaperInfo current_paper;
            bool in_result = false;

            while (std::getline(stream, line))
            {
                // Detect start of a result
                if (line.find("<div class=\"gs_ri\">") != std::string::npos)
                {
                    in_result = true;
                    current_paper = PaperInfo();
                    continue;
                }

                if (in_result)
                {
                    // Extract title
                    if (line.find("<h3 class=\"gs_rt\">") != std::string::npos)
                    {
                        size_t start = line.find(">") + 1;
                        size_t end = line.find("</h3>");
                        if (start != std::string::npos && end != std::string::npos)
                        {
                            current_paper.title = line.substr(start, end - start);
                        }
                    }

                    // Extract authors and year
                    if (line.find("<div class=\"gs_a\">") != std::string::npos)
                    {
                        size_t start = line.find(">") + 1;
                        size_t end = line.find("</div>");
                        if (start != std::string::npos && end != std::string::npos)
                        {
                            std::string author_text = line.substr(start, end - start);
                            // Parse authors
                            std::istringstream author_stream(author_text);
                            std::string author;
                            while (std::getline(author_stream, author, ','))
                            {
                                if (author.find("...") == std::string::npos)
                                {
                                    current_paper.authors.push_back(author);
                                }
                            }

                            // Try to extract year
                            std::regex year_pattern("\\b(19|20)\\d{2}\\b");
                            std::smatch year_match;
                            if (std::regex_search(author_text, year_match, year_pattern))
                            {
                                current_paper.year = year_match.str();
                            }
                        }
                    }

                    // Detect the end of a result
                    if (line.find("</div>") != std::string::npos)
                    {
                        in_result = false;
                        if (!current_paper.title.empty())
                        {
                            // Generate citation key
                            if (!current_paper.authors.empty() && !current_paper.year.empty())
                            {
                                size_t comma_pos = current_paper.authors[0].find(",");
                                std::string first_author =
                                    (comma_pos != std::string::npos)
                                        ? current_paper.authors[0].substr(0, comma_pos)
                                        : current_paper.authors[0];

                                first_author.erase(std::remove_if(first_author.begin(),
                                                                  first_author.end(),
                                                                  [](unsigned char c)
                                                                  { return !std::isalnum(c); }),
                                                   first_author.end());

                                current_paper.citation_key = first_author + current_paper.year;
                            }

                            result.papers.push_back(current_paper);
                        }
                    }
                }
            }

            result.success = true;
        }
        catch (const std::exception &e)
        {
            result.error_message = "Failed to parse response: " + std::string(e.what());
        }

        return result;
    }

    std::string name() const override { return "Google Scholar"; }
};

// Paper citation API class
class PaperCitationAPI
{
  private:
    std::vector<std::unique_ptr<CitationSource>> sources;

  public:
    PaperCitationAPI()
    {
        // Add supported data sources
        // sources.push_back(std::make_unique<CrossRefAPI>());
        sources.push_back(std::make_unique<GoogleScholarAPI>());
        // Additional sources can be added, such as arXiv, IEEE Xplore, Scopus, etc.
    }

    // Search for papers with the given query string
    std::vector<PaperInfo> search(const std::string &query_string)
    {
        std::vector<PaperInfo> results;

        for (const auto &source : sources)
        {
            std::cout << "Querying " << source->name() << "..." << "\n";
            auto query_result = source->query(query_string);

            if (query_result.success)
            {
                results.insert(results.end(), query_result.papers.begin(),
                               query_result.papers.end());
            }
            else
            {
                std::cerr << "Error querying " << source->name() << ": "
                          << query_result.error_message << "\n";
            }
        }

        return results;
    }

    // Convert paper information to BibTeX format
    std::string toBibTeX(const PaperInfo &paper)
    {
        std::string bib_entry;

        // Choose different entry type based on the paper type
        if (paper.type == "book")
        {
            bib_entry = "@book{" + paper.citation_key + ",\n";
        }
        else
        {
            bib_entry = "@article{" + paper.citation_key + ",\n";
        }

        if (!paper.title.empty())
        {
            if (paper.type == "book")
            {
                bib_entry += "  title = {" + paper.title + "},\n";
            }
            else
            {
                bib_entry += "  title = {" + paper.title + "},\n";
            }
        }

        if (!paper.authors.empty())
        {
            bib_entry += "  author = {";
            for (size_t i = 0; i < paper.authors.size(); ++i)
            {
                if (i > 0)
                    bib_entry += " and ";
                bib_entry += paper.authors[i];
            }
            bib_entry += "},\n";
        }

        if (paper.type == "book")
        {
            if (!paper.edition.empty())
            {
                bib_entry += "  edition = {" + paper.edition + "},\n";
            }
            if (!paper.isbn.empty())
            {
                bib_entry += "  isbn = {" + paper.isbn + "},\n";
            }
        }
        else
        {
            if (!paper.journal.empty())
            {
                bib_entry += "  journal = {" + paper.journal + "},\n";
            }
            if (!paper.volume.empty())
            {
                bib_entry += "  volume = {" + paper.volume + "},\n";
            }
            if (!paper.issue.empty())
            {
                bib_entry += "  number = {" + paper.issue + "},\n";
            }
            if (!paper.pages.empty())
            {
                bib_entry += "  pages = {" + paper.pages + "},\n";
            }
        }

        if (!paper.year.empty())
        {
            bib_entry += "  year = {" + paper.year + "},\n";
        }

        if (!paper.publisher.empty())
        {
            bib_entry += "  publisher = {" + paper.publisher + "},\n";
        }

        if (!paper.doi.empty())
        {
            bib_entry += "  doi = {" + paper.doi + "},\n";
        }

        if (!paper.url.empty())
        {
            bib_entry += "  url = {" + paper.url + "},\n";
        }

        // Remove the last comma and newline
        bib_entry.pop_back();
        bib_entry.pop_back();

        bib_entry += "\n}";

        return bib_entry;
    }

    // Generate and save a .bib file
    bool saveBibFile(const std::vector<PaperInfo> &papers,
                     const std::string &filename = "references.bib")
    {
        try
        {
            std::ofstream bib_file(filename);
            if (!bib_file.is_open())
            {
                std::cerr << "Failed to open file: " << filename << "\n";
                return false;
            }

            for (const auto &paper : papers)
            {
                bib_file << toBibTeX(paper) << "\n\n";
            }

            bib_file.close();
            std::cout << "BibTeX file saved: " << std::filesystem::absolute(filename).string()
                      << "\n";
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error saving BibTeX file: " << e.what() << "\n";
            return false;
        }
    }
};

} // namespace citation

#endif // PAPER_CITATION_API_H
