export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.getByRole('button', { name: 'Save' }).first().click();
  await page.waitForTimeout(3000);
  let snap = await ui.snapshot();
  const btns = [...snap.matchAll(/@(e\d+) button "([^"]+)"/g)].map(m => m[2]).filter(t => /Save|Confirm|Yes|OK|Cancel/.test(t));
  await page.screenshot({ path: 'rmp-after-save.png' });
  return { url: page.url(), btns };
}
