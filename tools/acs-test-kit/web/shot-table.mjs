// Screenshot of an edit page's sub-item table, once at the left edge and once scrolled right. URL from env URL, files OUT-left/right.png
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1920, height: 1080 });
  await page.goto(process.env.URL);
  await page.waitForTimeout(5000);
  await page.locator('#sub-item-table').scrollIntoViewIfNeeded();
  await page.waitForTimeout(500);
  await page.screenshot({ path: process.env.OUT + '-left.png' });
  const moved = await page.evaluate(() => {
    const t = document.getElementById('sub-item-table');
    const all = [t, ...t.querySelectorAll('*')].concat(t.shadowRoot ? [...t.shadowRoot.querySelectorAll('*')] : []);
    let n = 0; for (const e of all) { if (e.scrollWidth > e.clientWidth + 20) { e.scrollLeft = e.scrollWidth; n++; } } return n;
  });
  await page.waitForTimeout(800);
  await page.screenshot({ path: process.env.OUT + '-right.png' });
  return { moved };
}
