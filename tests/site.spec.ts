import { expect, test } from '@playwright/test';

test('home page renders sorted navigation', async ({ page }) => {
  await page.goto('/');

  await expect(page.locator('.panel-header > h1')).toHaveText('Propr Review Demo');
  await expect(page.locator('.site-nav a')).toHaveText(['Propr Review Demo', 'Blog', 'About']);
});

test('about page renders markdown content', async ({ page }) => {
  await page.goto('/about/');

  await expect(page.locator('.panel-header > h1')).toHaveText('About');
  await expect(page.locator('.markdown')).toContainText('content lives in content/');
});

test('blog listing shows articles in descending date order', async ({ page }) => {
  await page.goto('/blog/');

  await expect(page.locator('.panel-header > h1')).toHaveText('Blog');
  await expect(page.locator('.article-card h2')).toHaveText([
    'Welcome to the Demo',
    'Reviewing Pull Requests Effectively',
  ]);
});

test('article page renders title and back link', async ({ page }) => {
  await page.goto('/blog/welcome-to-the-demo/');

  await expect(page.locator('.panel-header > h1')).toHaveText('Welcome to the Demo');
  await expect(page.locator('.back-link')).toHaveAttribute('href', '/blog/');
  await expect(page.locator('.markdown')).toContainText('Write markdown, add frontmatter');
});

test('second article route resolves correctly', async ({ page }) => {
  await page.goto('/blog/reviewing-pull-requests-effectively/');

  await expect(page.locator('.panel-header > h1')).toHaveText('Reviewing Pull Requests Effectively');
  await expect(page.locator('.panel-header')).toContainText('Published May 6, 2026');
});
