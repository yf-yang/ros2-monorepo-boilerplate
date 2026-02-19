import { readdirSync, readFileSync, statSync } from "node:fs";
import { basename, join, relative } from "node:path";
import { defineConfig } from "vitepress";
import fm from "front-matter";

const docsDir = join(import.meta.dirname, "..");

function titleCase(slug: string): string {
  return slug
    .split("-")
    .map((w) => w.charAt(0).toUpperCase() + w.slice(1))
    .join(" ");
}

interface SidebarItem {
  text: string;
  link: string;
}

interface SidebarGroup {
  text: string;
  items: SidebarItem[];
}

function buildSidebar(): SidebarGroup[] {
  const groups: SidebarGroup[] = [];

  for (const entry of readdirSync(docsDir).sort()) {
    const entryPath = join(docsDir, entry);
    if (!statSync(entryPath).isDirectory() || entry.startsWith(".")) continue;

    const items: SidebarItem[] = [];
    for (const file of readdirSync(entryPath).sort()) {
      if (!file.endsWith(".md")) continue;

      const raw = readFileSync(join(entryPath, file), "utf8");
      const { attributes } = fm<{ title?: string }>(raw);
      const slug = basename(file, ".md");

      items.push({
        text: attributes.title || titleCase(slug),
        link: `/${entry}/${slug}`,
      });
    }

    if (items.length > 0) {
      groups.push({ text: titleCase(entry), items });
    }
  }

  return groups;
}

export default defineConfig({
  title: "ROS2 Workspace",
  description: "Project documentation",
  lang: "zh-CN",
  themeConfig: {
    sidebar: buildSidebar(),
  },
});
