"""
description: C++ static-site generator review conventions covering content-driven sections, routing, navigation, ordering, and HTML rendering.
when-to-use: When files change in src/main.cpp, content/, static/, Makefile, or generated route/navigation behavior.
"""

# ProPR Review Instructions

## Project Summary

This repository is a small static blog demo used for pull request review workflows.

- `content/` contains markdown source files and frontmatter.
- `src/main.cpp` is the native static-site generator.
- `Makefile` compiles the generator and writes the site into `dist/`.
- `static/styles.css` is copied into the generated site unchanged.

## Review Priorities

Prioritize correctness, regressions, and maintainability over style nits.

Focus most on:

- content pipeline correctness
- route resolution and navigation behavior
- consistency between markdown conventions and generated output
- user-facing rendering issues
- changes that weaken the escaping or HTML rendering safety story

Avoid low-value comments about minor wording, formatting, or subjective style unless they affect behavior, clarity, or consistency.

## Important Repo Conventions

### Content Structure

- `content/index.md` maps to `/`.
- `content/<name>.md` maps to `/<name>/`.
- `content/<section>/_index.md` defines a top-level section at `/<section>/`.
- additional markdown files in `content/<section>/` become article pages at `/<section>/<article>/`.
- first-class site sections should be content-driven under `content/`, rather than declared as built-in sections in `src/main.cpp`

Reviewers should flag changes that break these conventions without updating the generator and app logic consistently.

### Sorting and Navigation Invariants

The current generator behavior is intentional:

- pages and sections are ordered by `order`, then `title`
- articles are ordered by `date` descending, then `order`, then `title`
- navigation is derived from content files and section landing pages, not maintained separately

Flag PRs that accidentally change these ordering rules or introduce duplicated sources of truth.

### Routing Expectations

- the generated site uses directory routes with `index.html` files
- every expected route should exist as a concrete file under `dist/`
- section landing pages and article pages must remain stable as content changes

Be alert for regressions where route generation, navigation highlighting, or article links stop matching the generated paths.

## Risk Areas Worth Extra Attention

### HTML Rendering

`src/main.cpp` renders markdown into HTML during the build step.

Reviewers should scrutinize changes that:

- allow raw HTML through the markdown pipeline without deliberate escaping rules
- bypass the generator when producing files in `dist/`
- change supported markdown constructs without updating tests

### Date Handling

Dates come from frontmatter strings and drive article ordering.

Flag changes that make article ordering unstable, display invalid dates, or mix formatted dates with raw values inconsistently.

## Good Review Questions

When relevant, ask yourself and analyze:

- Does this change preserve the content-to-route mapping rules?
- If source markdown changed, does `make build` still generate the expected files?
- If the generator changed, do the rendered pages and article ordering still match the content rules?
- Does this introduce a second source of truth for navigation or routes?
- Does this change broaden the HTML safety surface or reduce escaping?
- Does this alter article or navigation order unintentionally?

## Review Tone

Keep comments concrete and actionable. Prefer identifying:

- broken behavior
- routing regressions
- maintainability risks from duplicated route or sorting logic

Prefer not to comment on purely stylistic or syntactic choices unless they obscure intent or increase the chance of future mistakes.

## Semantic Benchmark Guidance

- First-class user-visible pages and sections must remain content-driven under `content/`.
- Flag PRs that introduce hardcoded or built-in site content in generator code instead of representing that content in markdown.
