#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Frontmatter {
  std::map<std::string, std::string> fields;
};

struct Document {
  std::string sourcePath;
  std::string slug;
  std::string route;
  std::string title;
  std::string description;
  std::string summary;
  std::string date;
  int order = 0;
  std::string bodyMarkdown;
  std::string bodyHtml;
};

struct Section {
  Document landing;
  std::vector<Document> articles;
};

std::string trim(const std::string &value) {
  std::size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }

  std::size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }

  return value.substr(start, end - start);
}

std::string readFile(const fs::path &path) {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("Failed to read file: " + path.string());
  }

  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void writeFile(const fs::path &path, const std::string &content) {
  fs::create_directories(path.parent_path());

  std::ofstream output(path);
  if (!output) {
    throw std::runtime_error("Failed to write file: " + path.string());
  }

  output << content;
}

void copyFile(const fs::path &from, const fs::path &to) {
  fs::create_directories(to.parent_path());
  fs::copy_file(from, to, fs::copy_options::overwrite_existing);
}

std::string escapeHtml(const std::string &value) {
  std::string result;
  result.reserve(value.size());

  for (char ch : value) {
    switch (ch) {
    case '&':
      result += "&amp;";
      break;
    case '<':
      result += "&lt;";
      break;
    case '>':
      result += "&gt;";
      break;
    case '"':
      result += "&quot;";
      break;
    default:
      result.push_back(ch);
      break;
    }
  }

  return result;
}

std::string renderInlineMarkdown(const std::string &source) {
  std::string escaped = escapeHtml(source);
  std::string result;
  result.reserve(escaped.size());

  bool inCode = false;
  bool inStrong = false;

  for (std::size_t i = 0; i < escaped.size();) {
    if (escaped.compare(i, 2, "**") == 0) {
      result += inStrong ? "</strong>" : "<strong>";
      inStrong = !inStrong;
      i += 2;
      continue;
    }

    if (escaped[i] == '`') {
      result += inCode ? "</code>" : "<code>";
      inCode = !inCode;
      ++i;
      continue;
    }

    result.push_back(escaped[i]);
    ++i;
  }

  if (inCode) {
    result += "</code>";
  }

  if (inStrong) {
    result += "</strong>";
  }

  return result;
}

std::pair<Frontmatter, std::string> parseFrontmatter(const std::string &contents) {
  Frontmatter frontmatter;
  std::istringstream stream(contents);
  std::string line;

  if (!std::getline(stream, line) || trim(line) != "---") {
    throw std::runtime_error("Expected frontmatter block");
  }

  while (std::getline(stream, line)) {
    if (trim(line) == "---") {
      break;
    }

    std::size_t separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }

    std::string key = trim(line.substr(0, separator));
    std::string value = trim(line.substr(separator + 1));
    frontmatter.fields[key] = value;
  }

  std::ostringstream body;
  bool first = true;
  while (std::getline(stream, line)) {
    if (!first) {
      body << '\n';
    }
    body << line;
    first = false;
  }

  return {frontmatter, body.str()};
}

std::vector<std::string> splitLines(const std::string &contents) {
  std::vector<std::string> lines;
  std::istringstream stream(contents);
  std::string line;

  while (std::getline(stream, line)) {
    lines.push_back(line);
  }

  if (!contents.empty() && contents.back() == '\n') {
    lines.push_back("");
  }

  return lines;
}

const std::string *findField(const std::map<std::string, std::string> &fields, const std::string &key) {
  auto it = fields.find(key);
  if (it == fields.end()) {
    return nullptr;
  }
  return &it->second;
}

std::string renderMarkdown(const std::string &markdown) {
  std::vector<std::string> lines = splitLines(markdown);
  std::ostringstream html;
  std::vector<std::string> paragraph;
  bool inList = false;

  auto flushParagraph = [&]() {
    if (paragraph.empty()) {
      return;
    }

    std::ostringstream joined;
    for (std::size_t i = 0; i < paragraph.size(); ++i) {
      if (i > 0) {
        joined << ' ';
      }
      joined << trim(paragraph[i]);
    }

    html << "<p>" << renderInlineMarkdown(joined.str()) << "</p>\n";
    paragraph.clear();
  };

  auto closeList = [&]() {
    if (inList) {
      html << "</ul>\n";
      inList = false;
    }
  };

  for (const std::string &rawLine : lines) {
    std::string line = trim(rawLine);

    if (line.empty()) {
      flushParagraph();
      closeList();
      continue;
    }

    if (line.rfind("# ", 0) == 0) {
      flushParagraph();
      closeList();
      html << "<h1>" << renderInlineMarkdown(trim(line.substr(2))) << "</h1>\n";
      continue;
    }

    if (line.rfind("- ", 0) == 0) {
      flushParagraph();
      if (!inList) {
        html << "<ul>\n";
        inList = true;
      }
      html << "<li>" << renderInlineMarkdown(trim(line.substr(2))) << "</li>\n";
      continue;
    }

    closeList();
    paragraph.push_back(line);
  }

  flushParagraph();
  closeList();
  return html.str();
}

