export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.mouse.click(1547, 315);
  await page.waitForTimeout(1200);
  await page.screenshot({ path: 'po-kebab.png' });
  return await ui.snapshot();
}
