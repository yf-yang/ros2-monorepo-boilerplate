#!/usr/bin/env node
import { readFileSync, readdirSync, statSync } from 'node:fs';
import { join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';
import { Command } from 'commander';
import fm from 'front-matter';

const __dirname = fileURLToPath(new URL('.', import.meta.url));
const REPO_ROOT = join(__dirname, '..');
const DOCS_DIR = join(REPO_ROOT, 'docs');

function collectMdFiles(dir) {
  const results = [];
  for (const entry of readdirSync(dir)) {
    const full = join(dir, entry);
    if (statSync(full).isDirectory()) {
      results.push(...collectMdFiles(full));
    } else if (entry.endsWith('.md')) {
      results.push(full);
    }
  }
  return results;
}

function relPath(absPath) {
  return relative(DOCS_DIR, absPath);
}

function blank(value) {
  return value ?? '(blank)';
}

const program = new Command();

program
  .name('read-docs')
  .description('Browse documentation files')
  .argument('[files...]', 'specific doc file paths relative to `docs/` to print in full. If no files are provided, all doc metadata will be printed.')
  .action((files) => {
    if (files.length > 0) {
      for (const filePath of files) {
        const absPath = filePath.startsWith('/') ? filePath : join(DOCS_DIR, filePath);
        const raw = readFileSync(absPath, 'utf8');
        const { frontmatter, body } = fm(raw);
        if (frontmatter) {
          console.log(`---\n${frontmatter}\n---`);
        }
        process.stdout.write(body);
        if (!body.endsWith('\n')) {
          console.log();
        }
      }
    } else {
      const files = collectMdFiles(DOCS_DIR);
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
  });

program.parse();
