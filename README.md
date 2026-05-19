# Propr Review Demo C++

Small static blog review demo built with a native C++ static-site generator.

## Requirements

- `g++` with C++17 support
- Node.js and npm

## Commands

- `make build` compiles `src/main.cpp` and generates `dist/`
- `npm install` installs Playwright test dependencies
- `npx playwright install --with-deps chromium` installs the browser used by e2e tests
- `npm test` runs the Playwright suite against `dist/`
- `make test` rebuilds the site and runs e2e tests

## Output

The generated site includes these routes:

- `/`
- `/about/`
- `/blog/`
- `/blog/welcome-to-the-demo/`
- `/blog/reviewing-pull-requests-effectively/`

## Content model

- First-class site pages and sections are expected to come from markdown content stored under `content/`.
- New user-visible areas should be introduced by adding content files instead of embedding page definitions directly in the generator.

## Review branches

- `BUG_SCENARIOS.md` lists the intentionally defective feature branches that should be reviewed against `main`.
