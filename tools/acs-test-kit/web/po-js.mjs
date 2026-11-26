import fs from 'fs';
export default async function run(page, ui) {
  const all = await page.evaluate(() => [...document.scripts].filter(s => !s.src).map(s => s.textContent).join('\n/*----*/\n'));
  fs.writeFileSync('po-page.js', all);
  return all.length;
}
