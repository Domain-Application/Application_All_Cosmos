import fs from 'fs';
import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  const all = await page.evaluate(() => [...document.scripts].filter(s => !s.src).map(s => s.textContent).join('\n/*----*/\n'));
  fs.writeFileSync('rmp-page.js', all);
  return await mainWorld(page, `
    const tables = [...document.querySelectorAll('domain-editor-table, domain-listing-table')].map(t => ({ id: t.id, rows: t.getDataList ? t.getDataList().length : -1 }));
    const inputs = [...document.querySelectorAll('[id]')].filter(e => /input|select|picker/i.test(e.tagName) && e.id).map(e => e.tagName + '#' + e.id + '=' + (e.value ?? ''));
    return { tables, inputs: inputs.slice(0, 60) };`);
}
