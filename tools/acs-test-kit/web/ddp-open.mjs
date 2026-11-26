import fs from 'fs';
import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.goto('https://allcosmos.domainsb.com.my/DailyDeliveryPlanning/Create?despatchTicketRefNo=WB-DT-0001');
  await page.waitForTimeout(5000);
  const all = await page.evaluate(() => [...document.scripts].filter(s => !s.src).map(s => s.textContent).join('\n/*----*/\n'));
  fs.writeFileSync('ddp-page.js', all);
  await page.screenshot({ path: 'ddp-create.png' });
  return await mainWorld(page, `
    const tables = [...document.querySelectorAll('domain-editor-table, domain-listing-table')].map(t => ({ id: t.id, rows: t.getDataList ? t.getDataList() : null }));
    const inputs = [...document.querySelectorAll('[id]')].filter(e => /^DOMAIN-/.test(e.tagName)).map(e => e.tagName + '#' + e.id + '=' + (e.value ?? ''));
    return { tables, inputs };`);
}