int parseOrder(const std::map<std::string, std::string> &fields) {
  auto it = fields.find("order");
  if (it == fields.end() || trim(it->second).empty()) {
    return 0;
  }

  return std::stoi(it->second);
}

Document parseDocument(const fs::path &path, const std::string &route, const std::string &slug) {
  auto parsed = parseFrontmatter(readFile(path));
  const Frontmatter &frontmatter = parsed.first;
  const std::string &body = parsed.second;

  Document document;
  document.sourcePath = path.string();
  document.slug = slug;
  document.route = route;
  document.title = frontmatter.fields.at("title");
  document.description = frontmatter.fields.count("description") ? frontmatter.fields.at("description") : "";
  if (frontmatter.fields.count("featured_summary")) {
    document.summary = *findField(frontmatter.fields, "featured_summary");
  } else {
    document.summary = frontmatter.fields.count("summary") ? frontmatter.fields.at("summary") : "";
  }
  document.date = frontmatter.fields.count("date") ? frontmatter.fields.at("date") : "";
  document.order = parseOrder(frontmatter.fields);
  document.bodyMarkdown = body;
  document.bodyHtml = renderMarkdown(body);
  return document;
}

bool sortNav(const Document &left, const Document &right) {
  if (left.order != right.order) {
    return left.order < right.order;
  }
  return left.title < right.title;
}

bool sortArticles(const Document &left, const Document &right) {
  if (left.date != right.date) {
    return left.date > right.date;
  }
  if (left.order != right.order) {
    return left.order < right.order;
  }
  return left.title < right.title;
}

std::string formatDate(const std::string &date) {
  if (date.size() != 10) {
    return date;
  }

  static const char *months[] = {
      "January", "February", "March",     "April",   "May",      "June",
      "July",    "August",   "September", "October", "November", "December",
  };

  int month = std::stoi(date.substr(5, 2));
  int day = std::stoi(date.substr(8, 2));
  int year = std::stoi(date.substr(0, 4));

  std::ostringstream formatted;
  formatted << months[month - 1] << ' ' << day << ", " << year;
  return formatted.str();
}

std::string pageTitle(const std::string &title) {
  return title + " | Propr Review Demo";
}

std::string navHtml(const std::vector<Document> &navItems, const std::string &currentRoute) {
  std::ostringstream html;

  for (const Document &item : navItems) {
    std::string className = "nav-link";
    if (item.route == currentRoute) {
      className += " nav-link-active";
    }

    html << "<a class=\"" << className << "\" href=\"" << item.route << "\">"
         << escapeHtml(item.title) << "</a>";
  }

  return html.str();
}

std::string layout(const std::string &title, const std::string &nav, const std::string &mainContent) {
  std::ostringstream html;
  html << "<!DOCTYPE html>\n"
       << "<html lang=\"en\">\n"
       << "<head>\n"
       << "  <meta charset=\"utf-8\">\n"
       << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
       << "  <title>" << escapeHtml(title) << "</title>\n"
       << "  <link rel=\"stylesheet\" href=\"/styles.css\">\n"
       << "</head>\n"
       << "<body>\n"
       << "  <div class=\"app-shell\">\n"
       << "    <header class=\"site-header\">\n"
       << "      <div>\n"
       << "        <a class=\"site-title\" href=\"/\">Propr Review Demo</a>\n"
       << "        <p class=\"site-tagline\">A lightweight static blog scaffold used for testing pull request review flows.</p>\n"
       << "      </div>\n"
       << "      <nav class=\"site-nav\">" << nav << "</nav>\n"
       << "    </header>\n"
       << "    <main>" << mainContent << "</main>\n"
       << "  </div>\n"
       << "</body>\n"
       << "</html>\n";
  return html.str();
}

std::string renderPageContent(const Document &document) {
  std::ostringstream html;
  html << "<article class=\"panel stack-gap\">\n"
       << "  <header class=\"panel-header\">\n"
       << "    <h1>" << escapeHtml(document.title) << "</h1>\n";

  if (!document.description.empty()) {
    html << "    <p>" << escapeHtml(document.description) << "</p>\n";
  }

  html << "  </header>\n"
       << "  <div class=\"markdown\">\n"
       << document.bodyHtml
       << "  </div>\n"
       << "</article>\n";
  return html.str();
}

