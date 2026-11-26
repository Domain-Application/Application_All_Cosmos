import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  const v = await mainWorld(page, `const d = document.querySelector('domain-datepicker[name="saveDto.DeliveryDate"]'); d.value = '2026-11-26'; d.dispatchEvent(new Event('change', { bubbles: true })); return d.value;`);
  await page.waitForTimeout(800);
  await mainWorld(page, `document.querySelector("[data-edit-page-button='save']").click(); return true;`);
  await page.waitForURL(/Edit\?modelKey=/, { timeout: 20000 }).catch(() => {});
  await page.waitForTimeout(4000);
  await page.screenshot({ path: 'ddp-saved.png' });
  return { v, url: page.url() };
}
