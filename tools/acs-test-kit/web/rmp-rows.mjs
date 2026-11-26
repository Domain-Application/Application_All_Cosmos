import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  const r = await mainWorld(page, `const tbl = document.getElementById('sub-item-table'); return tbl.getDataList();`);
  await page.locator('#sub-item-table').scrollIntoViewIfNeeded();
  await page.locator('#sub-item-table').screenshot({ path: 'rmp-rows.png' });
  return r;
}
