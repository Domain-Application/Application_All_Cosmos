export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  const reqs = [];
  page.on('response', async r => { const u = r.url(); if (!/\.(js|css|png|svg|woff2?|ico)(\?|$)/.test(u)) { let b=''; try { b = (await r.text()).slice(0,300); } catch {} reqs.push(r.request().method()+' '+r.status()+' '+u+' :: '+b); } });
  await page.locator('text=Site (company_code)').first().click();
  await page.waitForTimeout(2500);
  await page.screenshot({ path: 'po-site.png' });
  return reqs;
}
