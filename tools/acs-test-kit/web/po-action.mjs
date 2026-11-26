export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.getByRole('button', { name: 'Action' }).first().click();
  await page.waitForTimeout(1200);
  await page.screenshot({ path: 'po-action.png' });
  const items = await page.evaluate(() => [...document.querySelectorAll('[role=menuitem], .dropdown-menu a, .dropdown-menu button, li a, li button')].map(e => e.innerText.trim()).filter(t => /Create|Edit|View|Planning|Costing/i.test(t)));
  return items;
}
