export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  const reqs = [];
  page.on('request', r => { if (!/\.(js|css|png|svg|woff2?)(\?|$)/.test(r.url())) reqs.push(r.method() + ' ' + r.url()); });
  await page.getByRole('button', { name: 'Select First' }).first().click();
  await page.waitForLoadState('networkidle').catch(() => {});
  await page.waitForTimeout(3000);
  await page.screenshot({ path: 'po-select2.png' });
  return reqs;
}
