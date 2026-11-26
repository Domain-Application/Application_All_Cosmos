import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  const filled = await mainWorld(page, `
    const d = document.getElementById('date-input'); d.value = '2026-11-26'; d.dispatchEvent(new Event('change', { bubbles: true }));
    const t = document.getElementById('total-trip-input'); t.value = '2'; t.dispatchEvent(new Event('change', { bubbles: true }));
    const tbl = document.getElementById('sub-item-table');
    tbl.setDataList([
      { DeliveryDate: '2026-11-26', DeliveryQuantity: 30, SiteCode: 'SUASA' },
      { DeliveryDate: '2026-11-26', DeliveryQuantity: 30, SiteCode: 'SUASA' }]);
    return { date: d.value, trips: t.value, perTrip: document.getElementById('quantity-per-trip-input').value, rows: tbl.getDataList() };`);
  await page.waitForTimeout(800);
  await page.screenshot({ path: 'rmp-filled.png', fullPage: true });
  if (process.env.SAVE !== '1') return filled;
  await page.getByRole('button', { name: 'Save' }).click();
  await page.waitForTimeout(2500);
  const snap1 = await ui.snapshot();
  const confirm = snap1.match(/@(e\d+) button "(Save|Confirm|Yes|OK)"/g);
  await page.screenshot({ path: 'rmp-after-save.png' });
  return { filled, confirm, url: page.url(), text: (await page.evaluate(() => document.body.innerText)).slice(0, 800) };
}
