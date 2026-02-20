#!/usr/bin/env node
import { readFileSync } from 'node:fs';
import { join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';
import { Command } from 'commander';
import fg from 'fast-glob';
import fm from 'front-matter';

const __dirname = fileURLToPath(new URL('.', import.meta.url));
const DOCS_DIR = join(__dirname, '..', 'docs');

function findDocs(patterns) {
  const globs =
    patterns.length > 0
      ? patterns.map((p) => (p.endsWith('.md') ? p : `${p}*.md`))
      : ['**/*.md'];
  return fg.sync(globs, { cwd: DOCS_DIR, absolute: true }).sort();
}

function relPath(absPath) {
  return relative(DOCS_DIR, absPath);
}

function blank(value) {
  return value ?? '(blank)';
}

function printMetadata(files) {
  for (const absPath of files) {
    const { attributes } = fm(readFileSync(absPath, 'utf8'));
    const title = blank(attributes?.title);
    const description = blank(attributes?.description);
    const readWhen = blank(attributes?.read_when);
    console.log(`${relPath(absPath)} - ${title}`);
    console.log(`\tDescription: ${description}`);
    console.log(`\tRead When: ${readWhen}`);
  }
}

function printFullContent(files) {
  for (const absPath of files) {
    const raw = readFileSync(absPath, 'utf8');
    const { frontmatter, body } = fm(raw);
    console.log(`\n========== ${relPath(absPath)} ==========`);
    if (frontmatter) {
      console.log(`---\n${frontmatter}\n---`);
    }
    process.stdout.write(body);
    if (!body.endsWith('\n')) {
      console.log();
    }
  }
}

const program = new Command();

program.name('docs').description('Browse documentation files');

program
  .command('list')
  .description('List docs metadata, optionally filtered by glob patterns relative to docs/')
  .argument('[patterns...]', 'glob patterns relative to docs/')
  .action((patterns) => {
    const matched = findDocs(patterns);
    if (matched.length === 0) {
      console.log('No docs matched the given patterns.');
      return;
    }
    printMetadata(matched);
  });

program
  .command('read')
  .description('Print full content of docs matching glob patterns relative to docs/')
  .argument('<patterns...>', 'glob patterns relative to docs/')
  .action((patterns) => {
    const matched = findDocs(patterns);
    if (matched.length === 0) {
      console.log('No docs matched the given patterns.');
      return;
    }
    printFullContent(matched);
  });

program.parse();
