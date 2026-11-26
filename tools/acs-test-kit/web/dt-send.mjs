import { mainWorld } from './mainworld.mjs';
// Sends test Despatch Ticket WB-DT-0001 (DO WB-DO-0001) through the API Demo page's Despatch Ticket API (CSL) card.
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  await page.goto('https://allcosmos.domainsb.com.my/APIDemo/Index');
  await page.waitForTimeout(4000);
  const filled = await mainWorld(page, `
    const set = (id, v) => { const e = document.getElementById(id); e.value = v; e.dispatchEvent(new Event('change', { bubbles: true })); return e.value; };
    const r = {
      site: set('api-demo-despatch-site-input', '2'), div: set('api-demo-despatch-division-input', 'NT'),
      ticket: set('api-demo-despatch-ticket-input', ${JSON.stringify(process.env.DT)}), lorry: set('api-demo-despatch-lorry-input', ${JSON.stringify(process.env.LORRY)}),
      transporter: set('api-demo-despatch-transporter-input', 'PROG'), vendor: set('api-demo-despatch-vendor-input', '4005P093'),
      unit: set('api-demo-despatch-unit-input', 'MT')
    };
    document.getElementById('api-demo-despatch-line-table').setDataList([
      { CustomerCode: 'A0025', ItemCode: 'FG000000ERPST50', DONo: ${JSON.stringify(process.env.DO)}, Weight: 20, DOQuantity: 20, DOUnitCode: 'MT' }
    ]);
    return r;`);
  await page.waitForTimeout(800);
  await page.locator('#api-demo-despatch-line-table').scrollIntoViewIfNeeded();
  await page.screenshot({ path: 'dt-filled.png' });
  if (process.env.SEND !== '1') return { filled, sent: false };
  await mainWorld(page, `document.getElementById('api-demo-despatch-send-button').click(); return true;`);
  await page.waitForTimeout(6000);
  const result = await page.evaluate(() => document.getElementById('api-demo-despatch-ticket-result')?.innerText.slice(0, 4000));
  return { filled, result };
}
