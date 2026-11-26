export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  const po = page.getByRole('textbox').filter({ hasText: '' });
  const poBox = page.locator('input[value^="DEMO-2"]').first();
  await poBox.fill('WB-PO-0001');
  await page.getByRole('button', { name: 'Select First' }).first().click();
  await page.waitForTimeout(2500);
  const card = page.locator('text=Purchase Order API (CSL)').first().locator('xpath=ancestor::div[contains(@class,"card") or contains(@class,"Card")][1]');
  await page.screenshot({ path: 'po-select.png', fullPage: false });
  const inputs = await page.evaluate(() => [...document.querySelectorAll('input,select,textarea')].slice(0,60).map(e => ({tag:e.tagName, name:e.name||e.id||e.getAttribute('aria-label')||'', val:e.value, ph:e.placeholder||''})));
  return inputs;
}
