import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await mainWorld(page, `document.querySelector("[data-edit-page-button='save']").click(); return true;`);
  await page.waitForURL(/Edit\?modelKey=/, { timeout: 20000 }).catch(() => {});
  await page.waitForTimeout(4000);
  const msg = await page.evaluate(() => [...document.querySelectorAll('domain-notice, .notice, [role=alert], .toast')].map(e => e.innerText).join(' | '));
  await page.screenshot({ path: 'rmp-saved.png' });
  return { url: page.url(), msg };
}
