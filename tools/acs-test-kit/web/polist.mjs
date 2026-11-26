export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.goto('https://allcosmos.domainsb.com.my/PurchaseOrder/Index');
  await page.waitForTimeout(5000);
  const rows = await page.evaluate(() => [...document.querySelectorAll('tbody tr')].map(r => r.innerText.replace(/\s+/g,' ').trim()).filter(t => /WB-PO/.test(t)));
  await page.screenshot({ path: 'polist.png' });
  return rows;
}
