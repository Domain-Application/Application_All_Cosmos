import { mainWorld } from './mainworld.mjs';
import po from './po-send.mjs';
import dt from './dt-send.mjs';
// Creates one test PO + Raw Material Planning (2 lines of 30, today) and one Despatch Ticket + Daily Delivery Planning (today)
// on the UAT website, through the same pages a user would use. Env: PO, ITEM, DT, DO, LORRY, SEND=1, TODAY=yyyy-MM-dd.
export default async function run(page, ui) {
  const out = {};
  out.po = (await po(page, ui)).result?.match(/"Message": "[^"]*"|"status": "\w"/g);

  // Raw Material Planning from the new PO line
  await page.goto('https://allcosmos.domainsb.com.my/PurchaseOrder/Index');
  await page.waitForTimeout(5000);
  const line = await mainWorld(page, `return [...document.querySelectorAll('domain-listing-table')].flatMap(t => t.getDataList ? t.getDataList() : []).find(r => r.RefNo === ${JSON.stringify(process.env.PO)});`);
  out.poLineId = line?.Id; out.hasPlanning = line?.RawMaterialPlanningId;
  if (line && !line.RawMaterialPlanningId) {
    await page.goto('https://allcosmos.domainsb.com.my/RawMaterialPlanning/Create?purchaseOrderSubId=' + line.Id);
    await page.waitForTimeout(5000);
    await mainWorld(page, `
      const d = document.getElementById('date-input'); d.value = ${JSON.stringify(process.env.TODAY)}; d.dispatchEvent(new Event('change', { bubbles: true }));
      const t = document.getElementById('total-trip-input'); t.value = '2'; t.dispatchEvent(new Event('change', { bubbles: true }));
      document.getElementById('sub-item-table').setDataList([
        { DeliveryDate: ${JSON.stringify(process.env.TODAY)}, DeliveryQuantity: 30, SiteCode: 'SUASA' },
        { DeliveryDate: ${JSON.stringify(process.env.TODAY)}, DeliveryQuantity: 30, SiteCode: 'SUASA' }]);
      return true;`);
    await page.waitForTimeout(1500);
    await mainWorld(page, `document.querySelector("[data-edit-page-button='save']").click(); return true;`);
    await page.waitForURL(/Edit\?modelKey=/, { timeout: 20000 }).catch(() => {});
    out.planningUrl = page.url();
  }

  // Despatch Ticket + Daily Delivery Planning
  out.dt = (await dt(page, ui)).result?.match(/"Message": "[^"]*"|"status": "\w"/g);
  await page.goto('https://allcosmos.domainsb.com.my/DailyDeliveryPlanning/Create?despatchTicketRefNo=' + encodeURIComponent(process.env.DT));
  await page.waitForTimeout(5000);
  out.ddpLines = await mainWorld(page, `return document.getElementById('sub-item-table').getDataList().map(r => ({ DONo: r.DONo, DespatchTicketSubId: r.DespatchTicketSubId, SiteCode: r.SiteCode }));`);
  await mainWorld(page, `const d = document.querySelector('domain-datepicker[name="saveDto.DeliveryDate"]'); d.value = ${JSON.stringify(process.env.TODAY)}; d.dispatchEvent(new Event('change', { bubbles: true })); return true;`);
  await page.waitForTimeout(1000);
  await mainWorld(page, `document.querySelector("[data-edit-page-button='save']").click(); return true;`);
  await page.waitForURL(/Edit\?modelKey=/, { timeout: 20000 }).catch(() => {});
  out.ddpUrl = page.url();

  // Ref Nos and DO as the API Demo lists show them
  await page.goto('https://allcosmos.domainsb.com.my/APIDemo/Index');
  await page.waitForTimeout(6000);
  out.lists = await mainWorld(page, `
    const get = id => (document.getElementById(id)?.getDataList() ?? []);
    return { ticketIn: get('api-demo-ticket-in-table').filter(r => r.PurchaseOrderRefNo === ${JSON.stringify(process.env.PO)}).map(r => r.RefNo),
             doIn: get('api-demo-do-time-in-table').filter(r => r.DONo === ${JSON.stringify(process.env.DO)}) };`);
  return out;
}