std::string renderSectionContent(const Section &section) {
  std::ostringstream html;
  html << "<section class=\"panel stack-gap\">\n"
       << "  <header class=\"panel-header\">\n"
       << "    <h1>" << escapeHtml(section.landing.title) << "</h1>\n";

  if (!section.landing.description.empty()) {
    html << "    <p>" << escapeHtml(section.landing.description) << "</p>\n";
  }

  html << "  </header>\n"
       << "  <div class=\"markdown\">\n"
       << section.landing.bodyHtml
       << "  </div>\n"
       << "  <div class=\"article-list\">\n";

  for (const Document &article : section.articles) {
    html << "    <article class=\"article-card\">\n"
         << "      <div class=\"article-card-meta\"><span>" << escapeHtml(formatDate(article.date))
         << "</span></div>\n"
         << "      <h2><a href=\"" << article.route << "\">" << escapeHtml(article.title)
         << "</a></h2>\n";

    if (!article.summary.empty()) {
      html << "      <p>" << escapeHtml(article.summary) << "</p>\n";
    } else if (!article.description.empty()) {
      html << "      <p>" << escapeHtml(article.description) << "</p>\n";
    }

    html << "    </article>\n";
  }

  html << "  </div>\n"
       << "</section>\n";
  return html.str();
}

std::string renderArticleContent(const Section &section, const Document &article) {
  std::ostringstream html;
  html << "<article class=\"panel stack-gap\">\n"
       << "  <a class=\"back-link\" href=\"" << section.landing.route << "\">Back to "
       << escapeHtml(section.landing.title) << "</a>\n"
       << "  <header class=\"panel-header\">\n"
       << "    <h1>" << escapeHtml(article.title) << "</h1>\n";

  if (!article.description.empty()) {
    html << "    <p>" << escapeHtml(article.description) << "</p>\n";
  }

  html << "    <p>Published " << escapeHtml(formatDate(article.date)) << "</p>\n"
       << "  </header>\n"
       << "  <div class=\"markdown\">\n"
       << article.bodyHtml
       << "  </div>\n"
       << "</article>\n";
  return html.str();
}

void writeRoute(const fs::path &distDir, const std::string &route, const std::string &html) {
  fs::path filePath = distDir;
  if (route == "/") {
    filePath /= "index.html";
  } else {
    std::string trimmed = route;
    if (!trimmed.empty() && trimmed.front() == '/') {
      trimmed.erase(0, 1);
    }
    if (!trimmed.empty() && trimmed.back() == '/') {
      trimmed.pop_back();
    }
    filePath /= trimmed;
    filePath /= "index.html";
  }

  writeFile(filePath, html);
}

int main() {
  try {
    const fs::path repoRoot = fs::current_path();
    const fs::path contentDir = repoRoot / "content";
    const fs::path staticDir = repoRoot / "static";
    const fs::path distDir = repoRoot / "dist";

    if (fs::exists(distDir)) {
      fs::remove_all(distDir);
    }
    fs::create_directories(distDir);

    std::vector<Document> pages;
    std::vector<Section> sections;

    for (const fs::directory_entry &entry : fs::directory_iterator(contentDir)) {
      const fs::path path = entry.path();
      if (entry.is_regular_file() && path.extension() == ".md") {
        const std::string slug = path.stem().string();
        const std::string route = slug == "index" ? "/" : "/" + slug + "/";
        pages.push_back(parseDocument(path, route, slug));
      }

      if (entry.is_directory()) {
        const std::string sectionSlug = path.filename().string();
        const fs::path landingPath = path / "_index.md";
        if (!fs::exists(landingPath)) {
          continue;
        }

        Section section;
        section.landing = parseDocument(landingPath, "/" + sectionSlug + "/", sectionSlug);

        for (const fs::directory_entry &child : fs::directory_iterator(path)) {
          const fs::path childPath = child.path();
          if (!child.is_regular_file() || childPath.extension() != ".md" || childPath.filename() == "_index.md") {
            continue;
          }

          const std::string articleSlug = childPath.stem().string();
          section.articles.push_back(
              parseDocument(childPath, "/" + sectionSlug + "/" + articleSlug + "/", articleSlug));
        }

        std::sort(section.articles.begin(), section.articles.end(), sortArticles);
        sections.push_back(section);
      }
    }

    std::sort(pages.begin(), pages.end(), sortNav);
    std::sort(sections.begin(), sections.end(), [](const Section &left, const Section &right) {
      return sortNav(left.landing, right.landing);
    });

    std::vector<Document> navItems;
    for (const Document &page : pages) {
      navItems.push_back(page);
    }
    for (const Section &section : sections) {
      navItems.push_back(section.landing);
    }
    std::sort(navItems.begin(), navItems.end(), sortNav);

    for (const Document &page : pages) {
      writeRoute(distDir, page.route,
                 layout(pageTitle(page.title), navHtml(navItems, page.route), renderPageContent(page)));
    }

    for (const Section &section : sections) {
      writeRoute(distDir, section.landing.route,
                 layout(pageTitle(section.landing.title), navHtml(navItems, section.landing.route),
                        renderSectionContent(section)));

      for (const Document &article : section.articles) {
        writeRoute(distDir, article.route,
                   layout(pageTitle(article.title), navHtml(navItems, section.landing.route),
                          renderArticleContent(section, article)));
      }
    }

    copyFile(staticDir / "styles.css", distDir / "styles.css");
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }
}
