import { mainWorld } from './mainworld.mjs';
// Sends test PO WB-PO-0001 through the API Demo page's own form (Purchase Order API (CSL) card).
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.goto('https://allcosmos.domainsb.com.my/APIDemo/Index');
  await page.waitForTimeout(4000);
  const filled = await mainWorld(page, `
    const set = (id, v) => { const e = document.getElementById(id); e.value = v; e.dispatchEvent(new Event('change', { bubbles: true })); return e.value; };
    const r = {
      site: set('api-demo-site-input', '2'), div: set('api-demo-division-input', 'NT'),
      po: set('api-demo-po-number-input', ${JSON.stringify(process.env.PO)}), sup: set('api-demo-supplier-input', '4000A028'),
      cur: set('api-demo-currency-input', 'MYR'), ship: set('api-demo-shipping-term-input', 'DEL'),
      cred: set('api-demo-credit-term-input', '30'), type: set('api-demo-purchase-type-input', 'Local')
    };
    document.getElementById('api-demo-line-table').setDataList([
      { ItemCode: ${JSON.stringify(process.env.ITEM)}, PurchaseItemUnitCode: 'MT', Quantity: 60, PurchasePrice: 1500, OutstandingQuantity: 60 }
    ]);
    r.lines = document.getElementById('api-demo-line-table').getDataList().length;
    return r;`);
  await page.waitForTimeout(800);
  await page.screenshot({ path: 'po-filled.png' });
  if (process.env.SEND !== '1') return { filled, sent: false };
  await mainWorld(page, `document.getElementById('api-demo-send-button').click(); return true;`);
  await page.waitForTimeout(6000);
  const result = await page.evaluate(() => document.getElementById('api-demo-purchase-order-result')?.innerText.slice(0, 4000));
  await page.locator('#api-demo-purchase-order-result').screenshot({ path: 'po-result.png' }).catch(() => {});
  return { filled, result };
}
