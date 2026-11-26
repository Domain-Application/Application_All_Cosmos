import { mainWorld } from './mainworld.mjs';
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1600, height: 1000 });
  const rows = await mainWorld(page, `return [...document.querySelectorAll('domain-listing-table')].flatMap(t => (t.getDataList ? t.getDataList() : [])).filter(r => /WB-PO/.test(JSON.stringify(r))).map(r => ({ Id: r.Id, RefNo: r.PurchaseOrderRefNo ?? r.RefNo, Line: r.Sequence ?? r.Line, Rmp: r.RawMaterialPlanningId, keys: Object.keys(r).join(',') }));`);
  if (!rows.length || !rows[0].Id) return rows;
  await page.goto('https://allcosmos.domainsb.com.my/RawMaterialPlanning/Create?purchaseOrderSubId=' + encodeURIComponent(rows[0].Id));
  await page.waitForTimeout(5000);
  await page.screenshot({ path: 'rmp-create.png', fullPage: true });
  return { rows, snap: await ui.snapshot() };
}
