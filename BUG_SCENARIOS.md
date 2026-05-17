# Bug Scenarios

These feature branches are intentionally defective review targets built from a clean `main` branch.

- `feature/bug_1`: add reading-time metadata but compute it from rendered HTML instead of markdown text
- `feature/bug_2`: add a latest-posts panel but sort posts in ascending date order
- `feature/bug_3`: render formatted article summaries as raw HTML and widen the HTML injection surface
- `feature/bug_4`: add related-post navigation but exclude the current article incorrectly when dates are missing or titles collide
- `feature/bug_5`: make article cards fully clickable using nested interactive elements
- `feature/bug_6`: generate a sitemap but omit article pages from the output
- `feature/bug_7`: add draft preview asset copies but trust frontmatter paths and write previews into shared slug-based output files
- `feature/bug_8`: render article card snippets but inject `card_snippet` HTML without escaping
- `feature/bug_9`: filter nav links by slug but hard-code the menu to items whose slug contains `guide`
- `feature/bug_10`: add post-build hook support but execute `SITEGEN_POST_BUILD` directly with `std::system`
- `feature/bug_11`: write a build diagnostics log but leak the file descriptor after writing it
- `feature/bug_12`: cache route titles during build but keep a static cache that can retain stale entries across runs
- `feature/bug_13`: use a dedicated menu sort helper but assign in the equality check so ordering falls back to titles
- `feature/bug_14`: collect basic build stats but fail the build whenever any section has no articles
- `feature/bug_15`: load an optional footer include but never close the opened footer file handle
- `feature/bug_16`: add fallback date formatting but replace invalid dates with the article title
- `feature/bug_17`: add publication badge copy but make the `Scheduled` branch unreachable so undated posts show as drafts
- `feature/bug_18`: add a preview key helper but index into the title unsafely for empty titles or trailing spaces
- `feature/bug_19`: add a section title token but copy full titles into a fixed 16-byte buffer
- `feature/bug_20`: register route titles but emit an unrelated `route-title.txt` artifact into the build output
- `feature/bug_21`: write a current-route note but shadow the matched title so the output stays the raw route
- `feature/bug_22`: read an optional featured summary but silently let `featured_summary` override `summary`
- `feature/bug_23`: export generated routes but leak the heap-allocated output stream used to write them
- `feature/bug_24`: render page highlight labels but inject `highlight_label` without HTML escaping
- `feature/bug_25`: load route aliases from content but discard them by writing only a count file to `dist`
- `feature/bug_26`: retry date formatting before fallback but call `formatDate` again after errors so invalid dates still throw
